#pragma once

#include "VoiceCodec.h"

#include "UtlBuffer.h"

#include <speex.h>

class CSpeex : public CVoiceCodec {
public:
	static const int c_iMaxBufferSamples = 1024;
	static const int c_iSampleRate = 8000;
	static const int c_iRawFrameSize = 160;
	static const int c_rgiEncodedFrameSize[11];
	static const int c_nRawBytes = c_iRawFrameSize * c_iBytesPerSample;
	static const int c_nRawSamples = c_iRawFrameSize;

	CSpeex();
	virtual ~CSpeex();

	virtual bool	Init(int iQuality);
	virtual void	Release();
	virtual int		Compress(const short *psDecompressed, int nDecompressedSamples, byte *pbCompressed, int nMaxCompressedBytes);
	virtual int		Decompress(const byte *pbCompressed, int nCompressedBytes, short *psDecompressed, int nMaxDecompressedBytes);
	virtual bool	ResetState();

	CUtlBuffer		m_buffEncode;

	int				m_nEncodedBytes;

private:
	bool			InitStates();
	void			TermStates();

	int				m_iQuality;
	void *			m_pEncoderState;
	void *			m_pDecoderState;

	SpeexBits		m_bits;
};