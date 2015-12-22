#include "FunctionHook_Beginning.h"
#include <cstring>
#include "MemoryUnlocker.h"

FunctionHook_Beginning::FunctionHook_Beginning() {
	m_functionPtr = (uintptr_t)nullptr;
}

FunctionHook_Beginning::FunctionHook_Beginning(AnyPointer functionPtr, AnyPointer callbackPtr, bool doHook/* = true*/) {
	Create(functionPtr, callbackPtr, doHook);
}

FunctionHook_Beginning::~FunctionHook_Beginning() {
	if (IsCreated()) {
		Remove();
	}
}

void FunctionHook_Beginning::Create(AnyPointer functionPtr, AnyPointer callbackPtr, bool doHook/* = true*/) {
	m_functionPtr = functionPtr;

	memcpy(m_originalBytes, (AnyPointer)m_functionPtr, sizeof(m_originalBytes));
	JmpOpcode::SetHere(m_functionPtr, callbackPtr);
	memcpy(m_patchedBytes, (AnyPointer)m_functionPtr, sizeof(m_patchedBytes));

	if (!doHook) {
		UnHook();
	}
}

void FunctionHook_Beginning::Remove() {
	UnHook();

	m_functionPtr = (uintptr_t)nullptr;
}

bool FunctionHook_Beginning::IsCreated() {
	return m_functionPtr != (uintptr_t)nullptr;
}

void FunctionHook_Beginning::ReHook() {
	MemoryUnlocker unlocker(m_functionPtr, sizeof(m_patchedBytes));
	memcpy((AnyPointer)m_functionPtr, m_patchedBytes, sizeof(m_patchedBytes));
}

void FunctionHook_Beginning::UnHook() {
	MemoryUnlocker unlocker(m_functionPtr, sizeof(m_originalBytes));
	memcpy((AnyPointer)m_functionPtr, m_originalBytes, sizeof(m_originalBytes));
}