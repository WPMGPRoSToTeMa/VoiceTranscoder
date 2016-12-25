#pragma once

#include "VoiceCodec_Speex.h"
#include "VoiceCodec_SILK.h"
#include <EngineUTIL.h>
#include <rehlds_api.h>
#include <vector>
#include <memory>

// Structs and classes
struct clientData_t {
	bool m_hasNewCodec;
	bool m_isChecking;
	bool m_isVguiRunScriptReceived;
	bool m_isSpeaking;
	bool m_isMuted;
	uint64_t m_nextPacketTimeMicroSeconds;
	VoiceCodec_SILK *m_pNewCodec;
	VoiceCodec_Speex *m_pOldCodec;
};

// TODO: samples16k
struct playSound_t {
	std::vector<int16_t> samples8k;
	std::vector<int16_t> samples16k;
	size_t currentSample;
	client_t *receiver;
	float nextTime;
	std::unique_ptr<VoiceCodec_Speex> oldCodec;
	std::unique_ptr<VoiceCodec_SILK> newCodec;
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

// Constants
const size_t MAX_VOICEPACKET_SIZE = 8192; // or 4096? or 8192?
const size_t MAX_DECOMPRESSED_VOICEPACKET_SAMPLES = 32768; // or 8192?
// uint64_t(steamid) + uint8_t(VPC_SETSAMPLERATE or VPC_VDATA_SILK or VPC_VDATA_SILENCE) + uint16_t(arg) + ... + uint32_t(CRC32 checksum)
const size_t MIN_VOICEPACKET_SIZE = sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t);
const char VTC_CONFIGNAME[] = "VoiceTranscoder.cfg";
const size_t NEWCODEC_WANTED_SAMPLERATE = 16000;
const uint64_t SPEAKING_TIMEOUT = 200000;

// Externs
extern std::vector<playSound_t> g_playSounds;
extern clientData_t g_clientData[MAX_CLIENTS];

extern cvar_t *g_pcvarForceSendHLTV;
extern cvar_t *g_pcvarVolumeOldToNew;
extern cvar_t *g_pcvarVolumeNewToOld;

extern void OnClientCommandReceiving(edict_t *pClient);
extern qboolean OnClientConnected(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason);
extern void OnClientDisconnected(edict_t *pClient);
extern void OnServerActivated(edict_t *pEdictList, int nEdictCount, int nClientMax);
extern void OnFrameStarted();

extern void SV_ParseVoiceData_Hook(client_t *client);
extern void HandleNetCommand_Hook(IRehldsHook_HandleNetCommand *chain, IGameClient *client, uint8_t netcmd);

extern void VTC_InitCvars(void);
extern void VTC_InitAPI(void);
extern void VTC_ExecConfig(void);
extern void VTC_InitConfig(void);
extern void VTC_InitCodecs(void);
extern void VTC_UpdateCodecs();