#pragma once

#include "Hook.h"
#include "Patch.h"

class Hook_Begin : public Hook {
public:
	Hook_Begin(void *pfnAddr, void *pfnCallback, bool fHook = true);
	virtual void ReHook() final;
	virtual void UnHook() final;
	virtual ~Hook_Begin() final {
		UnHook();

		delete m_pPatch;
	}
private:
	Patch<sizeof(uint8_t) + sizeof(long)> *m_pPatch;
	uint8_t m_rgbOriginalBytes[sizeof(uint8_t) + sizeof(long)];
	uint8_t m_rgbPatchedBytes[sizeof(uint8_t) + sizeof(long)];
	void *m_pfnAddr;
};