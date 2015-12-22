#pragma once

#include "Signal.h"
#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

class Signal;

class Mutex {
public:
	Mutex();
	~Mutex();
	void Lock();
	void Unlock();

	friend Signal;
protected:
#ifdef _WIN32
	CRITICAL_SECTION m_critSect;
#elif defined __linux__
	pthread_mutex_t m_mutex;
#endif
};