#include "VoiceCodec_Speex.h"

const int CSpeex::c_rgiEncodedFrameSize[11] = {6,6,15,15,20,20,28,28,38,38,38};

CSpeex::CSpeex() {
	m_pEncoderState = NULL;
	m_pDecoderState = NULL;
	m_iQuality = 0;
	m_nEncodedBytes = 0;
}

CSpeex::~CSpeex() {
	if(m_pEncoderState) {
		speex_encoder_destroy(m_pEncoderState);
		m_pEncoderState = NULL;
	}

	if(m_pDecoderState) {
		speex_decoder_destroy(m_pDecoderState);
		m_pDecoderState = NULL;
	}

	speex_bits_destroy(&m_bits);
}

bool CSpeex::Init(int iQuality) {
	speex_bits_init(&m_bits);

	m_pEncoderState = speex_encoder_init(&speex_nb_mode);	// narrow band mode 8kbp
	m_pDecoderState = speex_decoder_init(&speex_nb_mode);

	if (!m_pEncoderState || !m_pDecoderState) {
		return false;
	}

	m_iQuality = (iQuality - 1) * 2;

	m_nEncodedBytes = c_rgiEncodedFrameSize[m_iQuality];

	speex_encoder_ctl( m_pEncoderState, SPEEX_SET_QUALITY, &m_iQuality);

	bool fPostFilter = true; // Set the perceptual enhancement on
	speex_decoder_ctl(m_pDecoderState, SPEEX_SET_ENH, &fPostFilter);

	int iSampleRate = c_iSampleRate;
	speex_decoder_ctl(m_pDecoderState, SPEEX_SET_SAMPLING_RATE, &iSampleRate);
	speex_encoder_ctl(m_pEncoderState, SPEEX_SET_SAMPLING_RATE, &iSampleRate);

	return true;
}

void CSpeex::Release() {
	delete this;
}

int	CSpeex::Compress(const short *psDecompressed, int nDecompressedSamples, byte *pbCompressed, int nMaxCompressedBytes) {
	m_buffEncode.Put(psDecompressed, nDecompressedSamples * c_iBytesPerSample);

	if (m_buffEncode.TellPut() < c_nRawBytes) {
		return 0;
	}

	float rgflInput[c_iRawFrameSize];
	CUtlBuffer buffCompressed(pbCompressed, nMaxCompressedBytes);

	int nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / c_iBytesPerSample;
	
	while (nSamplesRemain >= c_nRawSamples && (buffCompressed.Size() - buffCompressed.TellPut()) >= m_nEncodedBytes) {
		for (int i = 0; i < c_iRawFrameSize; i++) {
			rgflInput[i] = (float)m_buffEncode.GetShort();
		}

		speex_bits_reset(&m_bits);

		speex_encode(m_pEncoderState, rgflInput, &m_bits);

		int size = speex_bits_write(&m_bits, (char *)buffCompressed.PeekPut(), m_nEncodedBytes);

		buffCompressed.SeekPut(CUtlBuffer::SEEK_CURRENT, m_nEncodedBytes);

		nSamplesRemain = (m_buffEncode.TellPut() - m_buffEncode.TellGet()) / c_iBytesPerSample;
	}

	m_buffEncode.Clear();

	if (nSamplesRemain && nSamplesRemain <= nDecompressedSamples) {
		m_buffEncode.Put(&psDecompressed[nDecompressedSamples - nSamplesRemain], nSamplesRemain * c_iBytesPerSample);
	}

	return buffCompressed.TellPut();
}

int	CSpeex::Decompress(const byte *pbCompressed, int nCompressedBytes, short *psDecompressed, int nMaxDecompressedBytes) {
	float rgflOutput[c_iRawFrameSize];
	CUtlBuffer buffCompressed(pbCompressed, nCompressedBytes);
	CUtlBuffer buffDecompressed(psDecompressed, nMaxDecompressedBytes);

	while ((buffCompressed.Size() - buffCompressed.TellGet()) >= m_nEncodedBytes && (buffDecompressed.Size() - buffDecompressed.TellPut()) >= c_nRawBytes) {
		speex_bits_read_from(&m_bits, (char *)buffCompressed.PeekGet(), m_nEncodedBytes);

		speex_decode(m_pDecoderState, &m_bits, rgflOutput);

		for (int i = 0;i < c_iRawFrameSize; i++) {
			buffDecompressed.PutShort((short)rgflOutput[i]);
		}

		buffCompressed.SeekGet(CUtlBuffer::SEEK_CURRENT, m_nEncodedBytes);
	}

	return buffDecompressed.TellPut() / c_iBytesPerSample;
}

bool CSpeex::ResetState() {
	speex_encoder_ctl(m_pEncoderState, SPEEX_RESET_STATE, NULL);
	speex_decoder_ctl(m_pDecoderState, SPEEX_RESET_STATE, NULL);

	m_buffEncode.Clear();

	return true;
}