#define NOMINMAX
#include "Main.h"
#include <extdll.h>
#include <meta_api.h>
#include <GoldSrcEngineStructs.h>
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
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <memory>
#include <algorithm>
#include <array>
#include <cinttypes>
#include <opus.h>

#if !defined(_WIN32) && !defined(__linux__)
#error "Unknown platform"
#endif

// Variables
std::vector<playSound_t> g_playSounds;
clientData_t g_clientData[MAX_CLIENTS];
char g_execConfigCmd[300];
bool g_fThreadModeEnabled;

Module g_engineModule;
FunctionHook_Beginning g_hookSvParseVoiceData;

bool g_isUnregisteredVoiceCvars;

size_t g_oldVoiceQuality;

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

class VoiceCodec_OPUS_PLC {
	OpusDecoder* _opusDecoder;
public:
	VoiceCodec_OPUS_PLC() {
		_opusDecoder = opus_decoder_create(8000, 1, nullptr);
	}

	~VoiceCodec_OPUS_PLC() {
		opus_decoder_destroy(_opusDecoder);
	}

	void ResetState() {
		opus_decoder_ctl(_opusDecoder, OPUS_RESET_STATE);
	}

	size_t Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) {
		if (encodedBytes == nullptr) {
			return 0;
		}
		if (encodedBytesCount == 0) {
			return 0;
		}
		if (rawSamples == nullptr) {
			return 0;
		}
		if (maxRawSamples == 0) {
			return 0;
		}

		size_t curEncodedBytePos = 0;
		size_t decodedRawSamples = 0;

		while (encodedBytesCount >= sizeof(uint16_t) + sizeof(uint16_t)) {
			// TODO
			if (decodedRawSamples + 160 > maxRawSamples) {
				return 0;
			}

			uint16_t payloadSize = *(uint16_t *)&encodedBytes[curEncodedBytePos];
			
			curEncodedBytePos += sizeof(uint16_t);
			curEncodedBytePos += sizeof(uint16_t);
			encodedBytesCount -= sizeof(uint16_t);
			encodedBytesCount -= sizeof(uint16_t);

			if (payloadSize == 0) {
				//LOG_MESSAGE(PLID, "Silence frame");

				memset(&rawSamples[decodedRawSamples], 0, 160 * sizeof(uint16_t));
				decodedRawSamples += 160;

				continue;
			}
			if (payloadSize == 0xFFFF) {
				//LOG_MESSAGE(PLID, "Reset frame");

				ResetState();

				return decodedRawSamples;
			}
			if (payloadSize > encodedBytesCount) {
				return 0;
			}
			//LOG_MESSAGE(PLID, "Normal frame");

			int16_t decodedSamples = opus_decode(_opusDecoder, &encodedBytes[curEncodedBytePos], payloadSize, &rawSamples[decodedRawSamples], maxRawSamples - decodedRawSamples, 0);

			// An error occured during decode process
			// TODO: possibly we can use `opus_packet_parse` for preventive packet validation
			if (decodedSamples < 0) {
				return std::size_t(-1);
			}
			
			decodedRawSamples += decodedSamples;
			curEncodedBytePos += payloadSize;
			encodedBytesCount -= payloadSize;
		}

		return decodedRawSamples;
	}
};

#if !defined(_WIN32)
C_DLLEXPORT
#endif
void WINAPI GiveFnptrsToDll(enginefuncs_t *pEngFuncs, globalvars_t *pGlobalVars) {
#ifdef _WIN32
	// Thanks to https://stackoverflow.com/a/41910450
	#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
#endif

	memcpy(&g_engfuncs, pEngFuncs, sizeof(g_engfuncs));
	gpGlobals = pGlobalVars;
}

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientCommand = &OnClientCommandReceiving;

	return TRUE;
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientConnect = &OnClientConnected;
	pFunctionTable->pfnClientDisconnect = &OnClientDisconnected;
	pFunctionTable->pfnServerActivate = &OnServerActivated;
	pFunctionTable->pfnStartFrame = &OnFrameStarted;

	return TRUE;
}

template <typename T, typename = std::enable_if_t<!std::is_same<std::remove_reference_t<T>, std::string>::value>>
T NormalizeArg(T&& value) {
	return value;
}

template <typename T, typename = std::enable_if_t<std::is_same<std::remove_reference_t<T>, std::string>::value>>
const char *NormalizeArg(T &&value) {
	return value.c_str();
}

template <typename ...TArgs>
std::string Format(const char *format, TArgs&&... args) {
	size_t neededSize = snprintf(nullptr, 0, format, NormalizeArg(args)...) + 1;
	auto formatted = std::make_unique<char[]>(neededSize);
	snprintf(formatted.get(), neededSize, format, NormalizeArg(args)...);
	return {formatted.get(), neededSize - 1};
}

template <typename ...TArgs>
void PrintToConsole(edict_t* client, const char *format, TArgs&&... args) {
	CLIENT_PRINTF(client, print_console, (Format(format, args...) + '\n').c_str());
}

std::string MicrosecondsToString(uint64_t us) {
	constexpr uint64_t microsecondsInSecond = 1e6;
	constexpr uint64_t secondsInHour = 3600;
	constexpr uint64_t microsecondsInHour = secondsInHour * microsecondsInSecond;
	constexpr uint64_t secondsInMinute = 60;
	constexpr uint64_t microsecondsInMinute = secondsInMinute * microsecondsInSecond;
	auto hours = us / microsecondsInHour;
	us %= microsecondsInHour;
	auto minutes = us / microsecondsInMinute;
	us %= microsecondsInMinute;
	auto seconds = us / microsecondsInSecond;
	us %= microsecondsInSecond;
	return Format("%02" PRIi64 ":%02i:%02i.%06i", hours, (int)minutes, (int)seconds, (int)us);
}

// Entity API
void OnClientCommandReceiving(edict_t *pClient) {
	auto command = CMD_ARGV(0);

	auto playerSlot = ENTINDEX(pClient);
	auto &clientData = g_clientData[playerSlot-1];

	if (FStrEq(command, "VTC_CheckStart")) {
		clientData.IsChecking = true;
		clientData.HasNewCodec = false;
		clientData.IsVguiRunScriptReceived = false;

		RETURN_META(MRES_SUPERCEDE);
	} else if (clientData.IsChecking) {
		if (FStrEq(command, "vgui_runscript")) {
			clientData.IsVguiRunScriptReceived = true;

			RETURN_META(MRES_SUPERCEDE);
		} else if (FStrEq(command, "VTC_CheckEnd")) {
			clientData.IsChecking = false;
			clientData.HasNewCodec = clientData.IsVguiRunScriptReceived ? true : false;
			clientData.IsVguiRunScriptReceived = false;

			LOG_MESSAGE(PLID, "Client %s with %s codec connected", STRING(pClient->v.netname), clientData.HasNewCodec ? "new" : "old");

			RETURN_META(MRES_SUPERCEDE);
		}
	}

	RETURN_META(MRES_IGNORED);
}

// TODO: bool32_t
qboolean OnClientConnected(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason) {
	int playerSlot = ENTINDEX(pClient);
	auto &clientData = g_clientData[playerSlot-1];

	// Default client codec
	if (FStrEq(GETPLAYERAUTHID(pClient), "HLTV")) {
		clientData.HasNewCodec = FStrEq(g_pcvarHltvCodec->string, "old") ? false : true;
		// Add print?
	} else {
		clientData.HasNewCodec = FStrEq(g_pcvarDefaultCodec->string, "old") ? false : true;
	}
	clientData.IsChecking = false;
	clientData.NextVoicePacketExpectedTime = nullptr;
	clientData.VoiceEndTime = nullptr;
	clientData.IsSpeaking = false;
	clientData.IsMuted = false;
	clientData.IsBlocked = false;
	clientData.NewCodec->ResetState();
	clientData.NewCodec2->ResetState();
	clientData.SampleRate = nullptr;
	clientData.OldCodec->ResetState();

	RETURN_META_VALUE(MRES_IGNORED, META_RESULT_ORIG_RET(bool32_t));
}

void OnClientDisconnected(edict_t *pClient) {
	int playerSlot = ENTINDEX(pClient);
	clientData_t &clientData = g_clientData[playerSlot - 1];

	if (clientData.IsSpeaking) {
		clientData.IsSpeaking = false;

		g_OnClientStopSpeak(playerSlot);
	}

	// TODO: delete with swap in for-loop
	g_playSounds.erase(
		std::remove_if(
			g_playSounds.begin(),
			g_playSounds.end(),
			[client = EngineUTIL::GetClientByIndex(playerSlot)](const playSound_t &playSound) { return playSound.receiver == client; }),
		g_playSounds.end());

	RETURN_META(MRES_IGNORED);
}

void OnServerActivated(edict_t *pEdictList, int nEdictCount, int nClientMax) {
	g_playSounds.clear();

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

auto FullyConnectedClients() {
	class {
		class Iterator {
			size_t _clientIndex;
		public:
			Iterator(size_t clientIndex) : _clientIndex(clientIndex) {
				CheckAndMakeValid();
			}

			void CheckAndMakeValid() {
				while (_clientIndex != gpGlobals->maxClients && !operator*().m_fZombie) {
					_clientIndex++;
				}
			}

			bool operator!=(const Iterator &other) const {
				return _clientIndex != other._clientIndex;
			}
			void operator++() {
				_clientIndex++;
				CheckAndMakeValid();
			}
			const client_t &operator*() const {
				return *EngineUTIL::GetClientByIndex(_clientIndex + 1);
			}
		};
	public:
		auto begin() const {
			return Iterator(0);
		}
		auto end() const {
			return Iterator(gpGlobals->maxClients);
		}
	} enumerator;
	return enumerator;
}

void OnFrameStarted() {
	if ((size_t)CVAR_GET_FLOAT("sv_voicequality") != g_oldVoiceQuality) {
		VTC_UpdateCodecs();

		// TODO: this is very bad
		if (g_isUnregisteredVoiceCvars) {
			MESSAGE_BEGIN(MSG_INIT, SVC_VOICEINIT);
			WRITE_STRING("voice_speex");
			WRITE_BYTE(g_oldVoiceQuality);
			MESSAGE_END();
		}
	}

	if (g_fThreadModeEnabled) {
		VTC_ThreadVoiceFlusher();
	}

	for (const auto &client : FullyConnectedClients()) {
		auto clientIndex = EngineUTIL::GetClientIndex(&client) - 1;

		//LOG_MESSAGE(PLID, "%d", clientIndex);

		clientData_t &clientData = g_clientData[clientIndex];
		if (clientData.IsSpeaking && steady_clock::now() >= clientData.VoiceEndTime) {
			clientData.IsSpeaking = false;

			g_OnClientStopSpeak(clientIndex + 1);
		}
	}

	auto now = steady_clock::now();
	for (auto &playSound : g_playSounds) {
		if (now >= playSound.OldEncodedDataSendTime && playSound.currentSample != playSound.samples8k.size()) {
			int16_t *samplesSlice8k = &playSound.samples8k[playSound.currentSample];
			size_t sampleCount = min(playSound.samples8k.size() - playSound.currentSample, (size_t)160);
			playSound.currentSample += sampleCount;
			playSound.OldEncodedDataSendTime += 20ms - microseconds(125); // TODO: ..., 1/8000 for 8k NS good! for 16k S bad, for 8k S very bad!!!

			if (sampleCount != 0) {
				array<uint8_t, 1024> oldEncodedData;
				size_t oldEncodedDataSize;
				array<int16_t, 160> samples;
				memcpy(samples.data(), samplesSlice8k, sampleCount * sizeof(samplesSlice8k[0]));
				memset(&samples.data()[sampleCount], 0, (samples.size() - sampleCount) * sizeof(decltype(samples)::value_type));

				oldEncodedDataSize = playSound.oldCodec->Encode(samples.data(), samples.size(), oldEncodedData.data(), oldEncodedData.size());

				for (size_t i = 0; i < gpGlobals->maxClients; i++) {
					client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

					if (g_clientData[i].HasNewCodec) {
						continue;
					}
					if (!pDestClient->m_fZombie) {
						continue;
					}
					if (!(pDestClient->m_fHltv && g_pcvarForceSendHLTV->value != 0)) {
						if (playSound.receiver != nullptr && playSound.receiver != pDestClient) {
							continue;
						}
					}

					if (EngineUTIL::MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + oldEncodedDataSize) {
						EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, SVC_VOICEDATA);
						EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, MAX_CLIENTS);
						EngineUTIL::MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, oldEncodedDataSize);
						EngineUTIL::MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, oldEncodedData.data(), oldEncodedDataSize);
					}
				}
			}
		}
		if (now >= playSound.NewEncodedDataSendTime && playSound.PlayedNewEncodedSampleCount != playSound.samples16k.size()) {
			int16_t *samplesSlice16k = &playSound.samples16k[playSound.PlayedNewEncodedSampleCount];
			size_t sampleCount = min(playSound.samples16k.size() - playSound.PlayedNewEncodedSampleCount, (size_t)320);
			playSound.PlayedNewEncodedSampleCount += sampleCount;
			playSound.NewEncodedDataSendTime += 20ms - microseconds(137); // TODO: ..., 1/8000 for 8k NS good! for 16k S bad, for 8k S very bad!!!
			// chut chut rastet
			// vector vilet v konce!!

			if (sampleCount != 0) {
				array<byte, 1024> newEncodedData;
				size_t newEncodedDataSize;
				array<int16_t, 320> samples;
				memcpy(samples.data(), samplesSlice16k, sampleCount * sizeof(samplesSlice16k[0]));
				memset(&samples.data()[sampleCount], 0, (samples.size() - sampleCount) * sizeof(decltype(samples)::value_type));

				newEncodedDataSize = playSound.newCodec->Encode(samples.data(), samples.size(), &newEncodedData.data()[14], newEncodedData.size() - 18);

				SteamID steamid;
				steamid.SetUniverse(UNIVERSE_PUBLIC);
				steamid.SetAccountType(ACCOUNT_TYPE_INDIVIDUAL);
				steamid.SetAccountId(0xFFFFFFFE); // Use different for separate channels
				steamid.SetAccountInstance(STEAMUSER_DESKTOPINSTANCE);
				*(uint64_t *)newEncodedData.data() = steamid.ToUInt64();
				*(uint8_t *)&newEncodedData.data()[8] = VPC_SETSAMPLERATE;
				*(uint16_t *)&newEncodedData.data()[9] = 16000; // TODO: sent to steam with original samplerate, but samplerate should be % 4000 == 0
				*(uint8_t *)&newEncodedData.data()[11] = VPC_VDATA_SILK;
				*(uint16_t *)&newEncodedData.data()[12] = (uint16_t)newEncodedDataSize;

				CRC32 checksum;
				checksum.Init();
				checksum.Update(newEncodedData.data(), newEncodedDataSize + 14);
				checksum.Final();

				*(uint32_t *)&newEncodedData.data()[14 + newEncodedDataSize] = checksum.ToUInt32();

				newEncodedDataSize += 18;

				for (size_t i = 0; i < gpGlobals->maxClients; i++) {
					client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

					if (!g_clientData[i].HasNewCodec) {
						continue;
					}
					if (!pDestClient->m_fZombie) {
						continue;
					}
					if (!(pDestClient->m_fHltv && g_pcvarForceSendHLTV->value != 0)) {
						if (playSound.receiver != nullptr && playSound.receiver != pDestClient) {
							continue;
						}
					}

					if (EngineUTIL::MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + newEncodedDataSize) {
						EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, SVC_VOICEDATA);
						EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, MAX_CLIENTS);
						EngineUTIL::MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, newEncodedDataSize);
						EngineUTIL::MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, newEncodedData.data(), newEncodedDataSize);
					}
				}
			}
		}
	}

	// TODO: delete with swap in for-loop
	g_playSounds.erase(
		std::remove_if(
			g_playSounds.begin(),
			g_playSounds.end(),
			[](const playSound_t &playSound) { return playSound.currentSample == playSound.samples8k.size() && playSound.PlayedNewEncodedSampleCount == playSound.samples16k.size(); }),
		g_playSounds.end());

	RETURN_META(MRES_IGNORED);
}

// MetaMod API
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION,  // ifvers
	"VoiceTranscoder",       // name
	VOICETRANSCODER_VERSION, // version
	"2020.03.06",            // date
	"WPMG.PRoSToC0der",      // author
	"http://vtc.wpmg.ru/",   // url
	"VTC",                   // logtag, all caps please
	PT_ANYTIME,              // (when) loadable
	PT_ANYTIME,              // (when) unloadable
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(char *pchInterfaceVersion, plugin_info_t **pPluginInfo, mutil_funcs_t *pMetaUtilFuncs) {
	*pPluginInfo = &Plugin_info;
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return TRUE;
}

void VTC_Path() {
#ifndef _WIN32
	Dl_info dlinfo;
	dladdr((void*)&VTC_Path, &dlinfo);
	LOG_CONSOLE(PLID, "%s", dlinfo.dli_fname);
	dladdr((void*)g_engfuncs.pfnPrecacheModel, &dlinfo);
	LOG_CONSOLE(PLID, "%s", dlinfo.dli_fname);
#endif
}

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
	}

	g_engfuncs.pfnAddServerCommand("VTC_Path", &VTC_Path);

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

// TODO: place it lower
void VTC_DeinitCodecs(void) {
	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		delete g_clientData[i].OldCodec;
		delete g_clientData[i].NewCodec;
		delete g_clientData[i].NewCodec2;
	}
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
	if (g_fThreadModeEnabled) {
		VTC_ThreadDeinit();
	}

	VTC_DeinitCodecs();

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

	if (pClientData->IsBlocked) {
		return;
	}
	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}
	if (!pClient->m_fZombie) {
		return;
	}

	auto now = steady_clock::now();

	if (pClientData->NextVoicePacketExpectedTime > now) {
		if (*pClientData->NextVoicePacketExpectedTime - now > milliseconds((int)g_pcvarMaxDelta->value)) {
			//LOG_MESSAGE(PLID, "Delta is %g", (pClientData->m_nextPacketTimeMicroSeconds - currentMicroSeconds)/1000.0);

			return;
		}
	}

	int16_t rawSamples[MAX_DECOMPRESSED_VOICEPACKET_SAMPLES];
	size_t rawSampleCount;

	// Validate new codec packet
	if (pClientData->HasNewCodec) {
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
			LOG_MESSAGE(PLID, "Invalid steamid (%llu) in voice packet from %s", steamid.ToUInt64(), pClient->m_szPlayerName);
			EngineUTIL::DropClient(pClient, false, "Invalid steamid (%llu) in voice packet", steamid.ToUInt64());

			return;
		}

		rawSampleCount = 0;
		while (buf.Size() - buf.TellRead() - sizeof(uint32_t) >= sizeof(uint8_t) + sizeof(uint16_t)) {
			uint8_t vpc = buf.ReadUInt8();

			switch (vpc) {
				case VPC_SETSAMPLERATE: {
					pClientData->SampleRate = buf.ReadUInt16();
					//LOG_MESSAGE(PLID, "%s %d", pClient->m_szPlayerName, pClientData->SampleRate);

					if (*pClientData->SampleRate != NEWCODEC_WANTED_SAMPLERATE && *pClientData->SampleRate != NEWCODEC_WANTED_SAMPLERATE2) {
						LOG_MESSAGE(PLID, "Voice packet unwanted samplerate (cur = %u, want = %u or %u) from %s", *pClientData->SampleRate, NEWCODEC_WANTED_SAMPLERATE, NEWCODEC_WANTED_SAMPLERATE2, pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Voice packet unwanted samplerate (cur = %u, want = %u or %u)", *pClientData->SampleRate, NEWCODEC_WANTED_SAMPLERATE, NEWCODEC_WANTED_SAMPLERATE2);

						return;
					}
				}
				break;
				case VPC_VDATA_SILENCE: {
					size_t silenceSampleCount = buf.ReadUInt16();

					if (!pClientData->SampleRate.HasValue()) {
						//LOG_MESSAGE(PLID, "Received silence samples when samplerate is not received from %s", pClient->m_szPlayerName);
						//EngineUTIL::DropClient(pClient, false, "Received silence samples when samplerate is not received");

						break;
					}

					silenceSampleCount /= (*pClientData->SampleRate / 8000);
					//LOG_MESSAGE(PLID, "%s S %d", pClient->m_szPlayerName, silenceSampleCount);

					if (silenceSampleCount > ARRAYSIZE(rawSamples) - rawSampleCount) {
						LOG_MESSAGE(PLID, "Too many silence samples (cur %u, max %u) from %s", rawSampleCount, ARRAYSIZE(rawSamples), pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Too many silence samples (cur %u, max %u)", rawSampleCount, ARRAYSIZE(rawSamples), steamid.ToUInt64());

						return;
					}

					memset(&rawSamples[rawSampleCount], 0, silenceSampleCount * sizeof(int16_t));
					rawSampleCount += silenceSampleCount;
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

						rawSampleCount += pClientData->NewCodec->Decode((const uint8_t *)buf.PeekRead(), bytesCount, &rawSamples[rawSampleCount], remainSamples);
						buf.SkipBytes(bytesCount);
					} else {
						LOG_MESSAGE(PLID, "Voice packet invalid vdata size (cur = %u, need = %u) from %s", remainBytes, bytesCount, pClient->m_szPlayerName);
						EngineUTIL::DropClient(pClient, false, "Voice packet invalid vdata size (cur = %u, need = %u)", remainBytes, bytesCount);

						return;
					}
				}
				break;
				case VPC_VDATA_OPUS_PLC: {
					uint16_t bytesCount = buf.ReadUInt16();

					size_t remainBytes = buf.Size() - buf.TellRead() - sizeof(uint32_t);
					if (remainBytes >= bytesCount) {
						size_t remainSamples = ARRAYSIZE(rawSamples) - rawSampleCount;

						if (remainSamples == 0) {
							LOG_MESSAGE(PLID, "Voice packet not enough space for samples from %s", pClient->m_szPlayerName);
							EngineUTIL::DropClient(pClient, false, "Voice packet not enough space for samples");

							return;
						}

						std::size_t decodedSampleCount = pClientData->NewCodec2->Decode((const uint8_t *)buf.PeekRead(), bytesCount, &rawSamples[rawSampleCount], remainSamples);
						
						// The bug is still not fixed in mainstream steamclient.dll, so we need to block any further propagation of invalid payloads
						if (decodedSampleCount == std::size_t(-1)) {
							LOG_MESSAGE(PLID, "Invalid voice packet from %s", pClient->m_szPlayerName);
							EngineUTIL::DropClient(pClient, false, "Invalid voice packet");

							return;
						}
						
						rawSampleCount += decodedSampleCount;
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
		rawSampleCount = pClientData->OldCodec->Decode((const uint8_t *)receivedBytes, receivedBytesCount, rawSamples, ARRAYSIZE(rawSamples));
	}

	//LOG_CONSOLE(PLID, "%f %d %d", currentMicroSeconds / 1000000.0, receivedBytesCount, rawSampleCount);

	/*if (!rawSampleCount) {
		LOG_MESSAGE(PLID, "WTF");
		FILE *pFile = fopen("VoiceTranscoder_VoiceDumpWTF.dat", "wb");
		fwrite(receivedBytes, sizeof(uint8_t), receivedBytesCount, pFile);
		fclose(pFile);
	}*/

	typedef duration<int64_t, ratio<1, 8000>> sample8k;
	auto frameTimeLength = sample8k(rawSampleCount);

	// There can be % 20ms != 0 packets
	//LOG_MESSAGE(PLID, "%g", microseconds(frameTimeLength).count()/1000.0);

	//LOG_MESSAGE(PLID, "Frame time: %f %f %d %d", currentMicroSeconds / 1000000.0, frameTimeLength / 1000.0, receivedBytesCount, needTranscode);

	if (pClientData->NextVoicePacketExpectedTime > now) {
		*pClientData->NextVoicePacketExpectedTime += frameTimeLength;
	} else {
		pClientData->NextVoicePacketExpectedTime = now + frameTimeLength;
	}

	if (pClientData->VoiceEndTime > now) {
		*pClientData->VoiceEndTime += frameTimeLength;
	} else {
		pClientData->VoiceEndTime = now + frameTimeLength + SPEAKING_TIMEOUT;

		if (!pClientData->IsSpeaking) {
			pClientData->IsSpeaking = true;

			g_OnClientStartSpeak(clientIndex);
		}
	}

	if (pClientData->IsMuted) {
		return;
	}

	bool needTranscode = false;
	for (size_t i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

		if (pDestClient == pClient) {
			continue;
		} else {
			if (!pDestClient->m_fZombie) {
				continue;
			}
			if (!pDestClient->m_fHltv || g_pcvarForceSendHLTV->value == 0) {
				if (!(pClient->m_bsVoiceStreams[0] & (1 << i))) {
					continue;
				}
			}
		}

		if (g_clientData[i].HasNewCodec != g_clientData[clientIndex - 1].HasNewCodec) {
			needTranscode = true;

			break;
		}
	}

	if (!rawSampleCount) {
		needTranscode = false;
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
		if (pClientData->HasNewCodec) {
			ChangeSamplesVolume(rawSamples, rawSampleCount, g_pcvarVolumeNewToOld->value);

			recompressedSize = pClientData->OldCodec->Encode(rawSamples, rawSampleCount, recompressed, ARRAYSIZE(recompressed));
		} else {
			ChangeSamplesVolume(rawSamples, rawSampleCount, g_pcvarVolumeOldToNew->value);

			recompressedSize = pClientData->NewCodec->Encode(rawSamples, rawSampleCount, &recompressed[14], ARRAYSIZE(recompressed) - 18);

			SteamID steamid;
			steamid.SetUniverse(UNIVERSE_PUBLIC);
			steamid.SetAccountType(ACCOUNT_TYPE_INDIVIDUAL);
			// Use different for separate channels
			steamid.SetAccountId(0xFFFFFFFF); // 0 is invalid, but maximum value valid, TODO: randomize or get non-steam user steamid?
			steamid.SetAccountInstance(STEAMUSER_DESKTOPINSTANCE);
			*(uint64_t *)recompressed = steamid.ToUInt64();
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
			if (!pDestClient->m_fZombie) {
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
		if (g_clientData[i].HasNewCodec == g_clientData[clientIndex-1].HasNewCodec) {
			buf = receivedBytes;
			byteCount = receivedBytesCount;
		} else {
			if (g_fThreadModeEnabled) {
				continue;
			}
			if (!needTranscode) {
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
	g_oldVoiceQuality = (size_t)CVAR_GET_FLOAT("sv_voicequality");

	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		g_clientData[i].OldCodec = new VoiceCodec_Speex(g_oldVoiceQuality);
		g_clientData[i].NewCodec = new VoiceCodec_SILK(5);
		g_clientData[i].NewCodec2 = new VoiceCodec_OPUS_PLC{};
	}
}

void VTC_UpdateCodecs() {
	g_oldVoiceQuality = (size_t)CVAR_GET_FLOAT("sv_voicequality");

	for (size_t i = 0; i < MAX_CLIENTS; i++) {
		g_clientData[i].OldCodec->ChangeQuality(g_oldVoiceQuality);
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

	if (CVAR_GET_POINTER("sv_voicecodec") == nullptr) {
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