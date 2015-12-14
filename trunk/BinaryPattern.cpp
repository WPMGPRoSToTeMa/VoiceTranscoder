#include "BinaryPattern.h"

bool BinaryPattern::IsEqual(uint32_t dwBytes) {
	uint8_t *pBytes = (uint8_t *)dwBytes;

	for (size_t n = 0; n < m_nBytes; n++) {
		if ((pBytes[n] & m_pMask[n]) != (m_pBytes[n] & m_pMask[n])) {
			return false;
		}
	}

	return true;
}