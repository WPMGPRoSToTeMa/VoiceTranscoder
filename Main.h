#pragma once

#include "Util.h"
#include "VoiceCodec.h"

// Structs and classes
struct clientData_t {
	bool hasNewCodec;
	bool m_fIsChecking;
	bool m_fIsVguiRunScriptRecvd;
	int64_t m_nextPacketTimeMicroSeconds;
	VoiceCodec *m_pNewCodec;
	VoiceCodec *m_pOldCodec;
};

const size_t MAX_CLIENTS = 32;

// Externs
extern clientData_t g_rgClientData[MAX_CLIENTS];
extern client_t *g_firstClientPtr;
extern size_t g_clientStructSize;

extern void ClientCommand_PostHook(edict_t *pClient);
extern bool32_t ClientConnect_PostHook(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason);
extern void ServerActivate_PostHook(edict_t *pEdictList, int nEdictCount, int nClientMax);
extern void StartFrame_PostHook();
extern void VTC_InitCvars(void);
extern void VTC_ExecConfig(void);
extern void VTC_InitConfig(void);
extern void VTC_InitCodecs(void);
//extern bool EngineParser(void);
extern void Hacks_Init();
extern void Hacks_Deinit();

extern void MSG_WriteUInt8_UnSafe(sizebuf_t *buf, uint8_t value);
extern void MSG_WriteUInt16_UnSafe(sizebuf_t *buf, uint16_t value);
extern void MSG_WriteBuf_UnSafe(sizebuf_t *buf, void *binBuf, size_t byteCount);
extern size_t MSG_GetRemainBytesCount(sizebuf_t *buf);