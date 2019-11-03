#pragma once

#include "Hook.h"

class Hook_Call : public Hook {
public:
	Hook_Call(void *pfnAddr, void *pfnCallback, bool fHook = true);
	virtual void ReHook() final;
	virtual void UnHook() final;
	virtual ~Hook_Call() final {
		UnHook();
	}
private:
	byte m_rgbOriginalBytes[sizeof(byte) + sizeof(long)];
	byte m_rgbPatchedBytes[sizeof(byte) + sizeof(long)];
	void *m_pfnAddr;
};