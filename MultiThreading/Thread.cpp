#include "Thread.h"

#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <pthread.h>
#endif

#ifdef _WIN32
DWORD WINAPI Thread::Handler(LPVOID lpParam) {
	(*((Thread *)lpParam)->m_pfnHandler)(); // or use handler immediately?

	return 0;
}

Thread::Thread(void(*pfnHandler)(void)) {
	m_pfnHandler = pfnHandler;
	m_hThread = CreateThread(nullptr, 0, &Thread::Handler, this, 0, nullptr);
}

Thread::~Thread() {
	CloseHandle(m_hThread);
}
#elif defined __linux__
void *Thread::Handler(void *param) {
	(*((Thread *)param)->m_pfnHandler)();

	return nullptr;
}

Thread::Thread(void(*pfnHandler)(void)) {
	m_pfnHandler = pfnHandler;
	pthread_create(&m_thread, nullptr, &Thread::Handler, this);
}

Thread::~Thread() {
	// TODO: add thread terminate
}
#endif