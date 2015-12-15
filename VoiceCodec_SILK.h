#pragma once

#include "VoiceCodec.h"
#include <SKP_Silk_SDK_API.h>

class SILK /*final*/ : public VoiceCodec {
public:
	static const size_t MAX_INPUTFRAMES = 5;
	static const size_t FRAMELENGTHMS = 20;
	static const size_t MIN_SAMPLES = 8000 * FRAMELENGTHMS / 1000;

	SILK(size_t quality);
	virtual ~SILK() /*final*/;
	// Reinitialization without recreating object
	virtual void ChangeQuality(size_t quality) /*final*/;
	virtual void ResetState() /*final*/;
	virtual size_t Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) /*final*/;
	virtual size_t Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) /*final*/;

private:
	void *m_encState;
	SKP_SILK_SDK_EncControlStruct m_encControl;
	void *m_decState;
	SKP_SILK_SDK_DecControlStruct m_decControl;
};