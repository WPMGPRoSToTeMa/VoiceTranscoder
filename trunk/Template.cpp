#include "Template.h"

bool Template::IsEqual(dword dwBytes) {
	byte *pBytes = (byte *)dwBytes;

	for (size_t n = 0; n < m_nBytes; n++) {
		if ((pBytes[n] & m_pMask[n]) != (m_pBytes[n] & m_pMask[n])) {
			return false;
		}
	}

	return true;
}