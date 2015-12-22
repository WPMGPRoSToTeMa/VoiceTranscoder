#pragma once

#include "UtilTypes.h"

#ifndef SEEK_SET
#define SEEK_SET	0	/* seek to an absolute position */
#define SEEK_CUR	1			/* seek relative to current position */
#define SEEK_END	2			/* seek relative to end of file */
#endif

#define SEEK_START	SEEK_SET

class Buffer {
public:
	Buffer(void *pBuf, size_t nLen);
	~Buffer();

	size_t Size() const;

	void SeekRead(long lOffset, size_t nSeekType);
	void RewindRead();
	size_t TellRead() const;
	void *PeekRead() const;

	template <typename T>
	T ReadType();
	uint8_t ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();
	uint8_t PeekUInt8() const;

	void SkipBytes(size_t bytesCount);
protected:
	uint8_t *m_pData;
	uint8_t *m_pEnd;
	uint8_t *m_pEndAllocated;
	uint8_t *m_pReadPos;
};

template <typename T>
T Buffer::ReadType() {
	T ret = *(T *)m_pReadPos;

	m_pReadPos += sizeof(T);

	return ret;
}