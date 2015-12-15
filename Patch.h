#pragma once

#include "UtilTypes.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/mman.h>
#endif

template <size_t S>
class Patch {
public:
	Patch(void *pAddr, uint8_t *pbPatchedBytes, bool fPatch = true) {
		m_pAddr = pAddr;
		memcpy(m_rgbPatchedBytes, pbPatchedBytes, sizeof(m_rgbPatchedBytes));

		if (fPatch) {
			RePatch();
		}
	}
	~Patch() {
	}
	void RePatch() {
#ifdef WIN32
		DWORD oldProtection;
		VirtualProtect(m_pAddr, sizeof(m_rgbPatchedBytes), PAGE_EXECUTE_READWRITE, &oldProtection);
#else
		mprotect((void *)((uintptr_t)m_pAddr & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
		memcpy(m_pAddr, m_rgbPatchedBytes, sizeof(m_rgbPatchedBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbPatchedBytes), oldProtection, &oldProtection);
#else
		mprotect((void *)((uintptr_t)m_pAddr & 0xFFFFF000), 0x1000, PROT_READ | PROT_EXEC);
#endif
	}
	void UnPatch() {
#ifdef WIN32
		DWORD oldProtection;
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), PAGE_EXECUTE_READWRITE, &oldProtection);
#else
		mprotect((void *)((uintptr_t)m_pAddr & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
		memcpy(m_pAddr, m_rgbOrigBytes, sizeof(m_rgbOrigBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), oldProtection, &oldProtection);
#else
		mprotect((void *)((uintptr_t)m_pAddr & 0xFFFFF000), 0x1000, PROT_READ | PROT_EXEC);
#endif
	}
private:
	void *m_pAddr;
	uint8_t m_rgbOrigBytes[S];
	uint8_t m_rgbPatchedBytes[S];
};