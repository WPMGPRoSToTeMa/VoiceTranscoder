#pragma once

#include "UtilTypes.h"

class VoiceCodec {
protected:
	static const size_t BYTES_PER_SAMPLE = sizeof(short);

	virtual ~VoiceCodec() {}
public:
	// Reinitialization without recreating object
	virtual void ChangeQuality(size_t nQuality, size_t nSampleRate) = 0;
	virtual void ResetState() = 0;
	virtual size_t Encode(const void *pRaw, size_t nRawSamples, void *pEncoded, size_t nEncodedMaxSize) = 0;
	virtual size_t Decode(const void *pEncoded, size_t nEncodedSize, void *pRaw, size_t nRawMaxSamples) = 0;
};