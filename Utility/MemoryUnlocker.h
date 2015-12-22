#pragma once

#include "AnyPointer.h"
#ifdef _WIN32
	#include <Windows.h>
#endif

class MemoryUnlocker {
	uintptr_t m_ptr;
	size_t m_size;

#ifdef _WIN32
	DWORD m_oldProtection;
#endif
public:
	MemoryUnlocker(AnyPointer ptr, size_t size);
	~MemoryUnlocker();
};