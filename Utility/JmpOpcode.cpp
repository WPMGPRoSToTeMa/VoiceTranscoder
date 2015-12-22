#include "JmpOpcode.h"
#include "MemoryUnlocker.h"

void JmpOpcode::SetHere(AnyPointer opcodePtr, AnyPointer destPtr) {
	MemoryUnlocker unlocker(opcodePtr, SIZE);

	*(uint8_t *)opcodePtr = OPCODE;
	*(ptrdiff_t *)((uintptr_t)opcodePtr + 1) = (ptrdiff_t)destPtr - ((ptrdiff_t)opcodePtr + SIZE);
}