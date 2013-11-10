#include "voicecodecmanager.h"
#include "dproto_api.h"

void (* g_pfnDefCallback)(client_t *);

size_t g_sizeClientStruct;

server_static_t *g_psvs;

playervcodec_t g_PlayerVCodec[ MAX_CLIENTS+1 ];

IVoiceCodec *g_pVoiceSpeex[ MAX_CLIENTS ];
IVoiceCodec *g_pVoiceSilk[ MAX_CLIENTS ];

cvar_t g_cvarVoiceVolumeSpeex = {"sv_voicevolume_speex", "1.0", FCVAR_EXTDLL};
cvar_t g_cvarVoiceVolumeSilk = {"sv_voicevolume_silk", "1.0", FCVAR_EXTDLL};

cvar_t *g_pcvarVoiceEnable;
cvar_t *g_pcvarVoiceCodec;
cvar_t *g_pcvarVoiceQuality;
cvar_t *g_pcvarVoiceVolumeSpeex;
cvar_t *g_pcvarVoiceVolumeSilk;

size_t *pmsg_readcount;
int *pmsg_badread;
sizebuf_t *pnet_message;
void * (* SZ_GetSpace)(sizebuf_t *, size_t);

bool g_bIsntSpeex;

dp_enginfo_api_t *g_pDpApi;

char g_szOldVoiceCodec[12];
int g_iOldVoiceQuality;

qboolean VCM_Init( void ) {
	// Get dproto API
	sscanf(CVAR_GET_STRING(DPROTO_API_CVAR_NAME), "%u", &g_pDpApi);

	// Check major ver
	if (g_pDpApi->version_major != DPROTO_ENGINFO_API_VERSION_MAJOR) {
		LOG_ERROR(PLID, "DPAPI major version checking failed: current %d (needed %d)", g_pDpApi->version_major, DPROTO_ENGINFO_API_VERSION_MAJOR);

		return false;
	}
	// Check minor ver
	if (g_pDpApi->version_minor < DPROTO_ENGINFO_API_VERSION_MINOR) {
		LOG_ERROR(PLID, "DPAPI minor version checking failed: current %d (needed minimal %d)", g_pDpApi->version_minor, DPROTO_ENGINFO_API_VERSION_MINOR);

		return false;
	}

	// Change clc_voicedata callback
	sv_clcfuncs_t *pCLC = (sv_clcfuncs_t *)g_pDpApi->p_clc_funcs;

	g_pfnDefCallback = pCLC[CLC_VOICEDATA].pfnCallback;
	pCLC[CLC_VOICEDATA].pfnCallback = &SV_ParseVoiceData;

	// Get size client_t
	g_sizeClientStruct = g_pDpApi->client_t_size;

	// Get svs
	g_psvs = (server_static_t *)g_pDpApi->p_svs;

	// ...
	pmsg_readcount = (size_t *)g_pDpApi->p_msg_readcount;
	pmsg_badread = g_pDpApi->p_msg_badread;
	pnet_message = (sizebuf_t *)g_pDpApi->p_net_message_addr;
	SZ_GetSpace = (void *(*)(sizebuf_t *, size_t))g_pDpApi->p_SZ_GetSpace;

	CVAR_REGISTER(&g_cvarVoiceVolumeSpeex);
	CVAR_REGISTER(&g_cvarVoiceVolumeSilk);

	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");
	g_pcvarVoiceCodec = CVAR_GET_POINTER("sv_voicecodec");
	g_pcvarVoiceQuality = CVAR_GET_POINTER("sv_voicequality");
	g_pcvarVoiceVolumeSpeex = CVAR_GET_POINTER("sv_voicevolume_speex");
	g_pcvarVoiceVolumeSilk = CVAR_GET_POINTER("sv_voicevolume_silk");

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i] = CreateSpeexVoiceCodec();

		if ( !g_pVoiceSpeex[i] ) {
			LOG_ERROR( PLID, "Couldn't get speex interface" );

			return FALSE;
		}

		if ( !g_pVoiceSpeex[i]->Init(g_pcvarVoiceQuality->value) ) {
			LOG_ERROR( PLID, "Couldn't initialize speex interface" );

			return FALSE;
		}
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		g_pVoiceSilk[i] = CreateSilkVoiceCodec();

		if ( !g_pVoiceSilk[i] ) {
			LOG_ERROR( PLID, "Couldn't get silk interface" );

			return FALSE;
		}

		if ( !g_pVoiceSilk[i]->Init(0) ) {
			LOG_ERROR( PLID, "Couldn't initialize silk interface" );

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

qboolean VCM_End( void ) {
	// Change clc_voicedata callback
	sv_clcfuncs_t *pCLC = (sv_clcfuncs_t *)g_pDpApi->p_clc_funcs;

	pCLC[CLC_VOICEDATA].pfnCallback = g_pfnDefCallback;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i]->Release();
		g_pVoiceSilk[i]->Release();
	}

	return TRUE;
}

int MSG_ReadShort(void) {
	short iResult;

	if ( *pmsg_readcount + sizeof(short) > pnet_message->cursize )
	{
		*pmsg_badread = 1;

		iResult = -1;
	}
	else
	{
		iResult = *(short *)&(pnet_message->data[*pmsg_readcount]);

		*pmsg_readcount += sizeof(short);
	}

	return iResult;
}

int MSG_ReadBuf(size_t size, void *pDest) {
	int iResult;

	if ( *pmsg_readcount + size > pnet_message->cursize )
	{
		*pmsg_badread = 1;

		iResult = -1;
	}
	else
	{
		memcpy(pDest, &pnet_message->data[*pmsg_readcount], size);

		*pmsg_readcount += size;

		iResult = 1;
	}

	return iResult;
}

void MSG_WriteByte(sizebuf_t *pDatagram, byte bVal) {
	byte *pbData = (byte *)SZ_GetSpace(pDatagram, sizeof(byte));

	*pbData = bVal;
}

void MSG_WriteShort(sizebuf_t *pDatagram, short sVal) {
	short *psData = (short *)SZ_GetSpace(pDatagram, sizeof(short));

	*psData = sVal;
}

void MSG_WriteBuf(sizebuf_t *pDatagram, size_t size, void *pBuf) {
	void *pData = SZ_GetSpace(pDatagram, size);

	memcpy(pData, pBuf, size);
}

void SV_ParseVoiceData(client_t *pClient) {
	int i, iClient;
	int nDataLength, nCompressedLength, nDecompressedSamples;
	char chReceived[4096];
	char chDecompressed[8192];
	char chCompressed[4096];
	client_t *pDestClient;

	iClient = ((size_t)pClient - (size_t)g_psvs->m_pClients) / g_sizeClientStruct;

	nDataLength = MSG_ReadShort( );

	if ( nDataLength > sizeof( chReceived ) ) {
		return;
	}

	MSG_ReadBuf( nDataLength, chReceived );

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

		nCompressedLength = g_pVoiceSilk[iClient]->Compress(chDecompressed, nDecompressedSamples, chCompressed, sizeof(chCompressed), false);
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

		nCompressedLength = g_pVoiceSpeex[iClient]->Compress(chDecompressed, nDecompressedSamples, chCompressed, sizeof(chCompressed), false);
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		qboolean bLocal;
		int nSendLength;
		int nSend;
		char *chSend;

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
		LOG_MESSAGE( PLID, "Connected(%d) (%s) from(%s) protocol(%d) vcodec(Miles/Speex)", iId, pszName, pszAddress, iProtocol );
	} else {
		LOG_MESSAGE( PLID, "Connected(%d) (%s) from(%s) protocol(%d) vcodec(Check...)", iId, pszName, pszAddress, iProtocol );
	}

	RETURN_META_VALUE( MRES_IGNORED, TRUE );
}

void StartFrame ( void ) {
	if (g_iOldVoiceQuality == g_pcvarVoiceQuality->value && stricmp(g_szOldVoiceCodec, g_pcvarVoiceCodec->string) == 0) {
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
		LOG_MESSAGE( PLID, "Checked(%d) build(%d) vcodec(Miles/Speex)", iId, iBuild );
	} else {
		LOG_MESSAGE( PLID, "Checked(%d) build(%d) vcodec(Silk)", iId, iBuild );
	}

	RETURN_META( MRES_IGNORED );
}
