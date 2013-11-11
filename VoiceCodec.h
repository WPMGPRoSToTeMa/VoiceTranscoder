#pragma once

#include "Util.h"

class CVoiceCodec {
protected:
	static const int	c_iBytesPerSample = 2;

	virtual				~CVoiceCodec() {}

public:
	virtual bool		Init(int iQuality) = 0;
	virtual void		Release() = 0;
	virtual int			Compress(const short *psDecompressed, int nDecompressedSamples, byte *pbCompressed, int nMaxCompressedBytes) = 0;
	virtual int			Decompress(const byte *pbCompressed, int nCompressedBytes, short *psDecompressed, int nMaxDecompressedBytes) = 0;
	virtual bool		ResetState() = 0;
};