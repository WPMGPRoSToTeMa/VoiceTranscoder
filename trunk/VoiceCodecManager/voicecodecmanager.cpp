#include "voicecodecmanager.h"

DYNLIB *g_pdlEngine;

HOOKER< SV_PARSEVOICEDATA > *g_phookParseVoiceData;
HOOKER< SV_WRITEVOICECODEC > *g_phookWriteVoiceCodec;

MSG_READSHORT *g_pfnReadShort;
MSG_READBUF *g_pfnReadBuf;
MSG_WRITEBYTE *g_pfnWriteByte;
MSG_WRITESHORT *g_pfnWriteShort;
MSG_WRITEBUF *g_pfnWriteBuf;

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

bool g_bIsMiles;

qboolean VCM_Init( void ) {
	g_pdlEngine = new DYNLIB( (void *)g_engfuncs.pfnPrecacheModel );

	ParseSizeClientStruct();

	g_phookParseVoiceData = new HOOKER<SV_PARSEVOICEDATA>(g_pdlEngine->FindAddr(SEARCH_SV_PARSEVOICEDATA), SV_ParseVoiceData);
	g_phookWriteVoiceCodec = new HOOKER<SV_WRITEVOICECODEC>(g_pdlEngine->FindAddr(SEARCH_SV_WRITEVOICECODEC), SV_WriteVoiceCodec);

	g_pfnReadShort = (MSG_READSHORT *)g_pdlEngine->FindAddr(SEARCH_MSG_READSHORT);
	g_pfnReadBuf = (MSG_READBUF *)g_pdlEngine->FindAddr(SEARCH_MSG_READBUF);
	g_pfnWriteByte = (MSG_WRITEBYTE *)g_pdlEngine->FindAddr(SEARCH_MSG_WRITEBYTE);
	g_pfnWriteShort = (MSG_WRITESHORT *)g_pdlEngine->FindAddr(SEARCH_MSG_WRITESHORT);
	g_pfnWriteBuf = (MSG_WRITEBUF *)g_pdlEngine->FindAddr(SEARCH_MSG_WRITEBUF);

	g_psvs = (server_static_t *)g_pdlEngine->FindAddr(SEARCH_SVS);

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

	if (!strcmp(g_pcvarVoiceCodec->string, "voice_miles")) {
		LOG_MESSAGE( PLID, "Warning vcodec is miles. Voice disabled." );

		g_bIsMiles = true;
	} else {
		g_bIsMiles = false;
	}

	return TRUE;
}

qboolean VCM_End( void ) {
	delete g_phookParseVoiceData;
	delete g_phookWriteVoiceCodec;

	delete g_pdlEngine;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i]->Release();
		g_pVoiceSilk[i]->Release();
	}

	return TRUE;
}

void ParseSizeClientStruct(void) {
#ifdef _WIN32
	g_sizeClientStruct = sizeof(client_t);
#else
	void *pAddr = g_pdlEngine->FindAddr(SV_CHECKTIMEOUTS);

	pAddr = g_pdlEngine->FindAddr(pAddr, "\x81\x00\x00\x00\x00\x00", "\xFF\x00\x00\x00\xFF\xFF", 6);

	g_sizeClientStruct = *(size_t *)((size_t)pAddr + 2);

	LOG_MESSAGE( PLID, "Size of client_t structure is %d", g_sizeClientStruct );
#endif
}

void SV_WriteVoiceCodec( sizebuf_t *pDatagram ) {
	g_phookWriteVoiceCodec->Unhook( );
	g_phookWriteVoiceCodec->m_pfnOrig( pDatagram );
	g_phookWriteVoiceCodec->Hook( );

	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_pVoiceSpeex[i]->Init(g_pcvarVoiceQuality->value);
	}

	if (!strcmp(g_pcvarVoiceCodec->string, "voice_miles")) {
		LOG_MESSAGE( PLID, "Warning vcodec is miles. Voice disabled." );

		g_bIsMiles = true;
	} else {
		g_bIsMiles = false;
	}
}

void SV_ParseVoiceData(client_t *pClient) {
	int i, iClient;
	int nDataLength, nCompressedLength, nDecompressedSamples;
	char chReceived[4096];
	char chDecompressed[8192];
	char chCompressed[4096];
	client_t *pDestClient;

	iClient = ((size_t)pClient - (size_t)g_psvs->m_pClients) / g_sizeClientStruct;

	nDataLength = g_pfnReadShort( );

	if ( nDataLength > sizeof( chReceived ) ) {
		return;
	}

	g_pfnReadBuf( nDataLength, chReceived );

	if ( g_PlayerVCodec[ iClient + 1 ].m_voiceCodec == VOICECODEC_NONE) {
		return;
	}
	if (g_bIsMiles) {
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

		//nDataLength = g_pVoiceSpeex[iClient]->Compress(chDecompressed, nDecompressedSamples, chReceived, sizeof(chReceived), false);

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
			g_pfnWriteByte( &pDestClient->m_Datagram, 53 );
			g_pfnWriteByte( &pDestClient->m_Datagram, iClient );
			g_pfnWriteShort( &pDestClient->m_Datagram, nSendLength );
			g_pfnWriteBuf( &pDestClient->m_Datagram, nSendLength, chSend );
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
