#pragma once

#include <cstddef>

const size_t VOICETRANSCODER_API_VERSION_MAJOR = 1;
const size_t VOICETRANSCODER_API_VERSION_MINOR = 1;

template <typename ...T_ARGS>
class IVoidCallbackRegistry {
public:
	virtual ~IVoidCallbackRegistry() {}

	typedef void (* callback_t)(T_ARGS...);

	virtual void RegisterCallback(callback_t callback) = 0;
	virtual void UnregisterCallback(callback_t callback) = 0;
};

typedef IVoidCallbackRegistry<size_t> ICallbackRegistry_ClientStartSpeak;
typedef IVoidCallbackRegistry<size_t> ICallbackRegistry_ClientStopSpeak;

class IVoiceTranscoderAPI {
public:
	virtual ~IVoiceTranscoderAPI() {}

	virtual size_t GetMajorVersion() = 0;
	virtual size_t GetMinorVersion() = 0;

	virtual bool IsClientSpeaking(size_t clientIndex) = 0;

	virtual ICallbackRegistry_ClientStartSpeak *ClientStartSpeak() = 0;
	virtual ICallbackRegistry_ClientStopSpeak *ClientStopSpeak() = 0;

	virtual void MuteClient(size_t clientIndex) = 0;
	virtual void UnmuteClient(size_t clientIndex) = 0;
	virtual bool IsClientMuted(size_t clientIndex) = 0;
};