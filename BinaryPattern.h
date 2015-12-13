#pragma once

#include "UtilTypes.h"

class BinaryPattern {
public:
	BinaryPattern(const char *pszBytes, const char *pszMask, size_t nBytes) : m_pBytes((const uint8_t *)pszBytes), m_pMask((const uint8_t *)pszMask), m_nBytes(nBytes) {}
	bool IsEqual(uint32_t dwBytes);
private:
	friend class Library;

	const uint8_t *m_pBytes;
	const uint8_t *m_pMask;
	size_t m_nBytes;
};