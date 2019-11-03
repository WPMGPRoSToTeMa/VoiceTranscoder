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
	}
private:
	Patch<sizeof(byte) + sizeof(long)> *m_pPatch;
	byte m_rgbOriginalBytes[sizeof(byte) + sizeof(long)];
	byte m_rgbPatchedBytes[sizeof(byte) + sizeof(long)];
	void *m_pfnAddr;
};