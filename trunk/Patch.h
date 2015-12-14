#pragma once

#include "UtilTypes.h"
#include <Windows.h>

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
#endif
		memcpy(m_pAddr, m_rgbPatchedBytes, sizeof(m_rgbPatchedBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbPatchedBytes), oldProtection, &oldProtection);
#endif
	}
	void UnPatch() {
#ifdef WIN32
		DWORD oldProtection;
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), PAGE_EXECUTE_READWRITE, &oldProtection);
#endif
		memcpy(m_pAddr, m_rgbOrigBytes, sizeof(m_rgbOrigBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), oldProtection, &oldProtection);
#endif
	}
private:
	void *m_pAddr;
	uint8_t m_rgbOrigBytes[S];
	uint8_t m_rgbPatchedBytes[S];
};

