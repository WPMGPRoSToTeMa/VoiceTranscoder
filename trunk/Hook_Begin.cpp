#include "Hook_Begin.h"
#include <string.h>

Hook_Begin::Hook_Begin(void *pfnAddr, void *pfnCallback, bool fHook /*= true*/) {
	m_pfnAddr = pfnAddr;

	// Patched bytes
	byte rgbPatchedBytes[sizeof(byte) + sizeof(long)];
	rgbPatchedBytes[0] = 0xE9; // jmp
	*(long *)&rgbPatchedBytes[sizeof(byte)] = (long)pfnCallback - ((long)pfnAddr + sizeof(byte) + sizeof(long));
	m_pPatch = new Patch<sizeof(byte) + sizeof(long)>(m_pfnAddr, rgbPatchedBytes, fHook);
}

void Hook_Begin::ReHook() {
	m_pPatch->RePatch();
}

void Hook_Begin::UnHook() {
	m_pPatch->UnPatch();
}