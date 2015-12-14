#include <extdll.h>
#include <meta_api.h>
#include "ThreadMode.h"
#include "MultiThread.h"
#include "Util.h"
#include "Main.h"

VoiceBufQueue *g_pVoiceRawBufQueue, *g_pVoiceTranscodedBufQueue;
Mutex *g_pRawQueueMutex, *g_pTranscodedQueueMutex;
Signal *g_pHaveRawBufSignal;
Thread *g_pThread;

void VTC_ThreadInit() {
	// Create
	g_pRawQueueMutex = new Mutex;
	g_pTranscodedQueueMutex = new Mutex;
	g_pHaveRawBufSignal = new Signal;
	g_pVoiceRawBufQueue = new VoiceBufQueue;
	g_pVoiceTranscodedBufQueue = new VoiceBufQueue;
	g_pThread = new Thread(&VTC_ThreadHandler);
}

void VTC_ThreadDeinit() {
	// Non-accurate thread deleting...
	delete g_pThread;
	delete g_pVoiceTranscodedBufQueue;
	delete g_pVoiceRawBufQueue;
	delete g_pHaveRawBufSignal;
	delete g_pTranscodedQueueMutex;
	delete g_pRawQueueMutex;
}

void VTC_ThreadAddVoicePacket(client_t *pClient, size_t nClientIndex, clientData_t *pClientData, void *pData, size_t nDataSize) {
	VoiceBuf *pVoiceBuf = new VoiceBuf;
	pVoiceBuf->m_nPlayerIndex = nClientIndex;
	pVoiceBuf->m_nUserID = pClient->m_iUserID; // Check for player disconnect
	pVoiceBuf->m_fIsNewCodec = pClientData->hasNewCodec;
	pVoiceBuf->m_pBuf = new uint16_t[nDataSize];
	pVoiceBuf->m_nSize = nDataSize;
	memcpy(pVoiceBuf->m_pBuf, pData, nDataSize*sizeof(uint16_t));
	pVoiceBuf->m_pOutBuf = new uint8_t[2048];

	g_pRawQueueMutex->Lock();

	g_pVoiceRawBufQueue->Push(pVoiceBuf);
	g_pHaveRawBufSignal->Raise();

	g_pRawQueueMutex->Unlock();
}

void VTC_ThreadHandler(void) {
	VoiceBuf *pVoiceBuf;

	while (true) {
		g_pRawQueueMutex->Lock();

		// Non-atomic because IsEmpty + Clear (2 operations), not only IsEmpty
		// Also we can use double atomic, but I think compiler will optimize it (need volatile parameter)
		if (g_pVoiceRawBufQueue->IsEmpty()) {
			g_pHaveRawBufSignal->Wait(g_pRawQueueMutex);
		}

		pVoiceBuf = g_pVoiceRawBufQueue->Pop();

		g_pRawQueueMutex->Unlock();

		// Transcode
		if (pVoiceBuf->m_fIsNewCodec) {
			pVoiceBuf->m_nOutBufSize = g_rgClientData->m_pOldCodec->Encode((const int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, (uint8_t *)pVoiceBuf->m_pOutBuf, 2048);
		} else {
			pVoiceBuf->m_nOutBufSize = g_rgClientData->m_pNewCodec->Encode((const int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, (uint8_t *)pVoiceBuf->m_pOutBuf, 2048);
		}

		g_pTranscodedQueueMutex->Lock();

		g_pVoiceTranscodedBufQueue->Push(pVoiceBuf);

		g_pTranscodedQueueMutex->Unlock();
	}
}

void VTC_ThreadVoiceFlusher(void) {
	VoiceBuf *pVoiceBuf;

	while (true) {
		// Atomic operation
		if (g_pVoiceTranscodedBufQueue->IsEmpty()) {
			break;
		}

		g_pTranscodedQueueMutex->Lock();

		pVoiceBuf = g_pVoiceTranscodedBufQueue->Pop();

		// Send players and delete
		client_t *pClient = (client_t *)((uintptr_t)g_firstClientPtr + g_clientStructSize * (pVoiceBuf->m_nPlayerIndex - 1));

		if (pVoiceBuf->m_fIsNewCodec && g_engfuncs.pfnGetPlayerUserId(g_engfuncs.pfnPEntityOfEntIndex(pVoiceBuf->m_nPlayerIndex)) == pVoiceBuf->m_nUserID) {
			for (size_t i = 0; i < gpGlobals->maxClients; i++) {
				client_t *pDestClient = (client_t *)((uintptr_t)g_firstClientPtr + g_clientStructSize * i);

				if (!pDestClient->m_fActive) {
					continue;
				}
				if (!(pClient->m_bsVoiceStreams[0] & (1 << i))) {
					continue;
				}

				void *buf = pVoiceBuf->m_pOutBuf;
				size_t byteCount = pVoiceBuf->m_nOutBufSize;

				if (g_rgClientData[i].hasNewCodec != g_rgClientData[pVoiceBuf->m_nPlayerIndex-1].hasNewCodec && MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + byteCount) { // zachem tam eshe 2 byte v originale?
					MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, 53);
					MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, pVoiceBuf->m_nPlayerIndex - 1);
					MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, byteCount);
					MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, buf, byteCount);

					//LOG_MESSAGE(PLID, "Sended threaded recoded voice frame");
				}
			}
		}

		delete[] pVoiceBuf->m_pBuf;
		delete[] pVoiceBuf->m_pOutBuf;
		delete pVoiceBuf;

		g_pTranscodedQueueMutex->Unlock();
	}
}