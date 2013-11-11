#pragma once

#include "VoiceCodec.h"
#include "UtlBuffer.h"

#include <stdlib.h>

#include <SKP_Silk_SDK_API.h>

class CSilk : public CVoiceCodec
{
public:
	static const int c_iMaxInputFrames = 5;
	static const int c_iFrameLengthMs = 20;
	static const int c_iMaxApiFsKhz = 8;

	CSilk();
	virtual ~CSilk();

	virtual bool Init(int iQuality);
	virtual void Release();
	virtual int Compress(const short *psDecompressed, int nDecompressedSamples, byte *pbCompressed, int nMaxCompressedBytes);
	virtual int Decompress(const byte *pbCompressed, int nCompressedBytes, short *psDecompressed, int nMaxDecompressedBytes);
	virtual bool ResetState();

	void *m_pEncState;
	int m_iSampleRate;
	int m_iBitRate;
	int m_iPacketLossPercentage;
	SKP_SILK_SDK_EncControlStruct m_encStatus;
	CUtlBuffer m_buffEncode;
	void *m_pDecState;
	SKP_SILK_SDK_DecControlStruct m_decStatus;
};