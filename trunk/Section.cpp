#include "Section.h"

dword Section::FindString(const char *pszString) {
	dword dwCur = m_dwStart;
	dword dwEnd = m_dwEnd - strlen(pszString) - 1;

	while (dwCur <= dwEnd) {
		if (!strcmp((char *)dwCur, pszString)) {
			return dwCur;
		}

		dwCur++;
	}

	return 0;
}