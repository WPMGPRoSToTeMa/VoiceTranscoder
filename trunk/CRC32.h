#pragma once

#include "UtilTypes.h"

class CRC32 {
public:
	void Init();
	void Update(const void *pBuf, size_t nLen);
	void Final();
	dword ConvertToDWord();
protected:
	dword dwChecksum;
};