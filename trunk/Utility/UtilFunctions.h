#pragma once

#include "UtilTypes.h"

template <typename T>
T Min(T a, T b) {
	return (a < b) ? a : b;
}

template <typename T>
T Max(T a, T b) {
	return (a > b) ? a : b;
}

extern void ChangeSamplesVolume(int16_t *samples, size_t sampleCount, double volume);