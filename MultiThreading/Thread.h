#pragma once

#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

class Thread {
public:
	Thread(void(*pfnHandler)(void));
	~Thread();
protected:
#ifdef _WIN32
	static DWORD WINAPI Handler(LPVOID lpParam);

	HANDLE m_hThread;
#elif defined __linux__
	static void *Handler(void *param);

	pthread_t m_thread;
#endif
	void ( *m_pfnHandler)(void);
};