#include "Time.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/time.h>
#endif

uint64_t GetCurrentTimeInMicroSeconds(void) {
	static bool isBaseSet = false;
#ifdef __linux__
	static uint64_t baseMicroSeconds = 0;
	timespec timePoint;
	clock_gettime(CLOCK_MONOTONIC, &timePoint);
	if (!isBaseSet) {
		baseMicroSeconds = (uint64_t)timePoint.tv_sec * 1000000 + (uint64_t)timePoint.tv_nsec / 1000;
		isBaseSet = true;
	}

	return (uint64_t)timePoint.tv_sec * 1000000 + (uint64_t)timePoint.tv_nsec / 1000 - baseMicroSeconds;
#elif defined _WIN32
	static LARGE_INTEGER tickFrequency;
	static LARGE_INTEGER tickCountBase;
	LARGE_INTEGER tickCount;

	if (!isBaseSet) {
		QueryPerformanceFrequency(&tickFrequency);
		QueryPerformanceCounter(&tickCountBase);
		isBaseSet = true;
	}

	QueryPerformanceCounter(&tickCount);

	return (tickCount.QuadPart - tickCountBase.QuadPart) * 1000000 / tickFrequency.QuadPart;
#endif
}