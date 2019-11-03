#pragma once

#include "VoiceCodec.h"

#include <speex.h>

class Speex final : public VoiceCodec {
public:
	static const size_t s_rgEncodedFrameSize[];
	static const size_t SAMPLERATE = 8000;

	Speex(size_t nQuality, size_t nSampleRate);
	virtual ~Speex() final;
	// Reinitialization without recreating object
	virtual void ChangeQuality(size_t nQuality, size_t nSampleRate) final;
	virtual void ResetState() final;
	virtual size_t Encode(const short *pRaw, size_t nRawSamples, byte *pEncoded, size_t nEncodedMaxSize) final;
	virtual size_t Decode(const byte *pEncoded, size_t nEncodedSize, short *pRaw, size_t nRawMaxSamples) final;

private:
	void *m_pEncoder, *m_pDecoder;
	size_t m_nEncodedBytes;
	SpeexBits m_bits;
};