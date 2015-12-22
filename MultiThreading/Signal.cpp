#include "Signal.h"

#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

#ifdef _WIN32
Signal::Signal() {
	m_hEvent = CreateEvent(nullptr, FALSE/*TODO: or TRUE?*/, FALSE, nullptr);
}

Signal::~Signal() {
	CloseHandle(m_hEvent);
}

void Signal::Raise() {
	SetEvent(m_hEvent);
}

void Signal::Wait(Mutex *pMutex) {
	ResetEvent(m_hEvent);
	pMutex->Unlock();
	WaitForSingleObject(m_hEvent, INFINITE);
	pMutex->Lock();
}
#elif defined __linux__
Signal::Signal() {
	pthread_cond_init(&m_cond, nullptr);
}

Signal::~Signal() {
	pthread_cond_destroy(&m_cond);
}

void Signal::Raise() {
	pthread_cond_signal(&m_cond);
}

void Signal::Wait(Mutex *pMutex) {
	pthread_cond_wait(&m_cond, &pMutex->m_mutex);
}
#endif