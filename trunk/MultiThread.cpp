#include "MultiThread.h"

#ifdef WIN32
// THREAD
DWORD WINAPI Thread::Handler(LPVOID lpParam) {
	(*((Thread *)lpParam)->m_pfnHandler)();

	return 0;
}

Thread::Thread(void (* pfnHandler)(void)) {
	m_pfnHandler = pfnHandler;
	m_hThread = CreateThread(NULL, 0, &Thread::Handler, this, 0, NULL);
}



Thread::~Thread() {
	CloseHandle(m_hThread);
}

// SIGNAL
Signal::Signal() {
	m_hEvent = CreateEvent(NULL, FALSE /* Or TRUE ? */, FALSE, NULL);
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

// MUTEX
Mutex::Mutex() {
	InitializeCriticalSection(&m_CritSect);
}

Mutex::~Mutex() {
	DeleteCriticalSection(&m_CritSect);
}

void Mutex::Lock() {
	EnterCriticalSection(&m_CritSect);
}

void Mutex::Unlock() {
	LeaveCriticalSection(&m_CritSect);
}
#else
// SIGNAL
Signal::Signal() {
	pthread_cond_init(&m_Cond, NULL);
}

Signal::~Signal() {
	pthread_cond_destroy(&m_Cond);
}

void Signal::Raise() {
	pthread_cond_signal(&m_Cond);
}

void Signal::Wait(Mutex *pMutex) {
	pthread_cond_wait(&m_Cond, &pMutex->m_Mutex);
}

// MUTEX
Mutex::Mutex() {
	pthread_mutex_init(&m_Mutex, NULL);
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&m_Mutex);
}

void Mutex::Lock() {
	pthread_mutex_lock(&m_Mutex);
}

void Mutex::Unlock() {
	pthread_mutex_unlock(&m_Mutex);
}
#endif