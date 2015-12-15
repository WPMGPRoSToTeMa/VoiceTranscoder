#include "MultiThread.h"

#ifdef WIN32
// THREAD
DWORD WINAPI Thread::Handler(LPVOID lpParam) {
	(*((Thread *)lpParam)->m_pfnHandler)(); // or use handler immediately?

	return 0;
}

Thread::Thread(void (* pfnHandler)(void)) {
	m_pfnHandler = pfnHandler;
	m_hThread = CreateThread(nullptr, 0, &Thread::Handler, this, 0, nullptr);
}

Thread::~Thread() {
	CloseHandle(m_hThread);
}

// SIGNAL
Signal::Signal() {
	m_hEvent = CreateEvent(nullptr, FALSE /* Or TRUE ? */, FALSE, nullptr);
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
#else
// THREAD
void *Thread::Handler(void *param) {
	(*((Thread *)param)->m_pfnHandler)();

	return nullptr;
}

Thread::Thread(void (* pfnHandler)(void)) {
	m_pfnHandler = pfnHandler;
	pthread_create(&m_thread, nullptr, &Thread::Handler, this);
}

Thread::~Thread() {
	// TODO: add thread terminate
}

// SIGNAL
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

// MUTEX
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