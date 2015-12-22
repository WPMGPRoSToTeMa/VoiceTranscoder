#pragma once

#include <UtilTypes.h>
#include <cstring>

const size_t VOICETRANSCODER_API_VERSION_MAJOR = 1;
const size_t VOICETRANSCODER_API_VERSION_MINOR = 0;

const size_t MAX_CALLBACK_COUNT = 64;

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

template <typename ...T_ARGS>
class VoidCallbackRegistry : public IVoidCallbackRegistry<T_ARGS...> {
	typedef void (* callback_t)(T_ARGS...);

	callback_t m_callbacks[MAX_CALLBACK_COUNT];
	size_t m_callbackCount;
public:
	VoidCallbackRegistry() {
		m_callbackCount = 0;
	}
	virtual ~VoidCallbackRegistry() {}

	void Call(T_ARGS... args) {
		for (size_t i = 0; i < m_callbackCount; i++) {
			m_callbacks[i](args...);
		}
	}

	virtual void RegisterCallback(callback_t callback) override final {
		for (size_t i = 0; i < m_callbackCount; i++) {
			if (m_callbacks[i] == callback) {
				return;
			}
		}

		if (m_callbackCount == MAX_CALLBACK_COUNT) {
			return;
		}

		m_callbacks[m_callbackCount++] = callback;
	};
	virtual void UnregisterCallback(callback_t callback) override final {
		for (size_t i = 0; i < m_callbackCount; i++) {
			if (m_callbacks[i] == callback) {
				memmove(&m_callbacks[i], &m_callbacks[i+1], (m_callbackCount - i - 1) * sizeof(callback_t));
				m_callbackCount--;

				return;
			}
		}
	};
};

typedef VoidCallbackRegistry<size_t> CallbackRegistry_ClientStartSpeak;
typedef VoidCallbackRegistry<size_t> CallbackRegistry_ClientStopSpeak;

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
};

class VoiceTranscoderAPI : public IVoiceTranscoderAPI {
public:
	virtual ~VoiceTranscoderAPI() {}

	virtual size_t GetMajorVersion() override final;
	virtual size_t GetMinorVersion() override final;

	virtual bool IsClientSpeaking(size_t clientIndex) override final;

	virtual ICallbackRegistry_ClientStartSpeak *ClientStartSpeak() override final;
	virtual ICallbackRegistry_ClientStopSpeak *ClientStopSpeak() override final;

	virtual void MuteClient(size_t clientIndex) override final;
	virtual void UnmuteClient(size_t clientIndex) override final;
};

extern CallbackRegistry_ClientStartSpeak g_callback_ClientStartSpeak;
extern CallbackRegistry_ClientStopSpeak g_callback_ClientStopSpeak;

extern VoiceTranscoderAPI g_voiceTranscoderAPI;