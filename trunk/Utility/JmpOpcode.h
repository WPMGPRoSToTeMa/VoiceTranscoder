#pragma once

#include "AnyPointer.h"
#include "UtilTypes.h"

namespace JmpOpcode {
	const size_t SIZE = sizeof(uint8_t) + sizeof(ptrdiff_t);
	const size_t OPCODE = 0xE9;

	extern void SetHere(AnyPointer opcodePtr, AnyPointer destPtr);
}