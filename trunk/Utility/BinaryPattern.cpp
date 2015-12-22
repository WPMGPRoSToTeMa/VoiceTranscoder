#include "BinaryPattern.h"
#include <cstdlib>

BinaryPattern::BinaryPattern(const char *patternString) {
	m_byteCount = 0;

	while (*patternString == ' ' || *patternString == '\t') {
		patternString++;
	}

	while (*patternString != '\0') {
		if (*patternString == '?') {
			m_isUnknown[m_byteCount] = true;

			do {
				patternString++;
			} while (*patternString == '?');
		} else {
			m_isUnknown[m_byteCount] = false;
			m_bytes[m_byteCount] = (uint8_t)strtoul(patternString, (char **)&patternString, 16);
		}

		m_byteCount++;

		while (*patternString == ' ' || *patternString == '\t') {
			patternString++;
		}
	}
}


bool BinaryPattern::IsMatch(AnyPointer ptr) const {
	uint8_t *bytePtr = ptr;

	for (size_t i = 0; i < m_byteCount; i++) {
		if (!m_isUnknown[i] && bytePtr[i] != m_bytes[i]) {
			return false;
		}
	}

	return true;
}

size_t BinaryPattern::GetByteCount() const {
	return m_byteCount;
}


BinaryPattern operator""bp(const char *patternString, size_t strLength) {
	return BinaryPattern(patternString);
}