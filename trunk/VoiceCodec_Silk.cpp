#include "VoiceCodec_Silk.h"
#include "CRC32.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define NOSTEAM_TEST

CSilk::CSilk() {
	m_pEncState = NULL;
	m_pDecState = NULL;
	m_iSampleRate = 0;
	m_iBitRate = 25000;
	m_iPacketLossPercentage = 0;
}

CSilk::~CSilk() {
	free(m_pEncState);
	free(m_pDecState);
}

bool CSilk::Init(int iQuality) {
	int iSize;

	m_iSampleRate = 16000;
	m_iBitRate = 25000;
	m_iPacketLossPercentage = 0;

	SKP_Silk_SDK_Get_Encoder_Size(&iSize);
	m_pEncState = malloc(iSize);
	SKP_Silk_SDK_InitEncoder(m_pEncState, &m_encStatus);

	SKP_Silk_SDK_Get_Decoder_Size(&iSize);
	m_pDecState = malloc(iSize);
	SKP_Silk_SDK_InitDecoder(m_pDecState);

	return true;
}

void CSilk::Release() {
	delete this;
}

int CSilk::Compress(const short *psDecompressed, int nDecompressedSamples, byte *pbCompressed, int nMaxCompressedBytes) {
	// Standard size of header
	pbCompressed += 14;

	m_iSampleRate = 8000;

	int nMinSamples = c_iMaxInputFrames * m_iSampleRate * c_iFrameLengthMs / 1000;

	if (nDecompressedSamples <= 0) {
		return 0;
	}

	m_buffEncode.Put(psDecompressed, nDecompressedSamples * c_iBytesPerSample);

	if ((m_buffEncode.TellPut() / c_iBytesPerSample) < nMinSamples) {
		return 0;
	}

	short nBytesOut;
	int nSamplesRemain;
	CUtlBuffer buffCompressed(pbCompressed, nMaxCompressedBytes);

	nMinSamples = m_iSampleRate * c_iFrameLengthMs / 1000;

	nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / c_iBytesPerSample;

	while (nSamplesRemain >= nMinSamples) {
		nBytesOut = min(buffCompressed.Size() - buffCompressed.TellPut() - sizeof(short), 0x7FFF);

		m_encStatus.API_sampleRate = m_iSampleRate;
		m_encStatus.packetSize = nMinSamples;
		m_encStatus.packetLossPercentage = m_iPacketLossPercentage;
		m_encStatus.bitRate = m_iBitRate;
		m_encStatus.maxInternalSampleRate = 8000;
		m_encStatus.useInBandFEC = 0;
		m_encStatus.useDTX = 1;
		m_encStatus.complexity = 2;

		SKP_Silk_SDK_Encode(m_pEncState, &m_encStatus, (const short *)m_buffEncode.PeekGet(), nMinSamples, (unsigned char *)buffCompressed.PeekPut(sizeof(short)), (short *)&nBytesOut);

		m_buffEncode.SeekGet(CUtlBuffer::SEEK_CURRENT, nMinSamples * c_iBytesPerSample);

		buffCompressed.PutShort(nBytesOut);
		buffCompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nBytesOut);

		nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / c_iBytesPerSample;
	}

	m_buffEncode.Clear();

	if (nSamplesRemain && nSamplesRemain <= nDecompressedSamples) {
		m_buffEncode.Put(&psDecompressed[nDecompressedSamples - nSamplesRemain], nSamplesRemain << 1);
	}

	int iSize = buffCompressed.TellPut();

	if (iSize)
	{
		pbCompressed -= 14;

		buffCompressed.SetExternalBuffer(pbCompressed, nMaxCompressedBytes);
		
		buffCompressed.PutUnsignedInt(0xFFFFFFFF);
		buffCompressed.PutUnsignedInt(0x1100001);
		buffCompressed.PutUnsignedChar(0xB);
		buffCompressed.PutShort(m_iSampleRate * 2);
		buffCompressed.PutUnsignedChar(0x4);
		buffCompressed.PutShort(iSize);

		buffCompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, iSize);

		unsigned int uiCRC = ~ComputeCRC(0xFFFFFFFF, buffCompressed.Base(), buffCompressed.TellPut());
		
		buffCompressed.PutUnsignedInt(uiCRC);
	}

	return buffCompressed.TellPut();
}

int CSilk::Decompress(const byte *pbCompressed, int nCompressedBytes, short *psDecompressed, int nMaxUncompressedBytes)
{
	pbCompressed += 14;
	nCompressedBytes -= 18;

	m_iSampleRate = 8000;

	if (nCompressedBytes <= 0) {
		return 0;
	}

	int iLen;
	short nSamples;
	CUtlBuffer buffCompressed(pbCompressed, nCompressedBytes);
	CUtlBuffer buffDecompressed(psDecompressed, nMaxUncompressedBytes);

	m_decStatus.API_sampleRate = m_iSampleRate;

	int nMinSamples = m_iSampleRate * c_iFrameLengthMs / 1000;

	while (buffCompressed.TellGet() + 2 <= buffCompressed.Size()) {
		iLen = buffCompressed.GetUnsignedShort();

		if (iLen == 65535) {
			ResetState();

			return buffDecompressed.TellPut() / c_iBytesPerSample;
		}

		if (buffCompressed.TellGet() + iLen > buffCompressed.Size()) {
			return buffDecompressed.TellPut() / c_iBytesPerSample;
		}

		if (buffDecompressed.TellPut() + nMinSamples * c_iBytesPerSample > buffDecompressed.Size()) {
			return buffDecompressed.TellPut() / c_iBytesPerSample;
		}

		memset(buffDecompressed.PeekPut(), 0, nMinSamples * c_iBytesPerSample);

		if (iLen) {
			nSamples = (buffDecompressed.Size() - buffDecompressed.TellPut()) / c_iBytesPerSample;

			if (SKP_Silk_SDK_Decode(m_pDecState, &m_decStatus, 0, (const unsigned char *)buffCompressed.PeekGet(), iLen, (short *)buffDecompressed.PeekPut(), &nSamples)) {
				return buffDecompressed.TellPut() / c_iBytesPerSample;
			}

			buffCompressed.SeekGet(CUtlBuffer::SEEK_CURRENT, iLen);
			buffDecompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nSamples * c_iBytesPerSample);

			if (m_decStatus.moreInternalDecoderFrames) {
				return buffDecompressed.TellPut() / c_iBytesPerSample;
			}
		} else {
			buffDecompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nMinSamples * c_iBytesPerSample);
		}
	}

	return buffDecompressed.TellPut() / c_iBytesPerSample;
}

bool CSilk::ResetState() {
	if (m_pEncState) {
		SKP_Silk_SDK_InitEncoder(m_pEncState, &m_encStatus);
	}
	if (m_pDecState) {
		SKP_Silk_SDK_InitDecoder(m_pDecState);
	}

	m_buffEncode.Clear();

	return true;
}