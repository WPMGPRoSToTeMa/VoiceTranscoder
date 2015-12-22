#pragma once

#include "FunctionHook.h"
#include "JmpOpcode.h"
#include "AnyPointer.h"

class FunctionHook_Beginning : public FunctionHook {
public:
	FunctionHook_Beginning();
	FunctionHook_Beginning(AnyPointer functionPtr, AnyPointer callbackPtr, bool doHook = true);
	void Create(AnyPointer functionPtr, AnyPointer callbackPtr, bool doHook = true);
	virtual void Remove() override final;
	virtual bool IsCreated() override final;
	virtual void ReHook() override final;
	virtual void UnHook() override final;
	virtual ~FunctionHook_Beginning() override final;
private:
	uint8_t m_originalBytes[JmpOpcode::SIZE];
	uint8_t m_patchedBytes[JmpOpcode::SIZE];
	uintptr_t m_functionPtr;
};