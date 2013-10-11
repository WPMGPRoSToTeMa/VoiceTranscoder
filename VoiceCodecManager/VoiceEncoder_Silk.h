#if !defined VOICEENCODER_SPEEX_H
#define VOICEENCODER_SPEEX_H

#pragma once

#include "ivoicecodec.h"
#include "utlbuffer.h"

#include <stdlib.h>

#include <SKP_Silk_SDK_API.h>

#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps 
#define MAX_INPUT_FRAMES        5
#define FRAME_LENGTH_MS         20
#define MAX_API_FS_KHZ          8
#define MAX_FRAMEBUFFER_SAMPLES	16383

class VoiceEncoder_Silk : public IVoiceCodec
{
public:
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