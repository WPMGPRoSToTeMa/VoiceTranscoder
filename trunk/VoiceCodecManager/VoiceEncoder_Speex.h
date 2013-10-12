#pragma once

#include "iframeencoder.h"

#include <speex.h>

extern IVoiceCodec *CreateSpeexVoiceCodec();

class VoiceEncoder_Speex : public IFrameEncoder
{
public:
	VoiceEncoder_Speex();
	virtual ~VoiceEncoder_Speex();

	// Interfaces IFrameDecoder

	bool Init(int quality, int &rawFrameSize, int &encodedFrameSize);
	void Release();
	void DecodeFrame(const char *pCompressed, char *pDecompressedBytes);
	void EncodeFrame(const char *pUncompressedBytes, char *pCompressed);
	bool ResetState();

private:

	bool	InitStates();
	void	TermStates();

	int			m_Quality;		// voice codec quality ( 0,2,4,6,8 )
	void *		m_EncoderState;	// speex internal encoder state
	void *		m_DecoderState; // speex internal decoder state

	SpeexBits	m_Bits;	// helpful bit buffer structure
};