#pragma once

#include "Hook.h"

class HookEntry {
private:
	Hook *m_pHook;
	HookEntry *m_pNext;
};

class HookList {
private:
	HookEntry *m_pFirst;
	HookEntry *m_pLast;
};