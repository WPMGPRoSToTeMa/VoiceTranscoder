#include "Section.h"
#include <cstring>

AnyPointer Section::FindString(const char *str) const {
	uintptr_t ptr = m_startPtr;
	uintptr_t endPtr = m_endPtr - strlen(str) - sizeof(char);

	while (ptr <= endPtr) {
		if (!strcmp((char *)ptr, str)) {
			return ptr;
		}

		ptr++;
	}

	return nullptr;
}