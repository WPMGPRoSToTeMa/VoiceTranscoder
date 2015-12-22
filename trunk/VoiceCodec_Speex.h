#pragma once

#include "VoiceCodec.h"
#include <speex.h>

class VoiceCodec_Speex final : public VoiceCodec {
public:
	static const size_t ENCODED_FRAMESIZE[];
	static const size_t SAMPLERATE = 8000;
	static const size_t FRAMESIZE = 160;

	VoiceCodec_Speex(size_t quality);
	virtual ~VoiceCodec_Speex() final;
	// Reinitialization without recreating object
	virtual void ChangeQuality(size_t quality) override final;
	virtual void ResetState() override final;
	virtual size_t Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) override final;
	virtual size_t Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) override final;

private:
	void *m_encoder, *m_decoder;
	size_t m_encodedBytes;
	SpeexBits m_bits;
};