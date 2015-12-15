#include "Timer.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/time.h>
#endif

int64_t GetCurrentTimeInMicroSeconds(void) {
#if defined(linux)
	static int64_t baseMicroSeconds = 0;
	timeval tv;
	gettimeofday(&tv, NULL);
	if (!baseMicroSeconds) {
		baseMicroSeconds = (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
	}

	return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec - baseMicroSeconds;
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