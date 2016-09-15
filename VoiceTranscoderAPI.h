#pragma once

#include <cstddef>

const size_t VOICETRANSCODER_API_VERSION_MAJOR = 2;
const size_t VOICETRANSCODER_API_VERSION_MINOR = 0;

template <typename ...T_ARGS>
class IEvent {
public:
	virtual ~IEvent() {}

	typedef void (* callback_t)(T_ARGS...);

	virtual void operator+=(callback_t callback) = 0;
	virtual void operator-=(callback_t callback) = 0;
};

class IVoiceTranscoderAPI {
public:
	virtual ~IVoiceTranscoderAPI() {}

	virtual size_t MajorVersion() = 0;
	virtual size_t MinorVersion() = 0;

	virtual bool IsClientSpeaking(size_t clientIndex) = 0;

	virtual IEvent<size_t>& OnClientStartSpeak() = 0;
	virtual IEvent<size_t>& OnClientStopSpeak() = 0;

	virtual void MuteClient(size_t clientIndex) = 0;
	virtual void UnmuteClient(size_t clientIndex) = 0;
	virtual bool IsClientMuted(size_t clientIndex) = 0;

	virtual IEvent<size_t, bool&>& OnShouldAllowVoicePacket() = 0;
};