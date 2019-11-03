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

	size_t Size();

	void SeekRead(long lOffset, size_t nSeekType);
	void RewindRead();
	size_t TellRead();
	void *PeekRead();

	template <typename T>
	T ReadType();
	byte ReadByte();
	word ReadWord();
	dword ReadDWord();
	qword ReadQWord();
protected:
	byte *m_pData;
	byte *m_pEnd;
	byte *m_pEndAllocated;
	byte *m_pReadPos;
};

template <typename T>
T Buffer::ReadType() {
	T ret = *(T *)m_pReadPos;

	m_pReadPos += sizeof(T);

	return ret;
}