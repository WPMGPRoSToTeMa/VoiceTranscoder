#if !defined VOICEENCODER_SPEEX_H
#define VOICEENCODER_SPEEX_H

#pragma once

#include "ivoicecodec.h"
#include "utlbuffer.h"

#include <stdlib.h>

#include <SKP_Silk_SDK_API.h>

extern IVoiceCodec *CreateSilkVoiceCodec();

class VoiceEncoder_Silk : public IVoiceCodec
{
public:
	static const int s_iMaxInputFrames = 5;
	static const int s_iFrameLengthMs = 20;
	static const int s_iMaxApiFsKhz = 8;

	VoiceEncoder_Silk();
	virtual ~VoiceEncoder_Silk();

	virtual bool Init(int iQuality);
	virtual void Release();
	virtual int Compress(const char *pUncompressed, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal);
	virtual int Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes);
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

#endif