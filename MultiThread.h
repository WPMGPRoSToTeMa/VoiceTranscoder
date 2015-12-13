#pragma once

#include <Windows.h>

class Mutex;

class Thread {
public:
	Thread(void (* pfnHandler)(void));
	~Thread();
protected:
	static DWORD WINAPI Handler(LPVOID lpParam);

	HANDLE m_hThread;
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
	pthread_cond_t m_Cond;
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
	CRITICAL_SECTION m_CritSect;
#else
	pthread_mutex_t m_Mutex;
#endif
};