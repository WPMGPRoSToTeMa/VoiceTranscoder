#include "Timer.h"
#include <Windows.h>

int64_t GetCurrentTimeInMicroSeconds(void) {
#if defined(linux)
	static int64_t llBaseSecs = 0;
	timeval tv;
	gettimeofday(&tv, NULL);
	if (!llBaseSecs) {
		llBaseSecs = (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
	}

	return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec - llBaseSecs;
#else
	static LARGE_INTEGER tickFrequency;
	static LARGE_INTEGER tickCountBase;
	LARGE_INTEGER tickCount;

	if (!tickFrequency.QuadPart) {
		QueryPerformanceFrequency(&tickFrequency);
		QueryPerformanceCounter(&tickCountBase);
	}

	QueryPerformanceCounter(&tickCount);

	return (tickCount.QuadPart - tickCountBase.QuadPart) * 1000000 / tickFrequency.QuadPart;
#endif
}