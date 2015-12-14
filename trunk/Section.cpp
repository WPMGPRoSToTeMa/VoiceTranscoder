#include "Section.h"

uint32_t Section::FindString(const char *pszString) {
	uint32_t dwCur = m_dwStart;
	uint32_t dwEnd = m_dwEnd - strlen(pszString) - 1;

	while (dwCur <= dwEnd) {
		if (!strcmp((char *)dwCur, pszString)) {
			return dwCur;
		}

		dwCur++;
	}

	return 0;
}