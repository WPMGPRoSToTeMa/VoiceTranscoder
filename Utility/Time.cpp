#include "Time.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/time.h>
#endif

uint64_t GetCurrentTimeInMicroSeconds(void) {
#ifdef __linux__
	static uint64_t baseMicroSeconds = 0;
	timeval tv;
	gettimeofday(&tv, NULL);
	if (!baseMicroSeconds) {
		baseMicroSeconds = (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
	}

	return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec - baseMicroSeconds;
#elif defined _WIN32
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