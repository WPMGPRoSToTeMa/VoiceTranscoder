#pragma once

#include "Util.h"
#include "CRC32.h"
#include "VoiceCodec.h"
#include "VoiceCodec_Speex.h"
#include "VoiceCodec_SILK.h"
#include "SteamID.h"
#include <rehlds_api.h>

// Structs and classes
struct clientData_t {
	bool hasNewCodec;
	bool m_fIsChecking;
	bool m_fIsVguiRunScriptRecvd;
	int64_t m_nextPacketTimeMicroSeconds;
	VoiceCodec *m_pNewCodec;
	VoiceCodec *m_pOldCodec;
};

// VoicePacketCommand
enum : size_t {
	VPC_VDATA_SILENCE = 0,
	VPC_VDATA_MILES = 1, // Really unknown, deprecated
	VPC_VDATA_SPEEX = 2, // Deprecated
	VPC_VDATA_RAW = 3,
	VPC_VDATA_SILK = 4,
	VPC_UNKNOWN = 10,
	VPC_SETSAMPLERATE = 11
};
const size_t SVC_STUFFTEXT = 9;
const size_t SVC_VOICEDATA = 53;
const size_t MAX_VOICEPACKET_SIZE = 2048; // or 4096? or 8192?
const size_t MAX_DECOMPRESSED_VOICEPACKET_SIZE = 4096; // or 8192?
const size_t MAX_CLIENTS = 32;
// uint64_t(steamid) + uint8_t(VPC_SETSAMPLERATE or VPC_VDATA_SILK or VPC_VDATA_SILENCE) + uint16_t(arg) + ... + uint32_t(CRC32 checksum)
const size_t MIN_VOICEPACKET_SIZE = sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t);
const size_t CLC_VOICEDATA = 8;

// Externs
extern clientData_t g_rgClientData[MAX_CLIENTS];
extern client_t *g_firstClientPtr;
extern size_t g_clientStructSize;

extern cvar_t *g_pcvarVolumeOldToNew;
extern cvar_t *g_pcvarVolumeNewToOld;

extern bool g_isUsingRehldsAPI;
extern IRehldsApi *g_pRehldsAPI;

extern void (* g_pfnSvDropClient)(client_t *pClient, bool fCrash, char *pszFormat, ...);

extern void ClientCommand_PostHook(edict_t *pClient);
extern qboolean ClientConnect_PostHook(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason);
extern void ServerActivate_PostHook(edict_t *pEdictList, int nEdictCount, int nClientMax);
extern void StartFrame_PostHook();
extern void VTC_InitCvars(void);
extern void VTC_ExecConfig(void);
extern void VTC_InitConfig(void);
extern void VTC_InitCodecs(void);
//extern bool EngineParser(void);
extern void Hacks_Init();
extern void Hacks_Deinit();

extern void ChangeSamplesVolume(int16_t *samples, size_t sampleCount, double volume);