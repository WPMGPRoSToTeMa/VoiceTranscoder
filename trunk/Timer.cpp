#include "Timer.h"
#include <Windows.h>

longlong GetCurTime(void) {
#if defined(linux)
	static longlong llBaseSecs = 0;
	timeval tv;
	gettimeofday(&tv, NULL);
	if (!llBaseSecs) {
		llBaseSecs = (longlong)tv.tv_sec * 1000000 + (longlong)tv.tv_usec;
	}

	return (longlong)tv.tv_sec * 1000000 + (longlong)tv.tv_usec - llBaseSecs;
#else
	static LARGE_INTEGER tickFrequency;
	static LARGE_INTEGER tickCountBase;
	LARGE_INTEGER tickCount;

	if (!tickFrequency.QuadPart) {
		QueryPerformanceFrequency(&tickFrequency);
		QueryPerformanceCounter(&tickCountBase);
	}

	QueryPerformanceCounter(&tickCount);

	return (tickCount.QuadPart - tickCountBase.QuadPart) / tickFrequency.QuadPart * 1000000;
#endif
}