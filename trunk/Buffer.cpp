#include "Buffer.h"

Buffer::Buffer(void *pBuf, size_t nLen) {
	m_pReadPos = m_pData = (byte *)pBuf;
	m_pEndAllocated = m_pEnd = m_pData + nLen;
}

Buffer::~Buffer() {
}

size_t Buffer::Size() {
	return m_pEnd - m_pData;
}

void Buffer::SeekRead(long lOffset, size_t nSeekType) {
	switch (nSeekType) {
	case SEEK_START:
		m_pReadPos = m_pData + lOffset;
		break;
	case SEEK_CUR:
		m_pReadPos += lOffset;
		break;
	case SEEK_END:
		m_pReadPos = m_pEnd - lOffset;
		break;
	}
}

void Buffer::RewindRead() {
	m_pReadPos = m_pData;
}

size_t Buffer::TellRead() {
	return m_pReadPos - m_pData;
}

void *Buffer::PeekRead() {
	return m_pReadPos;
}

byte Buffer::ReadByte() {
	return ReadType<byte>();
}

word Buffer::ReadWord() {
	return ReadType<word>();
}

dword Buffer::ReadDWord() {
	return ReadType<dword>();
}

qword Buffer::ReadQWord() {
	return ReadType<qword>();
}