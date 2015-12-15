#include "VoiceCodec_Speex.h"

#define ARRAYSIZE(p) (sizeof(p)/sizeof(p[0]))

const size_t Speex::ENCODED_FRAMESIZE[] = {6, 6, 15, 15, 20, 20, 28, 28, 38, 38, 38};

Speex::Speex(size_t quality) {
	m_encoder = speex_encoder_init(&speex_nb_mode);
	m_decoder = speex_decoder_init(&speex_nb_mode);
	speex_bits_init(&m_bits);

	// Transformation
	quality = (quality - 1) * 2;
	m_encodedBytes = ENCODED_FRAMESIZE[quality];
	speex_encoder_ctl(m_encoder, SPEEX_SET_QUALITY, &quality);

	bool postFilter = true; // Set the perceptual enhancement on
	speex_decoder_ctl(m_decoder, SPEEX_SET_ENH, &postFilter);

	size_t sampleRate = SAMPLERATE;
	speex_encoder_ctl(m_encoder, SPEEX_SET_SAMPLING_RATE, &sampleRate);
	speex_decoder_ctl(m_decoder, SPEEX_SET_SAMPLING_RATE, &sampleRate);
}

Speex::~Speex() {
	speex_encoder_destroy(m_encoder);
	speex_encoder_destroy(m_decoder);
	speex_bits_destroy(&m_bits);
}

// We need reset on new client connected?
void Speex::ChangeQuality(size_t quality) {
	// Transformation
	quality = (quality - 1) * 2;
	m_encodedBytes = ENCODED_FRAMESIZE[quality];
	speex_encoder_ctl(m_encoder, SPEEX_SET_QUALITY, &quality);
	speex_encoder_ctl(m_encoder, SPEEX_RESET_STATE, NULL);
}

void Speex::ResetState() {
	speex_encoder_ctl(m_encoder, SPEEX_RESET_STATE, NULL);
	speex_decoder_ctl(m_encoder, SPEEX_RESET_STATE, NULL);
}

size_t Speex::Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) {
	if (rawSamples == NULL) {
		return 0;
	}
	if (rawSampleCount == 0) {
		return 0;
	}
	if (encodedBytes == NULL) {
		return 0;
	}
	if (maxEncodedBytes == 0) {
		return 0;
	}

	size_t encodedBytesCount = 0;
	size_t curRawSample = 0;
	float rawFrame[FRAMESIZE];

	while (rawSampleCount > 0) {
		// TODO: add save for unencoded samples
		if (encodedBytesCount + m_encodedBytes > maxEncodedBytes) {
			return 0;
		}

		for (size_t i = 0; i < ARRAYSIZE(rawFrame); i++) {
			// TODO: add save for unenough samples
			if (rawSampleCount == 0) {
				rawFrame[i] = 0;
			} else {
				rawFrame[i] = rawSamples[curRawSample];

				curRawSample++;
				rawSampleCount--;
			}
		}

		speex_bits_reset(&m_bits);
		speex_encode(m_encoder, rawFrame, &m_bits);
		encodedBytesCount += speex_bits_write(&m_bits, (char *)&encodedBytes[encodedBytesCount], m_encodedBytes);
	}

	return encodedBytesCount;
}

size_t Speex::Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) {
	if (encodedBytes == NULL) {
		return 0;
	}
	if (encodedBytesCount == 0) {
		return 0;
	}
	// TODO: add kick message for this
	if (encodedBytesCount % m_encodedBytes != 0) {
		return 0;
	}
	if (rawSamples == NULL) {
		return 0;
	}
	if (maxRawSamples == 0) {
		return 0;
	}

	size_t decodedRawSamples = 0;
	size_t curEncodedBytePos = 0;
	float rawFrame[FRAMESIZE];

	while (encodedBytesCount > 0) {
		// TODO: add save?
		if (decodedRawSamples + ARRAYSIZE(rawFrame)*sizeof(int16_t) > maxRawSamples) {
			return 0;
		}

		// Need speex_bits_reset?
		speex_bits_read_from(&m_bits, (char *)&encodedBytes[curEncodedBytePos], m_encodedBytes);
		speex_decode(m_decoder, &m_bits, rawFrame);

		for (size_t i = 0; i < ARRAYSIZE(rawFrame); i++) {
			rawSamples[decodedRawSamples++] = (int16_t)rawFrame[i];
		}

		encodedBytesCount -= m_encodedBytes;
		curEncodedBytePos += m_encodedBytes;
	}

	return decodedRawSamples;
}