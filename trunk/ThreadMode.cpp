#include "ThreadMode.h"
#include "MultiThread.h"
#include "Util.h"

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
	pVoiceBuf->m_fIsNewCodec = pClientData->m_fHasNewCodec;
	pVoiceBuf->m_pBuf = new byte[nDataSize];
	pVoiceBuf->m_nSize = nDataSize;
	memcpy(pVoiceBuf->m_pBuf, pData, nDataSize);

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
		delete pVoiceBuf;

		g_pTranscodedQueueMutex->Unlock();
	}
}