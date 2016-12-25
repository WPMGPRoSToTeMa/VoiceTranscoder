#include "VoiceCodec_SILK.h"
#include <cstdlib>
#include <cstring>

VoiceCodec_SILK::VoiceCodec_SILK(size_t quality) {
	int size;
	SKP_Silk_SDK_Get_Encoder_Size(&size);
	m_encState = malloc(size);
	SKP_Silk_SDK_InitEncoder(m_encState, &m_encControl);
	m_sampleRate = (quality == 10 ? 16000 : 8000);
	m_minSamples = m_sampleRate * FRAMELENGTHMS / 1000;

	m_encControl.API_sampleRate = m_sampleRate;
	m_encControl.maxInternalSampleRate = m_sampleRate;
	m_encControl.packetSize = m_minSamples;
	// TODO: add cvar for SILK bitrate
	m_encControl.bitRate = 25000; // or 12500? Because we divide samplerate by 2
	m_encControl.packetLossPercentage = 0;
	m_encControl.complexity = 2; // or use 1 for average CPU usage?
	m_encControl.useInBandFEC = 0;
	// TODO: need investigation about it (voice_silk.so and steamclient.dll)
	m_encControl.useDTX = 0;

	SKP_Silk_SDK_Get_Decoder_Size(&size);
	m_decState = malloc(size);
	SKP_Silk_SDK_InitDecoder(m_decState);

	m_decControl.API_sampleRate = m_sampleRate;
}

VoiceCodec_SILK::~VoiceCodec_SILK() {
	free(m_encState);
	free(m_decState);
}

void VoiceCodec_SILK::ChangeQuality(size_t quality) {}

void VoiceCodec_SILK::ResetState() {
	if (m_encState) {
		SKP_Silk_SDK_InitEncoder(m_encState, &m_encControl);

		m_encControl.API_sampleRate = m_sampleRate;
		m_encControl.maxInternalSampleRate = m_sampleRate;
		m_encControl.packetSize = m_minSamples;
		m_encControl.bitRate = 25000; // or 12500? Because we divide samplerate by 2
		m_encControl.packetLossPercentage = 0;
		m_encControl.complexity = 2; // or use 1 for average CPU usage?
		m_encControl.useInBandFEC = 0;
		m_encControl.useDTX = 0;
	}
	if (m_decState) {
		SKP_Silk_SDK_InitDecoder(m_decState);
	}
}

// TODO: maxEncodedBytes change to ptr which save encodedBytesCount
size_t VoiceCodec_SILK::Encode(const int16_t *rawSamples, size_t rawSampleCount, uint8_t *encodedBytes, size_t maxEncodedBytes) {
	if (rawSamples == nullptr) {
		return 0;
	}
	if (rawSampleCount == 0) {
		return 0;
	}
	// TODO
	if (rawSampleCount % m_minSamples != 0) {
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

	while (rawSampleCount != 0) {
		int16_t bytesOut = int16_t(maxEncodedBytes - encodedBytesCount);

		if (bytesOut <= sizeof(int16_t)) {
			return 0;
		}

		bytesOut -= sizeof(int16_t);
		encodedBytesCount += sizeof(int16_t);

		if (SKP_Silk_SDK_Encode(m_encState, &m_encControl, &rawSamples[curRawSample], m_minSamples, &encodedBytes[encodedBytesCount], &bytesOut) != SKP_SILK_NO_ERROR) {
			return 0;
		}
		if (bytesOut <= 0) {
			return 0;
		}

		encodedBytesCount -= sizeof(int16_t);
		*(int16_t *)&encodedBytes[encodedBytesCount] = bytesOut;
		encodedBytesCount += sizeof(int16_t);

		rawSampleCount -= m_minSamples;
		curRawSample += m_minSamples;
		encodedBytesCount += bytesOut;
	}

	return encodedBytesCount;
}

// TODO: add arg for error handling
size_t VoiceCodec_SILK::Decode(const uint8_t *encodedBytes, size_t encodedBytesCount, int16_t *rawSamples, size_t maxRawSamples) {
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

	while (encodedBytesCount >= sizeof(uint16_t)) {
		// TODO
		if (decodedRawSamples + m_minSamples > maxRawSamples) {
			return 0;
		}

		uint16_t payloadSize = *(uint16_t *)&encodedBytes[curEncodedBytePos];
		
		curEncodedBytePos += sizeof(uint16_t);
		encodedBytesCount -= sizeof(uint16_t);

		if (payloadSize == 0) {
			memset(&rawSamples[decodedRawSamples], 0, m_minSamples * sizeof(uint16_t));
			decodedRawSamples += m_minSamples;

			continue;
		}
		if (payloadSize == 0xFFFF) {
			ResetState();

			return decodedRawSamples;
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