#pragma once

#include "Mutex.h"
#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

class Mutex;

class Signal {
public:
	Signal();
	~Signal();
	void Raise();
	void Wait(Mutex *pMutex);
protected:
#ifdef _WIN32
	HANDLE m_hEvent;
#elif defined __linux__
	pthread_cond_t m_cond;
#endif
};