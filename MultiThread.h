#pragma once

#ifdef WIN32
	#include <Windows.h>
#else
	#include <pthread.h>
#endif

class Mutex;

class Thread {
public:
	Thread(void (* pfnHandler)(void));
	~Thread();
protected:
#ifdef WIN32
	static DWORD WINAPI Handler(LPVOID lpParam);

	HANDLE m_hThread;
#else
	static void *Handler(void *param);

	pthread_t m_thread;
#endif
	void (* m_pfnHandler)(void);
};

class Signal {
public:
	Signal();
	~Signal();
	void Raise();
	void Wait(Mutex *pMutex);
protected:
#ifdef WIN32
	HANDLE m_hEvent;
#else
	pthread_cond_t m_cond;
#endif
};

class Mutex {
public:
	Mutex();
	~Mutex();
	void Lock();
	void Unlock();

	friend Signal;
protected:
#ifdef WIN32
	CRITICAL_SECTION m_critSect;
#else
	pthread_mutex_t m_mutex;
#endif
};