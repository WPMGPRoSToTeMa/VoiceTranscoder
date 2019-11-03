#pragma once

#include "Util.h"

class NetMsgBuf : public sizebuf_t {
public:
	sizebuf_t *m_pNetMsg;
	size_t *m_pMsgReadCount;
	bool *m_pMsgBadRead;

	void *Get(void) {
		return &data[*m_pMsgReadCount];
	}
	bool BoundRead(size_t n) {
		if (*m_pMsgReadCount + n > cursize) {
			*m_pMsgBadRead = true;

			return false;
		}

		return true;
	}
	template <typename T>
	T ReadType(void) {
		if (!BoundRead(sizeof(T))) {
			return -1;
		}

		T tResult = *(T *)Get();

		*m_pMsgReadCount += sizeof(T);

		return tResult;
	}
	word ReadWord(void) {
		return ReadType<word>();
	}
	void ReadBuf(void *pMem, size_t nSize) {
		if (!BoundRead(nSize)) {
			return;
		}

		memcpy(pMem, Get(), nSize);

		*m_pMsgReadCount += nSize;
	}
};