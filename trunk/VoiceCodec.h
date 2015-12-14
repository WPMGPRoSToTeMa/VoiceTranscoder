#pragma once

#include "UtilTypes.h"

class VoiceCodec {
protected:
	static const size_t BYTES_PER_SAMPLE = sizeof(uint16_t);

	virtual ~VoiceCodec() {}
public:
	// Reinitialization without recreating object
	virtual void ChangeQuality(size_t quality) = 0;
	virtual void ResetState() = 0;
	virtual size_t Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) = 0;
	virtual size_t Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) = 0;
};