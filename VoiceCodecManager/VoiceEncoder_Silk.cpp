#include "VoiceEncoder_Silk.h"
#include "CRC32.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define NOSTEAM_TEST

IBaseInterface *CreateSilkVoiceCodec() {
	return (IBaseInterface *)new VoiceEncoder_Silk;
}

EXPOSE_INTERFACE_FN(CreateSilkVoiceCodec, IVoiceCodec, "voice_silk")

VoiceEncoder_Silk::VoiceEncoder_Silk() {
	m_pEncState = NULL;
	m_pDecState = NULL;
	m_iSampleRate = 0;
	m_iBitRate = 25000;
	m_iPacketLossPercentage = 0;
}

VoiceEncoder_Silk::~VoiceEncoder_Silk() {
	free(m_pEncState);
	free(m_pDecState);
}

bool VoiceEncoder_Silk::Init(int iQuality) {
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

void VoiceEncoder_Silk::Release() {
	delete this;
}

#include <stdio.h>

int VoiceEncoder_Silk::Compress(const char *psUncompressed, int nSamples, char *pbCompressed, int nMaxCompressedBytes, bool fFinal) {
	// Standard size of header
	pbCompressed += 14;

	m_iSampleRate = 8000;

	int nMinSamples = MAX_INPUT_FRAMES * m_iSampleRate * FRAME_LENGTH_MS / 1000;

	m_buffEncode.Put(psUncompressed, nSamples * BYTES_PER_SAMPLE);

	if ((m_buffEncode.TellPut() / BYTES_PER_SAMPLE) < nMinSamples && !fFinal) {
		return NULL;
	}

	short nBytesOut;
	int nSamplesRemain;
	CUtlBuffer buffCompressed(pbCompressed, nMaxCompressedBytes);

	nMinSamples = m_iSampleRate * FRAME_LENGTH_MS / 1000;

	nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / BYTES_PER_SAMPLE;

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

		m_buffEncode.SeekGet(CUtlBuffer::SEEK_CURRENT, nMinSamples * BYTES_PER_SAMPLE);

		buffCompressed.PutShort(nBytesOut);
		buffCompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nBytesOut);

		nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / BYTES_PER_SAMPLE;
	}

	m_buffEncode.Clear();

	if (nSamplesRemain && nSamplesRemain <= nSamples) {
		m_buffEncode.Put(&psUncompressed[nSamples - nSamplesRemain], nSamplesRemain << 1);
	}

	if (fFinal) {
		ResetState();

		if (buffCompressed.Size() - buffCompressed.TellPut() - sizeof(short)) {
			buffCompressed.PutUnsignedShort(0xFFFF);
		}
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

int VoiceEncoder_Silk::Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes)
{
	pCompressed += 14;
	compressedBytes -= 18;

	m_iSampleRate = 8000;

	if (compressedBytes <= 0) {
		return 0;
	}

	int iLen;
	short nSamples;
	CUtlBuffer buffCompressed(pCompressed, compressedBytes);
	CUtlBuffer buffDecompressed(pUncompressed, maxUncompressedBytes);

	m_decStatus.API_sampleRate = m_iSampleRate;

	int nMinSamples = m_iSampleRate * FRAME_LENGTH_MS / 1000;

	while (buffCompressed.TellGet() + 2 <= buffCompressed.Size()) {
		iLen = buffCompressed.GetUnsignedShort();

		if (iLen == 65535) {
			ResetState();

			return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;
		}

		if (buffCompressed.TellGet() + iLen > buffCompressed.Size()) {
			return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;
		}

		if (buffDecompressed.TellPut() + nMinSamples * BYTES_PER_SAMPLE > buffDecompressed.Size()) {
			return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;
		}

		memset(buffDecompressed.PeekPut(), 0, nMinSamples * BYTES_PER_SAMPLE);

		if (iLen) {
			nSamples = (buffDecompressed.Size() - buffDecompressed.TellPut()) / BYTES_PER_SAMPLE;

			if (SKP_Silk_SDK_Decode(m_pDecState, &m_decStatus, 0, (const unsigned char *)buffCompressed.PeekGet(), iLen, (short *)buffDecompressed.PeekPut(), &nSamples)) {
				return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;
			}

			buffCompressed.SeekGet(CUtlBuffer::SEEK_CURRENT, iLen);
			buffDecompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nSamples * BYTES_PER_SAMPLE);

			if (m_decStatus.moreInternalDecoderFrames) {
				return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;
			}
		} else {
			buffDecompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, nMinSamples * BYTES_PER_SAMPLE);
		}
	}

	return buffDecompressed.TellPut() / BYTES_PER_SAMPLE;

	/*m_nEncodedBytes = *(short *)&pCompressed[curCompressedByte];
	curCompressedByte += sizeof(short);

	if (m_nEncodedBytes == -1)
	{
		ResetState();

		return nDecompressedBytes / BYTES_PER_SAMPLE;
	}

	while ((compressedBytes - curCompressedByte) >= m_nEncodedBytes)
	{
		iLen = maxUncompressedBytes - nDecompressedBytes;
		SKP_Silk_SDK_Decode(m_pDecState, &m_decStatus, NULL, (const unsigned char *)&pCompressed[curCompressedByte], m_nEncodedBytes, (short *)&pUncompressed[nDecompressedBytes], (short *)&iLen);
		curCompressedByte += m_nEncodedBytes;
		nDecompressedBytes += iLen * BYTES_PER_SAMPLE;

		if (curCompressedByte >= compressedBytes)
			break;

		m_nEncodedBytes = *(short *)&pCompressed[curCompressedByte];
		curCompressedByte += sizeof(short);

		if (m_nEncodedBytes == -1)
		{
			ResetState();

			return nDecompressedBytes / BYTES_PER_SAMPLE;
		}
	}

	return nDecompressedBytes / BYTES_PER_SAMPLE;*/
}

bool VoiceEncoder_Silk::ResetState() {
	if (m_pEncState) {
		SKP_Silk_SDK_InitEncoder(m_pEncState, &m_encStatus);
	}
	if (m_pDecState) {
		SKP_Silk_SDK_InitDecoder(m_pDecState);
	}

	m_buffEncode.Clear();

	return true;
}