#pragma once

#include "Util.h"
#include "VoiceCodec.h"

// Typedefs
typedef void SV_DROPCLIENT(client_t *pClient, bool fCrash, char *pszFormat, ...);
typedef void *SZ_GETSPACE(sizebuf_t *pBuf, size_t nCountBytes);

// Structs and classes
struct clientData_t {
	bool m_fHasNewCodec;
	bool m_fIsChecking;
	bool m_fIsLogSecretRecvd;
	longlong m_llNextPacketUsec;
	VoiceCodec *m_pNewCodec;
	VoiceCodec *m_pOldCodec;
};

// Externs
extern void ClientCommand_Pre(edict_t *pClient);
extern qboolean ClientConnect_Post(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason);
extern void ServerActivate_Post(edict_t *pEdictList, int nEdictCount, int nClientMax);
extern void VTC_InitCvars(void);
extern void VTC_ExecConfig(void);
extern void VTC_InitConfig(void);
extern void VTC_InitCodecs(void);
extern bool EngineParser(void);