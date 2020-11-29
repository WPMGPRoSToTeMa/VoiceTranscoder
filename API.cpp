#include "API.h"
#include <extdll.h>
#include <meta_api.h>
#include "Main.h"
#include <EngineUTIL.h>
#include <memory>
#include "VoiceCodecs/SILK/SKP_Silk_SigProc_FIX.h"
#include <string>

Event<size_t> g_OnClientStartSpeak;
Event<size_t> g_OnClientStopSpeak;

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

	return g_clientData[clientIndex - 1].IsSpeaking;
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

	g_clientData[clientIndex - 1].IsMuted = true;
}

void VoiceTranscoderAPI::UnmuteClient(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return;
	}

	g_clientData[clientIndex - 1].IsMuted = false;
}

bool VoiceTranscoderAPI::IsClientMuted(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return false;
	}

	return g_clientData[clientIndex - 1].IsMuted;
}

void VoiceTranscoderAPI::PlaySound(size_t receiverClientIndex, const char *soundFilePath) {
	if (receiverClientIndex > gpGlobals->maxClients) {
		return;
	}

	client_t *client = nullptr;
	if (receiverClientIndex != 0) {
		client = EngineUTIL::GetClientByIndex(receiverClientIndex);

		if (!client->m_fSpawned)
			return;
	}
	
	if (soundFilePath == NULL || soundFilePath[0] == '\0')
	{
		for(auto & s : playSound)
		{
			if (client == playSound.receiver)
			{
				s.currentSample = 0x7FFFFFFF;
				return;
			}
		}
	}

	auto gamedirAbsPath = std::string(GET_GAME_INFO(PLID, GINFO_GAMEDIR));
	auto file = fopen((gamedirAbsPath + "/" + soundFilePath).c_str(), "rb");
	if (file == nullptr)
		return;

	while (true) {
		if (fgetc(file) == 'f') {
			if (fgetc(file) == 'm') {
				if (fgetc(file) == 't') {
					if (fgetc(file) == ' ') {
						break;
					}
				}
			}
		}
	}

	fseek(file, 0x8, SEEK_CUR);
	uint32_t inSampleRate;
	fread(&inSampleRate, sizeof(inSampleRate), 1, file);

	fseek(file, 0x6, SEEK_CUR);
	uint16_t inBitDepth;
	fread(&inBitDepth, sizeof(inBitDepth), 1, file);

	while (true) {
		if (fgetc(file) == 'd') {
			if (fgetc(file) == 'a') {
				if (fgetc(file) == 't') {
					if (fgetc(file) == 'a') {
						break;
					}
				}
			}
		}
	}

	uint32_t inDataSize;
	fread(&inDataSize, sizeof(inDataSize), 1, file);

	auto inData = std::make_unique<uint8_t[]>(inDataSize);
	fread(inData.get(), sizeof(decltype(inData)::element_type), inDataSize, file);

	fclose(file);

	size_t inSampleCount;
	std::unique_ptr<int16_t[]> inSamples;
	if (inBitDepth == 8) {
		inSampleCount = inDataSize;
		inSamples = std::make_unique<int16_t[]>(inDataSize);

		for (size_t i = 0; i < inSampleCount; i++) {
			inSamples[i] = int16_t(*(int8_t *)&inData[i] - 128) * 256;
		}

		inData.reset();
	} else {
		inSampleCount = inDataSize / sizeof(int16_t);
		inSamples = std::unique_ptr<int16_t[]>((int16_t *)inData.release());
	}

	playSound_t playSound;
	playSound.receiver = client;
	playSound.currentSample = 0;
	playSound.PlayedNewEncodedSampleCount = 0;
	playSound.NewEncodedDataSendTime = (playSound.OldEncodedDataSendTime = steady_clock::now());
	playSound.oldCodec = std::make_unique<VoiceCodec_Speex>(CVAR_GET_FLOAT("sv_voicequality"));
	playSound.newCodec = std::make_unique<VoiceCodec_SILK>(10);

	{
		size_t outSampleRate = 8000;
		size_t outSampleCount = size_t(ceil(double(inSampleCount) * (double(outSampleRate) / double(inSampleRate))));
		playSound.samples8k.resize(outSampleCount);

		SKP_Silk_resampler_state_struct resamplerState;
		SKP_Silk_resampler_init(&resamplerState, inSampleRate, outSampleRate);
		SKP_Silk_resampler(&resamplerState, playSound.samples8k.data(), inSamples.get(), inSampleCount);
	}
	{
		// TODO: we need to choose optimal samplerate for steam
		size_t outSampleRate = 16000;
		size_t outSampleCount = size_t(ceil(double(inSampleCount) * (double(outSampleRate) / double(inSampleRate))));
		playSound.samples16k.resize(outSampleCount);

		SKP_Silk_resampler_state_struct resamplerState;
		SKP_Silk_resampler_init(&resamplerState, inSampleRate, outSampleRate);
		SKP_Silk_resampler(&resamplerState, playSound.samples16k.data(), inSamples.get(), inSampleCount);
	}

	g_playSounds.push_back(std::move(playSound));
}

void VoiceTranscoderAPI::BlockClient(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return;
	}

	g_clientData[clientIndex - 1].IsBlocked = true;
}

void VoiceTranscoderAPI::UnblockClient(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return;
	}

	g_clientData[clientIndex - 1].IsBlocked = false;
}

bool VoiceTranscoderAPI::IsClientBlocked(size_t clientIndex) {
	if (clientIndex < 1 || clientIndex > gpGlobals->maxClients) {
		return false;
	}

	return g_clientData[clientIndex - 1].IsBlocked;
}
