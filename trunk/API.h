#pragma once

#include "VoiceTranscoderAPI.h"
#include <UtilTypes.h>
#include <cstring>

const size_t MAX_CALLBACK_COUNT = 64;

template <typename ...T_ARGS>
class Event : public IEvent<T_ARGS...> {
	typedef void (* callback_t)(T_ARGS...);

	callback_t m_callbacks[MAX_CALLBACK_COUNT];
	size_t m_callbackCount;
public:
	Event() {
		m_callbackCount = 0;
	}
	virtual ~Event() {}

	void operator()(T_ARGS... args) {
		for (size_t i = 0; i < m_callbackCount; i++) {
			m_callbacks[i](args...);
		}
	}

	virtual void operator+=(callback_t callback) override final {
		//for (size_t i = 0; i < m_callbackCount; i++) {
		//	if (m_callbacks[i] == callback) {
		//		return;
		//	}
		//}

		if (m_callbackCount == MAX_CALLBACK_COUNT) {
			return;
		}

		m_callbacks[m_callbackCount++] = callback;
	};
	virtual void operator-=(callback_t callback) override final {
		for (size_t i = 0; i < m_callbackCount; i++) {
			if (m_callbacks[i] == callback) {
				memmove(&m_callbacks[i], &m_callbacks[i+1], (m_callbackCount - i - 1) * sizeof(callback_t));
				m_callbackCount--;

				return;
			}
		}
	};
};

class VoiceTranscoderAPI final : public IVoiceTranscoderAPI {
public:
	virtual ~VoiceTranscoderAPI() {}

	size_t MajorVersion() override;
	size_t MinorVersion() override;

	bool IsClientSpeaking(size_t clientIndex) override;

	IEvent<size_t>& OnClientStartSpeak() override;
	IEvent<size_t>& OnClientStopSpeak() override;

	void MuteClient(size_t clientIndex) override;
	void UnmuteClient(size_t clientIndex) override;
	bool IsClientMuted(size_t clientIndex) override;

	void PlaySound(size_t receiverClientIndex, const char *soundFilePath) override;

	void BlockClient(size_t clientIndex) override;
	void UnblockClient(size_t clientIndex) override;
	bool IsClientBlocked(size_t clientIndex) override;
};

extern Event<size_t> g_OnClientStartSpeak;
extern Event<size_t> g_OnClientStopSpeak;

extern VoiceTranscoderAPI g_voiceTranscoderAPI;