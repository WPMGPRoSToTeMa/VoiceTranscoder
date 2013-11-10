#pragma once

#include <extdll.h>

#include <dllapi.h>
#include <meta_api.h>

#include "util.h"

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

struct sv_clcfuncs_t {
	unsigned int iMsgId;
	const char *pszMsgName;
	void (* pfnCallback)(client_t *pClient);
};

#define CLC_VOICEDATA	8

#define SVC_VOICEDATA	53

extern void SV_ParseVoiceData( client_t *pClient );

extern size_t g_sizeClientStruct;

extern server_static_t *g_psvs;

extern playervcodec_t g_PlayerVCodec[ MAX_CLIENTS+1 ];

extern IVoiceCodec *g_pVoiceSpeex[ MAX_CLIENTS ];
extern IVoiceCodec *g_pVoiceSilk[ MAX_CLIENTS ];

extern cvar_t *g_pcvarVoiceEnable;
extern cvar_t *g_pcvarVoiceCodec;
extern cvar_t *g_pcvarVoiceQuality;

extern qboolean VCM_Init( void );
extern qboolean VCM_End( void );

extern void ParseSizeClientStruct(void);

extern qboolean ClientConnect_Pre ( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
extern void StartFrame ( void );
extern void CvarValue2_Pre ( const edict_t *pEnt, int iRequestID, const char *pszCvarName, const char *pszValue );