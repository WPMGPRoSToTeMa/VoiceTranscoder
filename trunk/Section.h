#pragma once

#include "UtilTypes.h"
#include <string.h>

class Section {
public:
	Section() {}
	Section(dword dwStart, dword dwSize) : m_dwStart(dwStart), m_dwEnd(dwStart + dwSize) {}
	void Set(dword dwStart, dword dwEnd) {
		m_dwStart = dwStart;
		m_dwEnd = dwEnd;
	}
	dword FindString(const char *pszString);
	Section *Next() { return m_pNext; }
	dword Start() { return m_dwStart; }
	dword End() { return m_dwEnd; }
	void SetNext(Section *pSection) { m_pNext = pSection; }
private:
	dword m_dwStart;
	dword m_dwEnd;
	Section *m_pNext;
};