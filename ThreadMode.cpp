#include "ThreadMode.h"
#include <extdll.h>
#include <meta_api.h>
#include <Thread.h>
#include <Mutex.h>
#include <Signal.h>
#include <GoldSrcEngineStructs.h>
#include "Main.h"
#include <EngineUTIL.h>
#include <UtilFunctions.h>
#include <CRC32.h>
#include <SteamID.h>

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
	pVoiceBuf->m_fIsNewCodec = pClientData->HasNewCodec;
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
			ChangeSamplesVolume((int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, g_pcvarVolumeNewToOld->value);

			pVoiceBuf->m_nOutBufSize = g_clientData->OldCodec->Encode((const int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, pVoiceBuf->m_pOutBuf, 2048);
		} else {
			ChangeSamplesVolume((int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, g_pcvarVolumeOldToNew->value);

			pVoiceBuf->m_nOutBufSize = g_clientData->NewCodec->Encode((const int16_t *)pVoiceBuf->m_pBuf, pVoiceBuf->m_nSize, &pVoiceBuf->m_pOutBuf[14], 2048 - 18);

			SteamID steamid;
			steamid.SetUniverse(UNIVERSE_PUBLIC);
			steamid.SetAccountType(ACCOUNT_TYPE_INDIVIDUAL);
			steamid.SetAccountId(0xFFFFFFFF); // 0 is invalid, but maximum value valid, TODO: randomize or get non-steam user steamid?
			steamid.SetAccountInstance(STEAMUSER_DESKTOPINSTANCE);
			*(uint64_t *)pVoiceBuf->m_pOutBuf = steamid.ToUInt64();
			*(uint8_t *)&pVoiceBuf->m_pOutBuf[8] = VPC_SETSAMPLERATE;
			*(uint16_t *)&pVoiceBuf->m_pOutBuf[9] = 8000;
			*(uint8_t *)&pVoiceBuf->m_pOutBuf[11] = VPC_VDATA_SILK;
			*(uint16_t *)&pVoiceBuf->m_pOutBuf[12] = (uint16_t)pVoiceBuf->m_nOutBufSize;

			CRC32 checksum;
			checksum.Init();
			checksum.Update(pVoiceBuf->m_pOutBuf, 14 + pVoiceBuf->m_nOutBufSize);
			checksum.Final();

			*(uint32_t *)&pVoiceBuf->m_pOutBuf[14 + pVoiceBuf->m_nOutBufSize] = checksum.ToUInt32();

			pVoiceBuf->m_nOutBufSize += 18;
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
		if (g_engfuncs.pfnGetPlayerUserId(g_engfuncs.pfnPEntityOfEntIndex(pVoiceBuf->m_nPlayerIndex)) == pVoiceBuf->m_nUserID) {
			client_t *pClient = EngineUTIL::GetClientByIndex(pVoiceBuf->m_nPlayerIndex);

			for (size_t i = 0; i < gpGlobals->maxClients; i++) {
				client_t *pDestClient = EngineUTIL::GetClientByIndex(i + 1);

				if (!pDestClient->m_fActive) {
					continue;
				}
				if (!pDestClient->m_fHltv || g_pcvarForceSendHLTV->value == 0) {
					if (!(pClient->m_bsVoiceStreams[0] & (1 << i))) {
						continue;
					}
				}

				void *buf = pVoiceBuf->m_pOutBuf;
				size_t byteCount = pVoiceBuf->m_nOutBufSize;

				if (g_clientData[i].HasNewCodec != g_clientData[pVoiceBuf->m_nPlayerIndex-1].HasNewCodec && EngineUTIL::MSG_GetRemainBytesCount(&pDestClient->m_Datagram) >= sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + byteCount) { // zachem tam eshe 2 byte v originale?
					EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, 53);
					EngineUTIL::MSG_WriteUInt8_UnSafe(&pDestClient->m_Datagram, pVoiceBuf->m_nPlayerIndex - 1);
					EngineUTIL::MSG_WriteUInt16_UnSafe(&pDestClient->m_Datagram, byteCount);
					EngineUTIL::MSG_WriteBuf_UnSafe(&pDestClient->m_Datagram, buf, byteCount);

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