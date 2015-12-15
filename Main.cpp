#include <extdll.h>
#include <meta_api.h>
#include "MultiThread.h"
#include "NetMsgBuf.h"
#include "Util.h"
#include "Timer.h"
#include "Buffer.h"
#include "ThreadMode.h"
#include "Main.h"
#include "MetaUTIL.h"
#include "EngineUTIL.h"
#include "Library.h"
#include <rehlds_api.h>

#ifdef WIN32
	#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#endif

// Constants
const char *VTC_CONFIGNAME = "VoiceTranscoder.cfg";
const size_t WANTED_SAMPLERATE = 16000;

// Externs
extern size_t *g_pnMsgReadCount;
extern bool *g_pfMsgBadRead;

// Variables
clientData_t g_rgClientData[MAX_CLIENTS];

char g_szExecConfigCmd[300];

bool g_fThreadModeEnabled;

// Hacked
Library *g_pEngine;
size_t g_clientStructSize;
size_t *g_pnMsgReadCount;
bool *g_pfMsgBadRead;
NetMsgBuf g_netMessage;
void (* g_pfnSvDropClient)(client_t *pClient, bool fCrash, char *pszFormat, ...);
client_t *g_firstClientPtr;

bool g_isUsingRehldsAPI;
IRehldsApi *g_pRehldsAPI;

// Cvars
cvar_t g_cvarVersion = {"VTC_Version", Plugin_info.version, FCVAR_EXTDLL | FCVAR_SERVER, 0, nullptr};
cvar_t *g_pcvarVersion;
cvar_t g_cvarDefaultCodec = {"VTC_DefaultCodec", "old", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarDefaultCodec;
cvar_t g_cvarHltvCodec = {"VTC_HltvCodec", "old", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarHltvCodec;
cvar_t g_cvarThreadMode = {"VTC_ThreadMode", "0", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarThreadMode;
cvar_t g_cvarMaxDelta = {"VTC_MaxDelta", "200", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarMaxDelta;
cvar_t g_cvarVolumeOldToNew = {"VTC_Volume_OldToNew", "1.0", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarVolumeOldToNew;
cvar_t g_cvarVolumeNewToOld = {"VTC_Volume_NewToOld", "1.0", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarVolumeNewToOld;
cvar_t *g_pcvarVoiceEnable;

// Engine API
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEngFuncs, globalvars_t *pGlobalVars) {
	memcpy(&g_engfuncs, pEngFuncs, sizeof(g_engfuncs));
	gpGlobals = pGlobalVars;
}

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientCommand = &ClientCommand_PostHook;

	return TRUE;
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientConnect = &ClientConnect_PostHook;
	pFunctionTable->pfnServerActivate = &ServerActivate_PostHook;
	pFunctionTable->pfnStartFrame = &StartFrame_PostHook;

	return TRUE;
}

// Entity API
void ClientCommand_PostHook(edict_t *pClient) {
	const char *pszCmd = CMD_ARGV(0);

	int nClientIndex = ENTINDEX(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex-1];

	if (FStrEq(pszCmd, "VTC_CheckStart")) {
		pClientData->m_fIsChecking = true;
		pClientData->hasNewCodec = false;
		pClientData->m_fIsVguiRunScriptRecvd = false;
	} else if (pClientData->m_fIsChecking) {
		if (FStrEq(pszCmd, "vgui_runscript")) {
			pClientData->m_fIsVguiRunScriptRecvd = true;
		} else if (FStrEq(pszCmd, "VTC_CheckEnd")) {
			pClientData->m_fIsChecking = false;
			pClientData->hasNewCodec = pClientData->m_fIsVguiRunScriptRecvd ? true : false;
			pClientData->m_fIsVguiRunScriptRecvd = false;

			LOG_MESSAGE(PLID, "Client with %s codec connected", pClientData->hasNewCodec ? "new" : "old");
		}
	}

	RETURN_META(MRES_IGNORED);
}

// TODO: bool32_t
qboolean ClientConnect_PostHook(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason) {
	int nClientIndex = ENTINDEX(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex-1];

	// Default client codec
	if (FStrEq(GETPLAYERAUTHID(pClient), "HLTV")) {
		pClientData->hasNewCodec = FStrEq(g_pcvarHltvCodec->string, "old") ? false : true;
		// Add print?
	} else {
		pClientData->hasNewCodec = FStrEq(g_pcvarDefaultCodec->string, "old") ? false : true;
	}
	pClientData->m_fIsChecking = false;

	RETURN_META_VALUE(MRES_IGNORED, META_RESULT_ORIG_RET(bool32_t));
}

void ServerActivate_PostHook(edict_t *pEdictList, int nEdictCount, int nClientMax) {
	// It is bad because it sends to hltv
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("VTC_CheckStart\n");
	MESSAGE_END();
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("vgui_runscript\n");
	MESSAGE_END();
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("VTC_CheckEnd\n");
	MESSAGE_END();

	VTC_ExecConfig();
	VTC_InitCodecs();

	if (!g_isUsingRehldsAPI) {
		if (!g_clientStructSize) {
			g_clientStructSize = size_t(g_engfuncs.pfnGetInfoKeyBuffer(g_engfuncs.pfnPEntityOfEntIndex(2)) - g_engfuncs.pfnGetInfoKeyBuffer(g_engfuncs.pfnPEntityOfEntIndex(1))); // Asmodai idea
		}
		g_firstClientPtr = (decltype(g_firstClientPtr))(g_engfuncs.pfnGetInfoKeyBuffer(g_engfuncs.pfnPEntityOfEntIndex(1)) - offsetof(client_t, m_szUserInfo));
	}

	RETURN_META(MRES_IGNORED);
}

void StartFrame_PostHook() {
	if (g_fThreadModeEnabled) {
		VTC_ThreadVoiceFlusher();
	}

	RETURN_META(MRES_IGNORED);
}

// MetaMod API
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION, // ifvers
	"VoiceTranscoder",      // name
	"2.0 Reloaded",         // version
	"Dec 21 2014",          // date
	"WPMG.PR0SToCoder",     // author
	"http://vtc.wpmg.ru/",  // url
	"VTC",                  // logtag, all caps please
	PT_ANYTIME,             // (when) loadable
	PT_ANYTIME,             // (when) unloadable
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(char *pchInterfaceVersion, plugin_info_t **pPluginInfo, mutil_funcs_t *pMetaUtilFuncs) {
	*pPluginInfo = &Plugin_info;
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return TRUE;
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) {
	// Reset
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	gpMetaGlobals = pMGlobals;
	gpGamedllFuncs = pGamedllFuncs;

	pFunctionTable->pfnGetEntityAPI2 = &GetEntityAPI2;
	pFunctionTable->pfnGetEntityAPI2_Post = &GetEntityAPI2_Post;

	Hacks_Init();
	VTC_InitCvars();
	VTC_InitConfig();
	VTC_ExecConfig();
	VTC_InitCodecs();
	if (g_pcvarThreadMode->value != 0.0f) {
		g_fThreadModeEnabled = true;

		VTC_ThreadInit();
	}

	//EngineParser();

	return TRUE;
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
	if (g_fThreadModeEnabled) {
		VTC_ThreadDeinit();
	}
	Hacks_Deinit();

	return TRUE;
}

// AMXX API (TODO)

// Another code
/* forcehltv
 * forcesendvoicecodec*/

void ChangeSamplesVolume(int16_t *samples, size_t sampleCount, double volume) {
	if (volume == 1.0) {
		return;
	}

	for (size_t i = 0; i < sampleCount; i++) {
		double sample = samples[i] * volume;

		if (sample > 32767) {
			sample = 32767;
		} else if (sample < -32768) {
			sample = -32768;
		}

		double volumeChange = sample / samples[i];
		if (volumeChange < volume) {
			volume = volumeChange;
		}
	}

	if (volume == 1.0) {
		return;
	}

	for (size_t i = 0; i < sampleCount; i++) {
		samples[i] = (int16_t)(samples[i] * volume);
	}
}

// Flush remaining
// TODO: check for small packets
void SV_ParseVoiceData_Hook(client_t *client) {
	size_t clientIndex = EngineUTIL::GetClientIndex(client);
	clientData_t *pClientData = &g_rgClientData[clientIndex-1];

	size_t receivedBytesCount = g_netMessage.ReadUInt16();
	if (receivedBytesCount > MAX_VOICEPACKET_SIZE) {
		LOG_MESSAGE(PLID, "Oversize voice packet, size = %u (max = %u) from %s", receivedBytesCount, MAX_VOICEPACKET_SIZE, client->m_szPlayerName);
		EngineUTIL::DropClient(client, false, "Oversize voice packet, size = %u (max = %u)", receivedBytesCount, MAX_VOICEPACKET_SIZE);

		return;
	}

	uint8_t receivedBytes[MAX_VOICEPACKET_SIZE];
	g_netMessage.ReadBuf(receivedBytes, receivedBytesCount);

	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}
	if (!client->m_fActive) {
		return;
	}

	int64_t currentMicroSeconds = GetCurrentTimeInMicroSeconds();

	if (pClientData->m_nextPacketTimeMicroSeconds > currentMicroSeconds) {
		if ((pClientData->m_nextPacketTimeMicroSeconds - currentMicroSeconds)/1000.0 > g_pcvarMaxDelta->value) {
			//LOG_MESSAGE(PLID, "Delta is %g", (pClientData->m_nextPacketTimeMicroSeconds - currentMicroSeconds)/1000.0);

			return;
		}
	}

	bool needTranscode = false;
	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pDestClient = EngineUTIL::GetClientByIndex(i+1);

		if (pDestClient == client) {
			continue;
		} else {
			if (!pDestClient->m_fActive) {
				continue;
			}
			if (!(client->m_bsVoiceStreams[0] & (1 << i))) {
				continue;
			}
		}

		if (g_rgClientData[i].hasNewCodec != g_rgClientData[clientIndex-1].hasNewCodec) {
			needTranscode = true;

			break;
		}
	}

	int16_t rawSamples[MAX_DECOMPRESSED_VOICEPACKET_SIZE];
	size_t rawSampleCount;

	// Validate new codec packet
	if (pClientData->hasNewCodec) {
		if (receivedBytesCount < MIN_VOICEPACKET_SIZE) {
			LOG_MESSAGE(PLID, "Too small voice packet (real = %u, min = %u) from %s", receivedBytesCount, MIN_VOICEPACKET_SIZE, client->m_szPlayerName);
			EngineUTIL::DropClient(client, false, "Too small voice packet (real = %u, min = %u)", receivedBytesCount, MIN_VOICEPACKET_SIZE);

			return;
		}

		Buffer buf(receivedBytes, receivedBytesCount);

		CRC32 checksum;
		checksum.Init();
		checksum.Update(buf.PeekRead(), buf.Size() - sizeof(uint32_t));
		checksum.Final();

		buf.SeekRead(sizeof(uint32_t), SEEK_END);
		uint32_t dwRecvdChecksum = buf.ReadUInt32();

		if (checksum.ConvertToUInt32() != dwRecvdChecksum) {
			LOG_MESSAGE(PLID, "Voice packet checksum validation failed for %s", client->m_szPlayerName);
			EngineUTIL::DropClient(client, false, "Voice packet checksum validation failed");

			return;
		}

		// Go to start
		buf.RewindRead();

		// SteamID
		uint64_t qwSteamID = buf.ReadUInt64();

		// Validate SteamID
		SteamID steamid(qwSteamID);
		if (!steamid.IsValid()) {
			LOG_MESSAGE(PLID, "Invalid steamid (%llu) in voice packet from %s", steamid.ConvertToUInt64(), client->m_szPlayerName);
			EngineUTIL::DropClient(client, false, "Invalid steamid (%llu) in voice packet", steamid.ConvertToUInt64());

			return;
		}

		rawSampleCount = 0;
		while (buf.Size() - buf.TellRead() - sizeof(uint32_t) >= sizeof(uint8_t) + sizeof(uint16_t)) {
			uint8_t vpc = buf.ReadUInt8();

			switch (vpc) {
				case VPC_SETSAMPLERATE: {
					uint16_t sampleRate = buf.ReadUInt16();

					if (sampleRate != WANTED_SAMPLERATE) {
						LOG_MESSAGE(PLID, "Voice packet unwanted samplerate (cur = %u, want = %u) from %s", sampleRate, WANTED_SAMPLERATE, client->m_szPlayerName);
						EngineUTIL::DropClient(client, false, "Voice packet unwanted samplerate (cur = %u, want = %u)", sampleRate, WANTED_SAMPLERATE);

						return;
					}
				}
				break;
				case VPC_VDATA_SILENCE: {
					size_t silenceSampleCount = buf.ReadUInt16();

					if (silenceSampleCount > ARRAYSIZE(rawSamples) - rawSampleCount) {
						LOG_MESSAGE(PLID, "Too many silence samples (cur %u, max %u) from %s", rawSampleCount, ARRAYSIZE(rawSamples), client->m_szPlayerName);
						EngineUTIL::DropClient(client, false, "Too many silence samples (cur %u, max %u)", rawSampleCount, ARRAYSIZE(rawSamples), steamid.ConvertToUInt64());

						return;
					}

					/*memset(&rawSamples[rawSampleCount], 0, silenceSampleCount * sizeof(int16_t));
					rawSampleCount += silenceSampleCount;*/
				}
				break;
				case VPC_VDATA_SILK: {
					uint16_t bytesCount = buf.ReadUInt16();

					size_t remainBytes = buf.Size() - buf.TellRead() - sizeof(uint32_t);
					if (remainBytes >= bytesCount) {
						size_t remainSamples = ARRAYSIZE(rawSamples) - rawSampleCount;

						if (remainSamples == 0) {
							LOG_MESSAGE(PLID, "Voice packet not enough space for samples from %s", client->m_szPlayerName);
							EngineUTIL::DropClient(client, false, "Voice packet not enough space for samples");

							return;
						}

						rawSampleCount += pClientData->m_pNewCodec->Decode((const uint8_t *)buf.PeekRead(), bytesCount, &rawSamples[rawSampleCount], remainSamples);
						buf.SkipBytes(bytesCount);
					} else {
						LOG_MESSAGE(PLID, "Voice packet invalid vdata size (cur = %u, need = %u) from %s", remainBytes, bytesCount, client->m_szPlayerName);
						EngineUTIL::DropClient(client, false, "Voice packet invalid vdata size (cur = %u, need = %u)", remainBytes, bytesCount);

						return;
					}
				}
				break;
				default: {
					LOG_MESSAGE(PLID, "Voice packet unknown command %u from %s", vpc, client->m_szPlayerName);
					EngineUTIL::DropClient(client, false, "Voice packet unknown command %u", vpc);

					/*FILE *pFile = fopen("VoiceTranscoder_UnknownDump.dat", "wb");
					fwrite(receivedBytes, sizeof(uint8_t), receivedBytesCount, pFile);
					fclose(pFile);*/

					return;
				}
				break;
			}
		}

		size_t remainBytes = buf.Size() - buf.TellRead() - sizeof(uint32_t);
		if (remainBytes) {
			LOG_MESSAGE(PLID, "Voice packet unknown remain bytes, vpc is %u from %s", buf.PeekUInt8(), client->m_szPlayerName);
			EngineUTIL::DropClient(client, false, "Voice packet unknown remain bytes, vpc is %u", buf.PeekUInt8());

			return;
		}
	} else {
		rawSampleCount = pClientData->m_pOldCodec->Decode((const uint8_t *)receivedBytes, receivedBytesCount, rawSamples, ARRAYSIZE(rawSamples));
	}

	int64_t frameTimeLength = rawSampleCount * 1000000 / 8000;

	if (pClientData->m_nextPacketTimeMicroSeconds > currentMicroSeconds) {
		pClientData->m_nextPacketTimeMicroSeconds += frameTimeLength;
	} else {
		pClientData->m_nextPacketTimeMicroSeconds = currentMicroSeconds + frameTimeLength;
	}

	// Ok only thread
	if (g_fThreadModeEnabled && needTranscode) {
		// TODO
		VTC_ThreadAddVoicePacket(client, clientIndex, pClientData, rawSamples, rawSampleCount);
	}

	// After some manipulations...
	// Non-thread
	uint8_t recompressed[MAX_VOICEPACKET_SIZE];
	size_t recompressedSize;
	if (!g_fThreadModeEnabled && needTranscode) {
		if (pClientData->hasNewCodec) {
			ChangeSamplesVolume(rawSamples, rawSampleCount, g_pcvarVolumeNewToOld->value);

			recompressedSize = pClientData->m_pOldCodec->Encode(rawSamples, rawSampleCount, recompressed, ARRAYSIZE(recompressed));
		} else {
			ChangeSamplesVolume(rawSamples, rawSampleCount, g_pcvarVolumeOldToNew->value);

			recompressedSize = pClientData->m_pNewCodec->Encode(rawSamples, rawSampleCount, &recompressed[14], ARRAYSIZE(recompressed) - 18);

			SteamID steamid;
			steamid.SetUniverse(UNIVERSE_PUBLIC);
			steamid.SetAccountType(ACCOUNT_TYPE_INDIVIDUAL);
			steamid.SetAccountId(0xFFFFFFFF); // 0 is invalid, but maximum value valid, TODO: randomize or get non-steam user steamid?
			*(uint64_t *)recompressed = steamid.ConvertToUInt64();
			*(uint8_t *)&recompressed[8] = VPC_SETSAMPLERATE;
			*(uint16_t *)&recompressed[9] = 8000;
			*(uint8_t *)&recompressed[11] = VPC_VDATA_SILK;
			*(uint16_t *)&recompressed[12] = (uint16_t)recompressedSize;

			CRC32 checksum;
			checksum.Init();
			checksum.Update(recompressed, 14 + recompressedSize);
			checksum.Final();

			*(uint32_t *)&recompressed[14 + recompressedSize] = checksum.ConvertToUInt32();

			recompressedSize += 18;
		}
	}

	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

		if (pDestClient == client) {
			if (!client->m_bLoopback) {
				if (EngineUTIL::MSG_GetRemainBytesCount(&client->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t)) { // zachem tam eshe 2 byte v originale?
					EngineUTIL::MSG_WriteUInt8_UnSafe(&client->m_Datagram, SVC_VOICEDATA);
					EngineUTIL::MSG_WriteUInt8_UnSafe(&client->m_Datagram, clientIndex - 1);
					EngineUTIL::MSG_WriteUInt16_UnSafe(&client->m_Datagram, 0);
				}

				continue;
			}
		} else {
			if (!pDestClient->m_fActive) {
				continue;
			}
			if (!(client->m_bsVoiceStreams[0] & (1 << i))) {
				continue;
			}
		}

		void *buf;
		size_t byteCount;
		if (g_rgClientData[i].hasNewCodec == g_rgClientData[clientIndex-1].hasNewCodec) {
			buf = receivedBytes;
			byteCount = receivedBytesCount;
		} else {
			if (g_fThreadModeEnabled) {
				continue;
			}

			buf = recompressed;
			byteCount = recompressedSize;
		}

		if (EngineUTIL::MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + byteCount) { // zachem tam eshe 2 byte v originale?
			EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, SVC_VOICEDATA);
			EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, clientIndex - 1);
			EngineUTIL::MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, byteCount);
			EngineUTIL::MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, buf, byteCount);
		}
	}
}

/* Thanks list:
Lev - help with thread
Crock - help with thread
*/

void VTC_InitCodecs(void) {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		g_rgClientData[i].m_pOldCodec = new Speex(5);
		g_rgClientData[i].m_pNewCodec = new SILK(5);
	}
}

void VTC_InitCvars(void) {
	CVAR_REGISTER(&g_cvarVersion);
	g_pcvarVersion = CVAR_GET_POINTER(g_cvarVersion.name);
	CVAR_REGISTER(&g_cvarDefaultCodec);
	g_pcvarDefaultCodec = CVAR_GET_POINTER(g_cvarDefaultCodec.name);
	CVAR_REGISTER(&g_cvarHltvCodec);
	g_pcvarHltvCodec = CVAR_GET_POINTER(g_cvarHltvCodec.name);
	CVAR_REGISTER(&g_cvarThreadMode);
	g_pcvarThreadMode = CVAR_GET_POINTER(g_cvarThreadMode.name);
	CVAR_REGISTER(&g_cvarMaxDelta);
	g_pcvarMaxDelta = CVAR_GET_POINTER(g_cvarMaxDelta.name);
	CVAR_REGISTER(&g_cvarVolumeNewToOld);
	g_pcvarVolumeNewToOld = CVAR_GET_POINTER(g_cvarVolumeNewToOld.name);
	CVAR_REGISTER(&g_cvarVolumeOldToNew);
	g_pcvarVolumeOldToNew = CVAR_GET_POINTER(g_cvarVolumeOldToNew.name);
	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");
}

void VTC_InitConfig(void) {
	char szRelPath[260];
	MetaUTIL::GetPluginRelPath(szRelPath, sizeof(szRelPath));
	snprintf(g_szExecConfigCmd, sizeof(g_szExecConfigCmd), "exec \"%s%s\"\n", szRelPath, VTC_CONFIGNAME);
}

void VTC_ExecConfig(void) {
	SERVER_COMMAND(g_szExecConfigCmd);
	SERVER_EXECUTE();
}

Hook_Begin *g_phookSvParseVoiceData;

bool TryGetRehldsAPI(Library *pEngine) {
	void *(* CreateInterface)(const char *interfaceName, size_t *returnCode) = pEngine->FindSymbol("CreateInterface");

	if (CreateInterface == nullptr) {
		return false;
	}

	g_pRehldsAPI = (IRehldsApi *)CreateInterface("VREHLDS_HLDS_API_VERSION001", nullptr);

	if (g_pRehldsAPI == nullptr) {
		return false;
	}
	if (g_pRehldsAPI->GetMajorVersion() != REHLDS_API_VERSION_MAJOR) {
		return false;
	}
	if (g_pRehldsAPI->GetMinorVersion() < REHLDS_API_VERSION_MINOR) {
		return false;
	}

	return true;
}

void HandleNetCommand_Hook(IRehldsHook_HandleNetCommand *chain, IGameClient *client, uint8_t netcmd) {
	if (netcmd == CLC_VOICEDATA) {
		SV_ParseVoiceData_Hook(g_pRehldsAPI->GetServerStatic()->GetClient_t(client->GetId()));

		return;
	}

	chain->callNext(client, netcmd);
}

// Init base?
void Hacks_Init() {
	AnyPointer pfnSvParseVoiceData;

	g_pEngine = new Library(g_engfuncs.pfnPrecacheModel);

	if (TryGetRehldsAPI(g_pEngine)) {
		g_isUsingRehldsAPI = true;

		g_netMessage.m_pMsgReadCount = (size_t *)g_pRehldsAPI->GetFuncs()->GetMsgReadCount();
		g_netMessage.m_pMsgBadRead = (bool *)g_pRehldsAPI->GetFuncs()->GetMsgBadRead();
		g_netMessage.m_pNetMsg = g_pRehldsAPI->GetFuncs()->GetNetMessage();

		g_pRehldsAPI->GetHookchains()->HandleNetCommand()->registerHook((void (*)(IRehldsHook_HandleNetCommand *, IGameClient *, int8))&HandleNetCommand_Hook);
	} else {
#ifdef WIN32
		g_pfnSvDropClient = g_pEngine->FindFunctionByString("Dropped %s from server\nReason:  %s\n");

		// TODO: we can simple find next reloc
		uintptr_t ptr = g_pEngine->FindStringUsing("Failed command checksum for %s:%s\n");
		ptr = *(uintptr_t *)g_pEngine->SearchForTemplate(BinaryPattern("\xC7\x05\x00\x00\x00\x00\x01\x00\x00\x00", "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF", 10), ptr, 0x20, 2);
		g_netMessage.m_pMsgReadCount = (decltype(g_netMessage.m_pMsgReadCount))(ptr - 4);
		g_netMessage.m_pMsgBadRead = (decltype(g_netMessage.m_pMsgBadRead))ptr;

		ptr = g_pEngine->FindStringUsing("net_message");
		ptr = *(uintptr_t *)(ptr - 4);
		g_netMessage.m_pNetMsg = (decltype(g_netMessage.m_pNetMsg))(ptr - offsetof(sizebuf_t, buffername));

		pfnSvParseVoiceData = g_pEngine->FindFunctionByString("SV_ParseVoiceData: invalid incoming packet.\n");
#else
		g_pfnSvDropClient = g_pEngine->FindSymbol("SV_DropClient");
		g_netMessage.m_pMsgReadCount = (decltype(g_netMessage.m_pMsgReadCount))g_pEngine->FindSymbol("msg_readcount");
		g_netMessage.m_pMsgBadRead = (decltype(g_netMessage.m_pMsgBadRead))g_pEngine->FindSymbol("msg_badread");
		g_netMessage.m_pNetMsg = (decltype(g_netMessage.m_pNetMsg))g_pEngine->FindSymbol("net_message");
		pfnSvParseVoiceData = g_pEngine->FindSymbol("SV_ParseVoiceData");
#endif

		g_phookSvParseVoiceData = g_pEngine->HookFunction(pfnSvParseVoiceData, &SV_ParseVoiceData_Hook);
	}
}

void Hacks_Deinit() {
	if (!g_isUsingRehldsAPI) {
		g_phookSvParseVoiceData->UnHook();
		delete g_phookSvParseVoiceData;
	}

	delete g_pEngine;
}