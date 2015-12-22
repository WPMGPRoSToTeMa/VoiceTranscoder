#pragma once

#include "UtilTypes.h"
#include "AnyPointer.h"

class BinaryPattern {
public:
	BinaryPattern(const char *patternString);
	bool IsMatch(AnyPointer ptr) const;
	size_t GetByteCount() const;
private:
	static const size_t MAX_BYTES = 64;

	friend class Module;

	uint8_t m_bytes[MAX_BYTES];
	bool m_isUnknown[MAX_BYTES];
	size_t m_byteCount;
};

extern BinaryPattern operator""bp(const char *patternString, size_t strLength);