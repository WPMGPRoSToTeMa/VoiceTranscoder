#include "Mutex.h"
#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

#ifdef _WIN32
Mutex::Mutex() {
	InitializeCriticalSection(&m_critSect);
}

Mutex::~Mutex() {
	DeleteCriticalSection(&m_critSect);
}

void Mutex::Lock() {
	EnterCriticalSection(&m_critSect);
}

void Mutex::Unlock() {
	LeaveCriticalSection(&m_critSect);
}
#elif defined __linux__
Mutex::Mutex() {
	pthread_mutex_init(&m_mutex, nullptr);
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&m_mutex);
}

void Mutex::Lock() {
	pthread_mutex_lock(&m_mutex);
}

void Mutex::Unlock() {
	pthread_mutex_unlock(&m_mutex);
}
#endif