#include "Time.h"
#include <chrono>

uint64_t GetCurrentTimeInMicroSeconds(void) {
	using namespace std::chrono;

	static bool isBaseSet = false;
	static steady_clock::time_point base;

	auto currentTimePoint = steady_clock::now();
	if (!isBaseSet) {
		base = currentTimePoint;
		isBaseSet = true;
	}
	return duration_cast<microseconds>(currentTimePoint - base).count();
}