#pragma once

#include "UtilTypes.h"
#include <string.h>

class Section {
public:
	Section() {}
	Section(uint32_t dwStart, uint32_t dwSize) : m_dwStart(dwStart), m_dwEnd(dwStart + dwSize) {}
	void Set(uint32_t dwStart, uint32_t dwEnd) {
		m_dwStart = dwStart;
		m_dwEnd = dwEnd;
	}
	uint32_t FindString(const char *pszString);
	Section *Next() { return m_pNext; }
	uint32_t Start() { return m_dwStart; }
	uint32_t End() { return m_dwEnd; }
	void SetNext(Section *pSection) { m_pNext = pSection; }
private:
	uint32_t m_dwStart;
	uint32_t m_dwEnd;
	Section *m_pNext;
};