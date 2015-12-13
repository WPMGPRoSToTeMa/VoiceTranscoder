#include "VoiceCodec_SILK.h"
#include <stdlib.h>

SILK::SILK(size_t quality) {
	int size;
	SKP_Silk_SDK_Get_Encoder_Size(&size);
	m_encState = malloc(size);
	SKP_Silk_SDK_InitEncoder(m_encState, &m_encControl);

	m_encControl.API_sampleRate = 8000;
	m_encControl.maxInternalSampleRate = 8000;
	m_encControl.packetSize = MIN_SAMPLES;
	m_encControl.bitRate = 25000; // or 12500? Because we divide samplerate by 2
	m_encControl.packetLossPercentage = 0;
	m_encControl.complexity = 2; // or use 1 for average CPU usage?
	m_encControl.useInBandFEC = 0;
	m_encControl.useDTX = 1;

	SKP_Silk_SDK_Get_Decoder_Size(&size);
	m_decState = malloc(size);
	SKP_Silk_SDK_InitDecoder(m_decState);

	m_decControl.API_sampleRate = 8000;
}

SILK::~SILK() {
	free(m_encState);
	free(m_decState);
}

void SILK::ChangeQuality(size_t quality) {}

void SILK::ResetState() {
	if (m_encState) {
		SKP_Silk_SDK_InitEncoder(m_encState, &m_encControl);

		m_encControl.API_sampleRate = 8000;
		m_encControl.maxInternalSampleRate = 8000;
		m_encControl.packetSize = MIN_SAMPLES;
		m_encControl.bitRate = 25000; // or 12500? Because we divide samplerate by 2
		m_encControl.packetLossPercentage = 0;
		m_encControl.complexity = 2; // or use 1 for average CPU usage?
		m_encControl.useInBandFEC = 0;
		m_encControl.useDTX = 1;
	}
	if (m_decState) {
		SKP_Silk_SDK_InitDecoder(m_decState);
	}
}

// TODO: maxEncodedBytes change to ptr which save encodedBytesCount
size_t SILK::Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) {
	if (rawSamples == nullptr) {
		return 0;
	}
	if (rawSampleCount == 0) {
		return 0;
	}
	if (encodedBytes == nullptr) {
		return 0;
	}
	if (maxEncodedBytes == 0) {
		return 0;
	}

	size_t encodedBytesCount = 0;
	size_t curRawSample = 0;

	while (rawSampleCount >= MIN_SAMPLES) {
		int16_t bytesOut = maxEncodedBytes - encodedBytesCount;

		if (bytesOut <= sizeof(int16_t)) {
			return 0;
		}

		bytesOut -= sizeof(int16_t);
		encodedBytesCount += sizeof(int16_t);

		// TODO
		//for (size_t i = rawSampleCount; i < MIN_SAMPLES; i++) {
		//
		//}

		if (SKP_Silk_SDK_Encode(m_encState, &m_encControl, &rawSamples[curRawSample], MIN_SAMPLES, &encodedBytes[encodedBytesCount], &bytesOut) != SKP_SILK_NO_ERROR) {
			return 0;
		}
		if (bytesOut <= 0) {
			return 0;
		}

		encodedBytesCount -= sizeof(int16_t);
		*(int16_t *)&encodedBytes[encodedBytesCount] = bytesOut;
		encodedBytesCount += sizeof(int16_t);

		rawSampleCount -= MIN_SAMPLES;
		curRawSample += MIN_SAMPLES;
		encodedBytesCount += bytesOut;
	}

	return encodedBytesCount;
}

size_t SILK::Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) {
	if (encodedBytes == nullptr) {
		return 0;
	}
	if (encodedBytesCount == 0) {
		return 0;
	}
	if (rawSamples == nullptr) {
		return 0;
	}
	if (maxRawSamples == 0) {
		return 0;
	}

	size_t curEncodedBytePos = 0;
	size_t decodedRawSamples = 0;

	while (encodedBytesCount > sizeof(int16_t)) {
		if (decodedRawSamples + MIN_SAMPLES > maxRawSamples) {
			return 0;
		}

		int16_t payloadSize = *(int16_t *)&encodedBytes[curEncodedBytePos];
		
		curEncodedBytePos += sizeof(int16_t);
		encodedBytesCount -= sizeof(int16_t);

		if (payloadSize <= 0) {
			return 0;
		}
		if (payloadSize > encodedBytesCount) {
			return 0;
		}

		int16_t decodedSamples;
		if (SKP_Silk_SDK_Decode(m_decState, &m_decControl, 0, &encodedBytes[curEncodedBytePos], payloadSize, &rawSamples[decodedRawSamples], &decodedSamples) != SKP_SILK_NO_ERROR) {
			return 0;
		}
		if (m_decControl.moreInternalDecoderFrames != 0) {
			return 0;
		}

		decodedRawSamples += decodedSamples;
		curEncodedBytePos += payloadSize;
		encodedBytesCount -= payloadSize;
	}

	return decodedRawSamples;
}