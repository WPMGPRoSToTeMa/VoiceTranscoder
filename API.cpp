#include "API.h"
#include <extdll.h>
#include <meta_api.h>
#include "Main.h"
#include <EngineUTIL.h>

CallbackRegistry_ClientStartSpeak g_callback_ClientStartSpeak;
CallbackRegistry_ClientStopSpeak g_callback_ClientStopSpeak;

VoiceTranscoderAPI g_voiceTranscoderAPI;

size_t VoiceTranscoderAPI::GetMajorVersion() {
	return VOICETRANSCODER_API_VERSION_MAJOR;
}

size_t VoiceTranscoderAPI::GetMinorVersion() {
	return VOICETRANSCODER_API_VERSION_MINOR;
}

bool VoiceTranscoderAPI::IsClientSpeaking(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return false;
	}

	client_t *pClient = EngineUTIL::GetClientByIndex(clientIndex);
	if (!pClient->m_fActive) {
		return false;
	}

	return g_clientData[clientIndex - 1].m_isSpeaking;
}

ICallbackRegistry_ClientStartSpeak *VoiceTranscoderAPI::ClientStartSpeak() {
	return &g_callback_ClientStartSpeak;
}

ICallbackRegistry_ClientStopSpeak *VoiceTranscoderAPI::ClientStopSpeak() {
	return &g_callback_ClientStopSpeak;
}

void VoiceTranscoderAPI::MuteClient(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return;
	}

	g_clientData[clientIndex - 1].m_isMuted = true;
}

void VoiceTranscoderAPI::UnmuteClient(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return;
	}

	g_clientData[clientIndex - 1].m_isMuted = false;
}

bool VoiceTranscoderAPI::IsClientMuted(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return false;
	}

	return g_clientData[clientIndex - 1].m_isMuted;
}