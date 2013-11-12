#include "VoiceTranscoder.h"
#include "EngineFuncs.h"
#include "CRC32.h"
#include "Build.h"

PCLCFUNCS_CALLBACK g_pfnSVParseVoiceData;

size_t g_sizeClientStruct;

server_static_t *g_psvs;

playervcodec_t g_PlayerVCodec[ MAX_CLIENTS+1 ];

CSpeex *g_pVoiceSpeex[ MAX_CLIENTS ];
CSilk *g_pVoiceSilk[ MAX_CLIENTS ];

cvar_t g_cvarVoiceVolumeSpeex = {"sv_voicevolume_speex", "1.0", FCVAR_EXTDLL};
cvar_t g_cvarVoiceVolumeSilk = {"sv_voicevolume_silk", "1.0", FCVAR_EXTDLL};
cvar_t g_cvarVoiceFloodMs = {"sv_voicefloodms", "30", FCVAR_EXTDLL};
cvar_t g_cvarVTCVersion = {"vtc_version", GetBuildNumberAsString(), FCVAR_EXTDLL};

cvar_t *g_pcvarVoiceEnable;
cvar_t *g_pcvarVoiceCodec;
cvar_t *g_pcvarVoiceQuality;
cvar_t *g_pcvarVoiceVolumeSpeex;
cvar_t *g_pcvarVoiceVolumeSilk;
cvar_t *g_pcvarVoiceFloodMs;
cvar_t *g_pcvarVTCVersion;

bool g_bIsntSpeex;

char g_szOldVoiceCodec[12];
int g_iOldVoiceQuality;

qboolean VTC_Init( void ) {
	if (!DProtoAPI_Init()) {
		return false;
	}

	// Change clc_voicedata callback
	sv_clcfuncs_t *pCLC = (sv_clcfuncs_t *)g_pDprotoAPI->p_clc_funcs;

	g_pfnSVParseVoiceData = pCLC[CLC_VOICEDATA].pfnCallback;
	pCLC[CLC_VOICEDATA].pfnCallback = &SV_ParseVoiceData;

	// Get size client_t
	g_sizeClientStruct = g_pDprotoAPI->client_t_size;

	// Get svs
	g_psvs = (server_static_t *)g_pDprotoAPI->p_svs;

	// Get MSG_** specified vars and funcs
	g_pMsgReadcount = (size_t *)g_pDprotoAPI->p_msg_readcount;
	g_pMsgBadread = (bool *)g_pDprotoAPI->p_msg_badread;
	g_pNetMessage = (sizebuf_t *)g_pDprotoAPI->p_net_message_addr;
	g_pfnSZGetSpace = (PSZ_GETSPACE)g_pDprotoAPI->p_SZ_GetSpace;

	CVAR_REGISTER(&g_cvarVoiceVolumeSpeex);
	CVAR_REGISTER(&g_cvarVoiceVolumeSilk);
	CVAR_REGISTER(&g_cvarVoiceFloodMs);
	CVAR_REGISTER(&g_cvarVTCVersion);

	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");
	g_pcvarVoiceCodec = CVAR_GET_POINTER("sv_voicecodec");
	g_pcvarVoiceQuality = CVAR_GET_POINTER("sv_voicequality");
	g_pcvarVoiceVolumeSpeex = CVAR_GET_POINTER("sv_voicevolume_speex");
	g_pcvarVoiceVolumeSilk = CVAR_GET_POINTER("sv_voicevolume_silk");
	g_pcvarVoiceFloodMs = CVAR_GET_POINTER("sv_voicefloodms");
	g_pcvarVTCVersion = CVAR_GET_POINTER("vtc_version");

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i] = new CSpeex;

		if ( !g_pVoiceSpeex[i] ) {
			LOG_ERROR( PLID, "Couldn't get speex codec" );

			return FALSE;
		}

		if ( !g_pVoiceSpeex[i]->Init(g_pcvarVoiceQuality->value) ) {
			LOG_ERROR( PLID, "Couldn't initialize speex codec" );

			return FALSE;
		}
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		g_pVoiceSilk[i] = new CSilk;

		if ( !g_pVoiceSilk[i] ) {
			LOG_ERROR( PLID, "Couldn't get silk codec" );

			return FALSE;
		}

		if ( !g_pVoiceSilk[i]->Init(0) ) {
			LOG_ERROR( PLID, "Couldn't initialize silk codec" );

			return FALSE;
		}
	}

	strncpy(g_szOldVoiceCodec, g_pcvarVoiceCodec->string, sizeof(g_szOldVoiceCodec));
	g_iOldVoiceQuality = g_pcvarVoiceQuality->value;

	if (strcmp(g_pcvarVoiceCodec->string, "voice_speex") != 0) {
		LOG_MESSAGE( PLID, "Warning vcodec isn't speex. Voice disabled." );

		g_bIsntSpeex = true;
	} else {
		g_bIsntSpeex = false;
	}

	return TRUE;
}

qboolean VTC_End( void ) {
	DProtoAPI_Deinit();

	// Change clc_voicedata callback
	sv_clcfuncs_t *pCLC = (sv_clcfuncs_t *)g_pDprotoAPI->p_clc_funcs;

	pCLC[CLC_VOICEDATA].pfnCallback = g_pfnSVParseVoiceData;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i]->Release();
		g_pVoiceSilk[i]->Release();
	}

	return TRUE;
}

float g_flLastReceivedVoice[32];

void SV_ParseVoiceData(client_t *pClient) {
	int i, iClient;
	int nDataLength, nCompressedLength, nDecompressedSamples;
	byte chReceived[4096];
	short chDecompressed[4096];
	byte chCompressed[4096];
	client_t *pDestClient;

	iClient = ((size_t)pClient - (size_t)g_psvs->m_pClients) / g_sizeClientStruct;

	nDataLength = MSG_ReadShort( );

	if ( nDataLength > sizeof( chReceived ) ) {
		LOG_MESSAGE(PLID, "SV_ParseVoiceData: invalid incoming packet.\n");

		((void (*)(client_t *, bool, const char *, ...))g_pDprotoAPI->p_SV_DropClient)(pClient, false, "Invalid voice data\n");

		return;
	}

	MSG_ReadBuf( nDataLength, chReceived );

	if (g_PlayerVCodec[iClient + 1].m_voiceCodec == VOICECODEC_SILK) {
		if (nDataLength > sizeof(ulong)) {
			ulong ulOurCRC = ~ComputeCRC(0xFFFFFFFF, chReceived, nDataLength - sizeof(ulong));

			if (ulOurCRC != *(ulong *)&chReceived[nDataLength - sizeof(ulong)]) {
				LOG_MESSAGE(PLID, "SV_ParseVoiceData: invalid incoming packet.\n");

				((void (*)(client_t *, bool, const char *, ...))g_pDprotoAPI->p_SV_DropClient)(pClient, false, "Invalid voice data\n");

				return;
			}
		}
	}

	if ((gpGlobals->time - g_flLastReceivedVoice[iClient]) < (g_pcvarVoiceFloodMs->value * 0.001)) {
		//LOG_MESSAGE(PLID, "Block %f %f %f", gpGlobals->time, g_flLastReceivedVoice[iClient], gpGlobals->time - g_flLastReceivedVoice[iClient]);

		//((void (*)(client_t *, bool, const char *, ...))g_pDprotoAPI->p_SV_DropClient)(pClient, false, "Stop voice flooding!");

		return;
	}

	//LOG_MESSAGE(PLID, "Accept %f %f %f", gpGlobals->time, g_flLastReceivedVoice[iClient], gpGlobals->time - g_flLastReceivedVoice[iClient]);

	g_flLastReceivedVoice[iClient] = gpGlobals->time;

	if ( g_PlayerVCodec[ iClient + 1 ].m_voiceCodec == VOICECODEC_NONE) {
		return;
	}
	if (g_bIsntSpeex) {
		return;
	}
	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}

	if (g_PlayerVCodec[iClient + 1].m_voiceCodec == VOICECODEC_MILES_SPEEX) {
		nDecompressedSamples = g_pVoiceSpeex[iClient]->Decompress(chReceived, nDataLength, chDecompressed, sizeof(chDecompressed));

		if (g_pcvarVoiceVolumeSpeex->value != 1.0) {
			short *sSample = (short *)chDecompressed;

			for (int iSample = 0; iSample < nDecompressedSamples; iSample++)
			{
				*sSample = min(max(*sSample * g_pcvarVoiceVolumeSpeex->value, -32768), 32767);

				sSample++;
			}
		}

		nCompressedLength = g_pVoiceSilk[iClient]->Compress(chDecompressed, nDecompressedSamples, chCompressed, sizeof(chCompressed));
	} else {
		nDecompressedSamples = g_pVoiceSilk[iClient]->Decompress(chReceived, nDataLength, chDecompressed, sizeof(chDecompressed));

		if (g_pcvarVoiceVolumeSilk->value != 1.0) {
			short *sSample = (short *)chDecompressed;

			for (int iSample = 0; iSample < nDecompressedSamples; iSample++)
			{
				*sSample = min(max(*sSample * g_pcvarVoiceVolumeSilk->value, -32768), 32767);

				sSample++;
			}
		}

		nCompressedLength = g_pVoiceSpeex[iClient]->Compress(chDecompressed, nDecompressedSamples, chCompressed, sizeof(chCompressed));
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		qboolean bLocal;
		int nSendLength;
		int nSend;
		byte *chSend;

		if (g_PlayerVCodec[i + 1].m_voiceCodec == VOICECODEC_NONE) {
			continue;
		}

		pDestClient = (client_t *)((size_t)g_psvs->m_pClients + g_sizeClientStruct * i);
		bLocal = (pDestClient == pClient);

		// Does the game code want cl sending to this client?
		if(!(pClient->m_bsVoiceStreams[i>>5] & (1 << (i & 31))) && !bLocal)
			continue;

		// Is this client even on the server?
		if(!pDestClient->m_fActive && !pDestClient->m_fConnected && !bLocal)
			continue;

		// Is loopback enabled?
		if (g_PlayerVCodec[iClient + 1].m_voiceCodec == g_PlayerVCodec[i + 1].m_voiceCodec) {
			nSend = nDataLength;
			chSend = chReceived;
		} else {
			nSend = nCompressedLength;
			chSend = chCompressed;
		}
		nSendLength = nSend;
		if(bLocal && !pDestClient->m_bLoopback)
		{
			nSendLength = 0;	// Still send something, just zero length (this is so the client 
			// can display something that shows knows the server knows it's talking).
		}

		// Is there room to write this data in?
		if( (6 + nSend + pDestClient->m_Datagram.cursize) < pDestClient->m_Datagram.maxsize )
		{
			MSG_WriteByte( &pDestClient->m_Datagram, SVC_VOICEDATA );
			MSG_WriteByte( &pDestClient->m_Datagram, iClient );
			MSG_WriteShort( &pDestClient->m_Datagram, nSendLength );
			MSG_WriteBuf( &pDestClient->m_Datagram, nSendLength, chSend );
		}
	}
}

qboolean ClientConnect_Pre ( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] ) {
	char szCommand[ 256 ];
	int iId, iProtocol;

	iId = ENTINDEX(pEntity);

	g_PlayerVCodec[ iId ].m_voiceCodec = VOICECODEC_NONE;
	g_PlayerVCodec[ iId ].m_fIsRequested = false;

	sprintf( szCommand, "dp_clientinfo %d\n", iId );
	SERVER_COMMAND( szCommand );
	SERVER_EXECUTE( );

	iProtocol = (int)CVAR_GET_FLOAT( "dp_r_protocol" );

	switch ( iProtocol ) {
	case 47:
		g_PlayerVCodec[ iId ].m_voiceCodec = VOICECODEC_MILES_SPEEX;

		break;
	case 48:
		g_PlayerVCodec[ iId ].m_fIsRequested = true;
		g_PlayerVCodec[ iId ].m_iRequestID = MAKE_REQUESTID( PLID );

		QUERY_CLIENT_CVAR_VALUE2( pEntity, "sv_version", g_PlayerVCodec[ iId ].m_iRequestID );

		break;
	}

	if ( g_PlayerVCodec[ iId ].m_voiceCodec != VOICECODEC_NONE ) {
		//LOG_MESSAGE( PLID, "Connected(%d) (%s) from(%s) protocol(%d) vcodec(Miles/Speex)", iId, pszName, pszAddress, iProtocol );
	} else {
		//LOG_MESSAGE( PLID, "Connected(%d) (%s) from(%s) protocol(%d) vcodec(Check...)", iId, pszName, pszAddress, iProtocol );
	}

	RETURN_META_VALUE( MRES_IGNORED, TRUE );
}

void StartFrame ( void ) {
	if (g_iOldVoiceQuality == g_pcvarVoiceQuality->value && strcmp(g_szOldVoiceCodec, g_pcvarVoiceCodec->string) == 0) {
		RETURN_META( MRES_IGNORED );
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i]->Init(g_pcvarVoiceQuality->value);
	}

	strncpy(g_szOldVoiceCodec, g_pcvarVoiceCodec->string, sizeof(g_szOldVoiceCodec));
	g_iOldVoiceQuality = g_pcvarVoiceQuality->value;

	if (strcmp(g_pcvarVoiceCodec->string, "voice_speex") != 0) {
		LOG_MESSAGE( PLID, "Warning vcodec isn't speex. Voice disabled." );

		g_bIsntSpeex = true;
	} else {
		g_bIsntSpeex = false;
	}

	RETURN_META( MRES_IGNORED );
}

void CvarValue2_Pre ( const edict_t *pEnt, int iRequestID, const char *pszCvarName, const char *pszValue ) {
	int iId, iBuild;
	const char *pszBuild;

	iId = ENTINDEX( pEnt );

	if ( !g_PlayerVCodec[ iId ].m_fIsRequested ) {
		RETURN_META( MRES_IGNORED );
	}
	if ( iRequestID != g_PlayerVCodec[ iId ].m_iRequestID ) {
		RETURN_META( MRES_IGNORED );
	}
	g_PlayerVCodec[ iId ].m_fIsRequested = false;

	//////////////////////////////////////////////////////////////////////////
	// Parse BuildNumber in sv_version cvar
	// 
	// Struct is:
	// GameVersion,ProtocolVersion,BuildNumber
	// 
	// Examples:
	// 1.1.2.6/2.0.0.0,48,4554
	// 1.1.2.7/Stdio,48,6027
	//////////////////////////////////////////////////////////////////////////
	pszBuild = strrchr( pszValue, ',' );
	if ( pszBuild == NULL ) {
		RETURN_META( MRES_IGNORED );
	}
	pszBuild++;
	iBuild = atoi( pszBuild );

	if ( iBuild >= 6027 ) {
		g_PlayerVCodec[ iId ].m_voiceCodec = VOICECODEC_SILK;
	} else {
		g_PlayerVCodec[ iId ].m_voiceCodec = VOICECODEC_MILES_SPEEX;
	}

	if (g_PlayerVCodec[ iId ].m_voiceCodec == VOICECODEC_MILES_SPEEX) {
		//LOG_MESSAGE( PLID, "Checked(%d) build(%d) vcodec(Miles/Speex)", iId, iBuild );
	} else {
		//LOG_MESSAGE( PLID, "Checked(%d) build(%d) vcodec(Silk)", iId, iBuild );
	}

	RETURN_META( MRES_IGNORED );
}
