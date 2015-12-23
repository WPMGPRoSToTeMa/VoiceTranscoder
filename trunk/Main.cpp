#include "Main.h"
#include <extdll.h>
#include <meta_api.h>
#include <GoldSrcEngineStructs.h>
#include <Time.h>
#include <Buffer.h>
#include "ThreadMode.h"
#include <MetaUTIL.h>
#include <EngineUTIL.h>
#include <Module.h>
#include <FunctionHook_Beginning.h>
#include <rehlds_api.h>
#include <UtilFunctions.h>
#include <CRC32.h>
#include <SteamID.h>
#include "API.h"

#if !defined(_WIN32) && !defined(__linux__)
#error "Unknown platform"
#endif

#ifdef _WIN32
	#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#endif

// Variables
clientData_t g_clientData[MAX_CLIENTS];
char g_execConfigCmd[300];
bool g_fThreadModeEnabled;

Module g_engineModule;
FunctionHook_Beginning g_hookSvParseVoiceData;

bool g_isUnregisteredVoiceCvars;

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
cvar_t g_cvarForceSendHLTV = {"VTC_ForceSendHLTV", "1", FCVAR_EXTDLL, 0, nullptr};
cvar_t *g_pcvarForceSendHLTV;
cvar_t g_cvarAPI = {"VTC_API", "0", FCVAR_EXTDLL | FCVAR_PROTECTED, 0, nullptr};
cvar_t *g_pcvarAPI;
cvar_t g_cvarVoiceQuality = {"sv_voicequality", "5", FCVAR_EXTDLL, 0, nullptr};
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
	clientData_t *pClientData = &g_clientData[nClientIndex-1];

	if (FStrEq(pszCmd, "VTC_CheckStart")) {
		pClientData->m_isChecking = true;
		pClientData->m_hasNewCodec = false;
		pClientData->m_isVguiRunScriptReceived = false;
	} else if (pClientData->m_isChecking) {
		if (FStrEq(pszCmd, "vgui_runscript")) {
			pClientData->m_isVguiRunScriptReceived = true;
		} else if (FStrEq(pszCmd, "VTC_CheckEnd")) {
			pClientData->m_isChecking = false;
			pClientData->m_hasNewCodec = pClientData->m_isVguiRunScriptReceived ? true : false;
			pClientData->m_isVguiRunScriptReceived = false;

			LOG_MESSAGE(PLID, "Client with %s codec connected", pClientData->m_hasNewCodec ? "new" : "old");
		}
	}

	RETURN_META(MRES_IGNORED);
}

// TODO: bool32_t
qboolean ClientConnect_PostHook(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason) {
	int nClientIndex = ENTINDEX(pClient);
	clientData_t *pClientData = &g_clientData[nClientIndex-1];

	// Default client codec
	if (FStrEq(GETPLAYERAUTHID(pClient), "HLTV")) {
		pClientData->m_hasNewCodec = FStrEq(g_pcvarHltvCodec->string, "old") ? false : true;
		// Add print?
	} else {
		pClientData->m_hasNewCodec = FStrEq(g_pcvarDefaultCodec->string, "old") ? false : true;
	}
	pClientData->m_isChecking = false;
	pClientData->m_nextPacketTimeMicroSeconds = 0;
	pClientData->m_isSpeaking = false;
	pClientData->m_isMuted = false;

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
	VTC_UpdateCodecs();

	if (g_isUnregisteredVoiceCvars) {
		MESSAGE_BEGIN(MSG_INIT, SVC_VOICEINIT);
		WRITE_STRING("voice_speex");
		WRITE_BYTE((size_t)CVAR_GET_FLOAT("sv_voicequality"));
		MESSAGE_END();
	}

	RETURN_META(MRES_IGNORED);
}

void StartFrame_PostHook() {
	if (g_fThreadModeEnabled) {
		VTC_ThreadVoiceFlusher();
	}

	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pClient = EngineUTIL::GetClientByIndex(i + 1);
		if (!pClient->m_fActive) {
			continue;
		}

		clientData_t *pClientData = &g_clientData[i];
		uint64_t currentTimeMicroSeconds = GetCurrentTimeInMicroSeconds();
		if (pClientData->m_isSpeaking && currentTimeMicroSeconds >= pClientData->m_nextPacketTimeMicroSeconds + 100000) {
			pClientData->m_isSpeaking = false;

			g_callback_ClientStopSpeak.Call(i+1);
		}
	}

	RETURN_META(MRES_IGNORED);
}

// MetaMod API
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION, // ifvers
	"VoiceTranscoder",      // name
	"2.0RC1 Reloaded",      // version
	"Dec 22 2015",          // date
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

#include <cstdio>

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) {
	// Reset
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	gpMetaGlobals = pMGlobals;
	gpGamedllFuncs = pGamedllFuncs;

	pFunctionTable->pfnGetEntityAPI2 = &GetEntityAPI2;
	pFunctionTable->pfnGetEntityAPI2_Post = &GetEntityAPI2_Post;

	void *pfnSvParseVoiceData;
	g_engineModule.Open(g_engfuncs.pfnPrecacheModel);
	EngineUTIL::Init(g_engineModule);

	FILE *pFile = fopen("vtc.txt", "wt");
	fprintf(pFile, "%d", 1);
	fclose(pFile);

	if (!EngineUTIL::IsReHLDS()) {
#ifdef _WIN32
		g_engineModule.Analyze();

		pfnSvParseVoiceData = g_engineModule.FindFunctionByString("SV_ParseVoiceData: invalid incoming packet.\n");
#elif defined __linux__
		pfnSvParseVoiceData = g_engineModule.FindSymbol("SV_ParseVoiceData");
#endif

		g_hookSvParseVoiceData.Create(pfnSvParseVoiceData, &SV_ParseVoiceData_Hook);
	} else {
		EngineUTIL::GetRehldsAPI()->GetHookchains()->HandleNetCommand()->registerHook((void(*)(IRehldsHook_HandleNetCommand *, IGameClient *, int8))&HandleNetCommand_Hook);

		FILE *pFile = fopen("vtc.txt", "wt");
		fprintf(pFile, "%d", 2);
		fclose(pFile);
	}

	VTC_InitCvars();
	VTC_InitAPI();
	VTC_InitConfig();
	VTC_ExecConfig();
	VTC_InitCodecs();

	if (g_pcvarThreadMode->value != 0.0f) {
		g_fThreadModeEnabled = true;

		VTC_ThreadInit();
	}

	return TRUE;
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
	if (g_fThreadModeEnabled) {
		VTC_ThreadDeinit();
	}

	if (!EngineUTIL::IsReHLDS()) {
		g_hookSvParseVoiceData.Remove();
	}
	else {
		EngineUTIL::GetRehldsAPI()->GetHookchains()->HandleNetCommand()->unregisterHook((void(*)(IRehldsHook_HandleNetCommand *, IGameClient *, int8))&HandleNetCommand_Hook);
	}
	g_engineModule.Close();

	return TRUE;
}

void HandleNetCommand_Hook(IRehldsHook_HandleNetCommand *chain, IGameClient *client, uint8_t netcmd) {
	if (netcmd == CLC_VOICEDATA) {
		SV_ParseVoiceData_Hook(EngineUTIL::GetRehldsAPI()->GetServerStatic()->GetClient_t(client->GetId()));

		return;
	}

	chain->callNext(client, netcmd);
}

// TODO: check for spam small packets
void SV_ParseVoiceData_Hook(client_t *pClient) {
	size_t clientIndex = EngineUTIL::GetClientIndex(pClient);
	clientData_t *pClientData = &g_clientData[clientIndex-1];

	size_t receivedBytesCount = EngineUTIL::MSG_ReadUInt16();
	if (receivedBytesCount > MAX_VOICEPACKET_SIZE) {
		LOG_MESSAGE(PLID, "Oversize voice packet, size = %u (max = %u) from %s", receivedBytesCount, MAX_VOICEPACKET_SIZE, pClient->m_szPlayerName);
		EngineUTIL::DropClient(pClient, false, "Oversize voice packet, size = %u (max = %u)", receivedBytesCount, MAX_VOICEPACKET_SIZE);

		return;
	}

	uint8_t receivedBytes[MAX_VOICEPACKET_SIZE];
	EngineUTIL::MSG_ReadBuf(receivedBytes, receivedBytesCount);

	if (pClientData->m_isMuted) {
		return;
	}
	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}
	if (!pClient->m_fActive) {
		return;
	}

	uint64_t currentMicroSeconds = GetCurrentTimeInMicroSeconds();

	if (pClientData->m_nextPacketTimeMicroSeconds > currentMicroSeconds) {
		if ((pClientData->m_nextPacketTimeMicroSeconds - currentMicroSeconds)/1000.0 > g_pcvarMaxDelta->value) {
			//LOG_MESSAGE(PLID, "Delta is %g", (pClientData->m_nextPacketTimeMicroSeconds - currentMicroSeconds)/1000.0);

			return;
		}
	}

	int16_t rawSamples[MAX_DECOMPRESSED_VOICEPACKET_SAMPLES];
	size_t rawSampleCount;

	// Validate new codec packet
	if (pClientData->m_hasNewCodec) {
		if (receivedBytesCount < MIN_VOICEPACKET_SIZE) {
			LOG_MESSAGE(PLID, "Too small voice packet (real = %u, min = %u) from %s", receivedBytesCount, MIN_VOICEPACKET_SIZE, pClient->m_szPlayerName);
			EngineUTIL::DropClient(pClient, false, "Too small voice packet (real = %u, min = %u)", receivedBytesCount, MIN_VOICEPACKET_SIZE);

			return;
		}

		Buffer buf(receivedBytes, receivedBytesCount);

		CRC32 checksum;
		checksum.Init();
		checksum.Update(buf.PeekRead(), buf.Size() - sizeof(uint32_t));
		checksum.Final();

		buf.SeekRead(sizeof(uint32_t), SEEK_END);
		uint32_t dwRecvdChecksum = buf.ReadUInt32();

		if (checksum.ToUInt32() != dwRecvdChecksum) {
			LOG_MESSAGE(PLID, "Voice packet checksum validation failed for %s", pClient->m_szPlayerName);
			EngineUTIL::DropClient(pClient, false, "Voice packet checksum validation failed");

			return;
		}

		// Go to start
		buf.RewindRead();

		// SteamID
		uint64_t qwSteamID = buf.ReadUInt64();

		// Validate SteamID
		SteamID steamid(qwSteamID);
		if (!steamid.IsValid()) {
			LOG_MESSAGE(PLID, "Invalid steamid (%llu) in voice packet from %s", steamid.ConvertToUInt64(), pClient->m_szPlayerName);
			EngineUTIL::DropClient(pClient, false, "Invalid steamid (%llu) in voice packet", steamid.ConvertToUInt64());

			return;
		}

		rawSampleCount = 0;
		while (buf.Size() - buf.TellRead() - sizeof(uint32_t) >= sizeof(uint8_t) + sizeof(uint16_t)) {
			uint8_t vpc = buf.ReadUInt8();

			switch (vpc) {
				case VPC_SETSAMPLERATE: {
					uint16_t sampleRate = buf.ReadUInt16();

					if (sampleRate != NEWCODEC_WANTED_SAMPLERATE) {
						LOG_MESSAGE(PLID, "Voice packet unwanted samplerate (cur = %u, want = %u) from %s", sampleRate, NEWCODEC_WANTED_SAMPLERATE, pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Voice packet unwanted samplerate (cur = %u, want = %u)", sampleRate, NEWCODEC_WANTED_SAMPLERATE);

						return;
					}
				}
				break;
				case VPC_VDATA_SILENCE: {
					size_t silenceSampleCount = buf.ReadUInt16();

					if (silenceSampleCount > ARRAYSIZE(rawSamples) - rawSampleCount) {
						LOG_MESSAGE(PLID, "Too many silence samples (cur %u, max %u) from %s", rawSampleCount, ARRAYSIZE(rawSamples), pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Too many silence samples (cur %u, max %u)", rawSampleCount, ARRAYSIZE(rawSamples), steamid.ConvertToUInt64());

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
							LOG_MESSAGE(PLID, "Voice packet not enough space for samples from %s", pClient->m_szPlayerName);
							EngineUTIL::DropClient(pClient, false, "Voice packet not enough space for samples");

							return;
						}

						rawSampleCount += pClientData->m_pNewCodec->Decode((const uint8_t *)buf.PeekRead(), bytesCount, &rawSamples[rawSampleCount], remainSamples);
						buf.SkipBytes(bytesCount);
					} else {
						LOG_MESSAGE(PLID, "Voice packet invalid vdata size (cur = %u, need = %u) from %s", remainBytes, bytesCount, pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Voice packet invalid vdata size (cur = %u, need = %u)", remainBytes, bytesCount);

						return;
					}
				}
				break;
				default: {
					LOG_MESSAGE(PLID, "Voice packet unknown command %u from %s", vpc, pClient->m_szPlayerName);
					EngineUTIL::DropClient(pClient, false, "Voice packet unknown command %u", vpc);

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
			LOG_MESSAGE(PLID, "Voice packet unknown remain bytes, vpc is %u from %s", buf.PeekUInt8(), pClient->m_szPlayerName);
			EngineUTIL::DropClient(pClient, false, "Voice packet unknown remain bytes, vpc is %u", buf.PeekUInt8());

			return;
		}
	} else {
		rawSampleCount = pClientData->m_pOldCodec->Decode((const uint8_t *)receivedBytes, receivedBytesCount, rawSamples, ARRAYSIZE(rawSamples));
	}

	uint64_t frameTimeLength = rawSampleCount * 1000000 / 8000;

	//LOG_MESSAGE(PLID, "Frame time: %f %f %d %d", currentMicroSeconds / 1000000.0, frameTimeLength / 1000.0, receivedBytesCount, needTranscode);

	if (pClientData->m_nextPacketTimeMicroSeconds > currentMicroSeconds) {
		pClientData->m_nextPacketTimeMicroSeconds += frameTimeLength;
	} else {
		pClientData->m_nextPacketTimeMicroSeconds = currentMicroSeconds + frameTimeLength;

		if (!pClientData->m_isSpeaking) {
			pClientData->m_isSpeaking = true;

			g_callback_ClientStartSpeak.Call(clientIndex);
		}
	}

	bool needTranscode = false;
	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

		if (pDestClient == pClient) {
			continue;
		} else {
			if (!pDestClient->m_fActive) {
				continue;
			}
			if (!pDestClient->m_fHltv || g_pcvarForceSendHLTV->value == 0) {
				if (!(pClient->m_bsVoiceStreams[0] & (1 << i))) {
					continue;
				}
			}
		}

		if (g_clientData[i].m_hasNewCodec != g_clientData[clientIndex - 1].m_hasNewCodec) {
			needTranscode = true;

			break;
		}
	}

	// Ok only thread
	if (g_fThreadModeEnabled && needTranscode) {
		// TODO
		VTC_ThreadAddVoicePacket(pClient, clientIndex, pClientData, rawSamples, rawSampleCount);
	}

	// After some manipulations...
	// Non-thread
	uint8_t recompressed[MAX_VOICEPACKET_SIZE];
	size_t recompressedSize;
	if (!g_fThreadModeEnabled && needTranscode) {
		if (pClientData->m_hasNewCodec) {
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

			*(uint32_t *)&recompressed[14 + recompressedSize] = checksum.ToUInt32();

			recompressedSize += 18;
		}
	}

	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

		if (pDestClient == pClient) {
			if (!pClient->m_bLoopback) {
				if (EngineUTIL::MSG_GetRemainBytesCount(&pClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t)) { // zachem tam eshe 2 byte v originale?
					EngineUTIL::MSG_WriteUInt8_UnSafe(&pClient->m_Datagram, SVC_VOICEDATA);
					EngineUTIL::MSG_WriteUInt8_UnSafe(&pClient->m_Datagram, clientIndex - 1);
					EngineUTIL::MSG_WriteUInt16_UnSafe(&pClient->m_Datagram, 0);
				}

				continue;
			}
		} else {
			if (!pDestClient->m_fActive) {
				continue;
			}
			if (!pDestClient->m_fHltv || g_pcvarForceSendHLTV->value == 0) {
				if (!(pClient->m_bsVoiceStreams[0] & (1 << i))) {
					continue;
				}
			}
		}

		void *buf;
		size_t byteCount;
		if (g_clientData[i].m_hasNewCodec == g_clientData[clientIndex-1].m_hasNewCodec) {
			buf = receivedBytes;
			byteCount = receivedBytesCount;
		} else {
			if (g_fThreadModeEnabled) {
				continue;
			}

			buf = recompressed;
			byteCount = recompressedSize;
		}

		if (EngineUTIL::MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + byteCount) {
			EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, SVC_VOICEDATA);
			EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, clientIndex - 1);
			EngineUTIL::MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, byteCount);
			EngineUTIL::MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, buf, byteCount);
		}
	}
}

void VTC_InitCodecs(void) {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		g_clientData[i].m_pOldCodec = new VoiceCodec_Speex((size_t)CVAR_GET_FLOAT("sv_voicequality"));
		g_clientData[i].m_pNewCodec = new VoiceCodec_SILK(5);
	}
}

void VTC_UpdateCodecs() {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		g_clientData[i].m_pOldCodec->ChangeQuality((size_t)CVAR_GET_FLOAT("sv_voicequality"));
	}
}

void VTC_InitCvars(void) {
	CVAR_REGISTER(&g_cvarVersion);
	g_pcvarVersion = CVAR_GET_POINTER(g_cvarVersion.name);
	CVAR_SET_STRING(g_cvarVersion.name, g_cvarVersion.string);
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
	CVAR_REGISTER(&g_cvarForceSendHLTV);
	g_pcvarForceSendHLTV = CVAR_GET_POINTER(g_cvarForceSendHLTV.name);
	if (!EngineUTIL::IsReHLDS()) {
		CVAR_REGISTER(&g_cvarAPI);
		g_pcvarAPI = CVAR_GET_POINTER(g_cvarAPI.name);
	}
	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");

	if (CVAR_GET_POINTER("sv_voicequality") == nullptr) {
		CVAR_REGISTER(&g_cvarVoiceQuality);

		g_isUnregisteredVoiceCvars = true;
	}
}

void VTC_InitAPI() {
	if (EngineUTIL::IsReHLDS()) {
		EngineUTIL::GetRehldsAPI()->GetFuncs()->RegisterPluginApi(Plugin_info.name, &g_voiceTranscoderAPI);
	} else {
		char ptrStr[16];
		sprintf(ptrStr, "%.8X", (uintptr_t)&g_voiceTranscoderAPI);
		CVAR_SET_STRING(g_cvarAPI.name, ptrStr);
	}
}

void VTC_InitConfig(void) {
	char relPath[260];
	MetaUTIL::GetPluginRelPath(relPath, sizeof(relPath));
	snprintf(g_execConfigCmd, sizeof(g_execConfigCmd), "exec \"%s%s\"\n", relPath, VTC_CONFIGNAME);
}

void VTC_ExecConfig(void) {
	SERVER_COMMAND(g_execConfigCmd);
	SERVER_EXECUTE();
}