#include "MemoryUnlocker.h"
#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <sys/mman.h>

	#ifndef PAGESIZE
		#define PAGESIZE 0x1000
	#endif
#endif

MemoryUnlocker::MemoryUnlocker(AnyPointer _ptr, size_t size) {
	uintptr_t ptr = _ptr;
	m_ptr = ptr;
	m_size = size;
#ifdef _WIN32
	VirtualProtect((AnyPointer)m_ptr, m_size, PAGE_EXECUTE_READWRITE, &m_oldProtection);
#elif defined __linux__
	m_ptr &= ~(PAGESIZE - 1);
	m_size += ptr - m_ptr;

	mprotect((AnyPointer)m_ptr, m_size, PROT_EXEC | PROT_READ | PROT_WRITE);
#endif
}

MemoryUnlocker::~MemoryUnlocker() {
#ifdef _WIN32
	VirtualProtect((AnyPointer)m_ptr, m_size, m_oldProtection, &m_oldProtection);
#elif defined __linux__
	mprotect((AnyPointer)m_ptr, m_size, PROT_EXEC | PROT_READ); // TODO: find old protection
#endif
}