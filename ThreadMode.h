#pragma once

#include "UtilTypes.h"
#include "Main.h"

class VoiceBuf {
public:
	void *m_pBuf;
	uint8_t *m_pOutBuf;
	size_t m_nSize;
	size_t m_nOutBufSize;
	size_t m_nPlayerIndex;
	size_t m_nUserID;
	bool m_fIsNewCodec;
	VoiceBuf *m_pNext;
};

class VoiceBufQueue {
public:
	VoiceBuf *m_pFirst;
	VoiceBuf *m_pLast;

	VoiceBufQueue() {
		m_pFirst = nullptr;
		m_pLast = nullptr;
	}
	bool IsEmpty(void) {
		if (m_pFirst == nullptr) {
			return true;
		}

		return false;
	}
	void Push(VoiceBuf *pVoiceBuf) {
		if (IsEmpty()) {
			m_pFirst = m_pLast = pVoiceBuf;
		} else {
			pVoiceBuf->m_pNext = nullptr;
			m_pLast->m_pNext = pVoiceBuf;
		}
	}
	VoiceBuf *Pop(void) {
		if (m_pFirst == m_pLast) {
			m_pFirst = nullptr;

			return m_pLast;
		}

		VoiceBuf *pVoiceBuf = m_pFirst;
		m_pFirst = m_pFirst->m_pNext;

		return pVoiceBuf;
	}
};

extern void VTC_ThreadInit();
extern void VTC_ThreadDeinit();
extern void VTC_ThreadAddVoicePacket(client_t *pClient, size_t nClientIndex, clientData_t *pClientData, void *pData, size_t nDataSize);
extern void VTC_ThreadHandler(void);
extern void VTC_ThreadVoiceFlusher();