#pragma once

#include "UtilTypes.h"

class CRC32 {
public:
	CRC32() : checksum(0) {}
	void Init();
	void Update(const void *pBuf, size_t bufLength);
	void Final();
	uint32_t ToUInt32() const;
protected:
	uint32_t checksum;
};