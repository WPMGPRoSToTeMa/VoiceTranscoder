#pragma once

#include "UtilTypes.h"

class Template {
public:
	Template(const char *pszBytes, const char *pszMask, size_t nBytes) : m_pBytes((const byte *)pszBytes), m_pMask((const byte *)pszMask), m_nBytes(nBytes) {}
	bool IsEqual(dword dwBytes);
private:
	friend class Library;

	const byte *m_pBytes;
	const byte *m_pMask;
	size_t m_nBytes;
};

