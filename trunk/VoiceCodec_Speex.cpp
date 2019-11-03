#include "VoiceCodec_Speex.h"

const size_t Speex::s_rgEncodedFrameSize[] = {6, 6, 15, 15, 20, 20, 28, 28, 38, 38, 38};

Speex::Speex(size_t nQuality, size_t nSampleRate) {
	m_pEncoder = speex_encoder_init(&speex_nb_mode);
	m_pDecoder = speex_decoder_init(&speex_nb_mode);
	speex_bits_init(&m_bits);

	// Transformation
	nQuality = (nQuality - 1) * 2;
	m_nEncodedBytes = s_rgEncodedFrameSize[nQuality];
	speex_encoder_ctl(m_pEncoder, SPEEX_SET_QUALITY, &nQuality);

	bool fPostFilter = true; // Set the perceptual enhancement on
	speex_decoder_ctl(m_pDecoder, SPEEX_SET_ENH, &fPostFilter);

	nSampleRate = SAMPLERATE;
	speex_encoder_ctl(m_pEncoder, SPEEX_SET_SAMPLING_RATE, &nSampleRate);
	speex_decoder_ctl(m_pDecoder, SPEEX_SET_SAMPLING_RATE, &nSampleRate);
}

Speex::~Speex() {
	speex_encoder_destroy(m_pEncoder);
	speex_encoder_destroy(m_pDecoder);
	speex_bits_destroy(&m_bits);
}

void Speex::ChangeQuality(size_t nQuality, size_t nSampleRate) {
	// Transformation
	nQuality = (nQuality - 1) * 2;
	m_nEncodedBytes = s_rgEncodedFrameSize[nQuality];
	speex_encoder_ctl(m_pEncoder, SPEEX_SET_QUALITY, &nQuality);
	speex_encoder_ctl(m_pEncoder, SPEEX_RESET_STATE, NULL);
}

