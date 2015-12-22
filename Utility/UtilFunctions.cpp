#include "UtilFunctions.h"

void ChangeSamplesVolume(int16_t *samples, size_t sampleCount, double volume) {
	if (volume == 1.0) {
		return;
	}

	for (size_t i = 0; i < sampleCount; i++) {
		double sample = samples[i] * volume;

		if (sample > 32767) {
			sample = 32767;
		}
		else if (sample < -32768) {
			sample = -32768;
		}

		double volumeChange = sample / samples[i];
		if (volumeChange < volume) {
			volume = volumeChange;
		}
	}

	if (volume == 1.0) {
		return;
	}

	for (size_t i = 0; i < sampleCount; i++) {
		samples[i] = (int16_t)(samples[i] * volume);
	}
}