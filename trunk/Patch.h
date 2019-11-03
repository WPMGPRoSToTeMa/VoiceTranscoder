#pragma once

#include "UtilTypes.h"

template <size_t S>
class Patch {
public:
	Patch(void *pAddr, byte *pbPatchedBytes, bool fPatch = true) {
		m_pAddr = pAddr;
		m_rgbPatchedBytes = pbPatchedBytes;

		if (fPatch) {
			RePatch();
		}
	}
	~Patch() {
	}
	void RePatch() {
#ifdef WIN32
		dword dwOldProt;
		VirtualProtect(m_pAddr, sizeof(m_rgbPatchedBytes), PAGE_EXECUTE_READWRITE, &dwOldProt);
#endif
		memcpy(m_pAddr, m_rgbPatchedBytes, sizeof(m_rgbPatchedBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbPatchedBytes), dwOldProt, &dwOldProt);
#endif
	}
	void UnPatch() {
#ifdef WIN32
		dword dwOldProt;
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), PAGE_EXECUTE_READWRITE, &dwOldProt);
#endif
		memcpy(m_pAddr, m_rgbOrigBytes, sizeof(m_rgbOrigBytes));
#ifdef WIN32
		VirtualProtect(m_pAddr, sizeof(m_rgbOrigBytes), dwOldProt, &dwOldProt);
#endif
	}
private:
	void *m_pAddr;
	byte m_rgbOrigBytes[S];
	byte m_rgbPatchedBytes[S];
};

