#pragma once

#include "UtilTypes.h"

class CRC32 {
public:
	void Init();
	void Update(const void *pBuf, size_t nLen);
	void Final();
	uint32_t ConvertToUInt32();
protected:
	uint32_t dwChecksum;
};