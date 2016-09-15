#include "API.h"
#include <extdll.h>
#include <meta_api.h>
#include "Main.h"
#include <EngineUTIL.h>

Event<size_t> g_OnClientStartSpeak;
Event<size_t> g_OnClientStopSpeak;
Event<size_t, bool&> g_OnShouldAllowVoicePacket;

VoiceTranscoderAPI g_voiceTranscoderAPI;

size_t VoiceTranscoderAPI::MajorVersion() {
	return VOICETRANSCODER_API_VERSION_MAJOR;
}

size_t VoiceTranscoderAPI::MinorVersion() {
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

IEvent<size_t> &VoiceTranscoderAPI::OnClientStartSpeak() {
	return g_OnClientStartSpeak;
}

IEvent<size_t> &VoiceTranscoderAPI::OnClientStopSpeak() {
	return g_OnClientStopSpeak;
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

IEvent<size_t, bool &> &VoiceTranscoderAPI::OnShouldAllowVoicePacket() {
	return g_OnShouldAllowVoicePacket;
}
