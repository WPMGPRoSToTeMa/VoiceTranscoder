#include "Buffer.h"

Buffer::Buffer(void *pBuf, size_t nLen) {
	m_pReadPos = m_pData = (uint8_t *)pBuf;
	m_pEndAllocated = m_pEnd = m_pData + nLen;
}

Buffer::~Buffer() {
}

size_t Buffer::Size() const {
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

size_t Buffer::TellRead() const {
	return m_pReadPos - m_pData;
}

void *Buffer::PeekRead() const {
	return m_pReadPos;
}

uint8_t Buffer::ReadUInt8() {
	return ReadType<uint8_t>();
}

uint16_t Buffer::ReadUInt16() {
	return ReadType<uint16_t>();
}

uint32_t Buffer::ReadUInt32() {
	return ReadType<uint32_t>();
}

uint64_t Buffer::ReadUInt64() {
	return ReadType<uint64_t>();
}

uint8_t Buffer::PeekUInt8() const {
	return *(uint8_t *)m_pReadPos;
}

void Buffer::SkipBytes(size_t bytesCount) {
	m_pReadPos += bytesCount;
}