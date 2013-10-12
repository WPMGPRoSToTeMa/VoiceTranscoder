#pragma once

#include <extdll.h>

#include <dllapi.h>
#include <meta_api.h>

#include "util.h"

#include <dynamiclibrary.h>
#include <hooker.h>

#include "ivoicecodec.h"
#include "VoiceEncoder_Silk.h"
#include "VoiceEncoder_Speex.h"

#define MAX_CLIENTS	32

enum voicecodec_t {
	VOICECODEC_NONE = NULL,
	VOICECODEC_MILES_SPEEX,
	VOICECODEC_SILK
};

struct playervcodec_t {
	voicecodec_t	m_voiceCodec;
	bool			m_fIsRequested;
	int				m_iRequestID;
};

#ifdef _WIN32
#define SEARCH_SV_PARSEVOICEDATA	0xAB33F
#define SEARCH_SV_WRITEVOICECODEC	0x9D200
#define SEARCH_MSG_READSHORT		0x36830
#define SEARCH_MSG_READBUF			0x36980
#define SEARCH_MSG_WRITEBYTE		0x35BD0
#define SEARCH_MSG_WRITESHORT		0x35BF0
#define SEARCH_MSG_WRITEBUF			0x35CD0
#define SEARCH_SVS					0x4362C0
#else
#define SEARCH_SV_PARSEVOICEDATA	"SV_ParseVoiceData"
#define SEARCH_SV_WRITEVOICECODEC	"SV_WriteVoiceCodec"
#define SEARCH_MSG_READSHORT		"MSG_ReadShort"
#define SEARCH_MSG_READBUF			"MSG_ReadBuf"
#define SEARCH_MSG_WRITEBYTE		"MSG_WriteByte"
#define SEARCH_MSG_WRITESHORT		"MSG_WriteShort"
#define SEARCH_MSG_WRITEBUF			"MSG_WriteBuf"
#define SEARCH_SVS					"svs"
#endif

typedef void ( SV_PARSEVOICEDATA )( client_t *pClient );
typedef void ( SV_WRITEVOICECODEC )( sizebuf_t *pDatagram );
typedef int ( MSG_READSHORT )( void );
typedef void ( MSG_READBUF )( size_t, char * );
typedef void ( MSG_WRITEBYTE )( sizebuf_t *pDatagram, int );
typedef void ( MSG_WRITESHORT )( sizebuf_t *pDatagram, int );
typedef void ( MSG_WRITEBUF )( sizebuf_t *pDatagram, size_t, char * );

extern void SV_ParseVoiceData( client_t *pClient );
extern void SV_WriteVoiceCodec( sizebuf_t *pDatagram );

extern DYNLIB *g_pdlEngine;

extern HOOKER< SV_PARSEVOICEDATA > *g_phookParseVoiceData;
extern HOOKER< SV_WRITEVOICECODEC > *g_phookWriteVoiceCodec;

extern MSG_READSHORT *g_pfnReadShort;
extern MSG_READBUF *g_pfnReadBuf;
extern MSG_WRITEBYTE *g_pfnWriteByte;
extern MSG_WRITESHORT *g_pfnWriteShort;
extern MSG_WRITEBUF *g_pfnWriteBuf;

extern server_static_t *g_psvs;

extern playervcodec_t g_PlayerVCodec[ MAX_CLIENTS+1 ];

extern IVoiceCodec *g_pVoiceSpeex[ MAX_CLIENTS ];
extern IVoiceCodec *g_pVoiceSilk[ MAX_CLIENTS ];

extern cvar_t *g_pcvarVoiceEnable;
extern cvar_t *g_pcvarVoiceCodec;
extern cvar_t *g_pcvarVoiceQuality;

extern qboolean VCM_Init( void );
extern qboolean VCM_End( void );

extern qboolean ClientConnect_Pre ( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
extern void CvarValue2_Pre ( const edict_t *pEnt, int iRequestID, const char *pszCvarName, const char *pszValue );