#pragma once

#include <GoldSrcEngineStructs.h>
#include "Module.h"
#include <rehlds_api.h>

const size_t SVC_STUFFTEXT = 9;
const size_t SVC_VOICEINIT = 52;
const size_t SVC_VOICEDATA = 53;

const size_t CLC_VOICEDATA = 8;

namespace EngineUTIL {
	void Init(Module &engineModule);
	bool IsReHLDS();
	IRehldsApi *GetRehldsAPI();
	size_t GetClientIndex(const client_t *pClient);
	client_t *GetClientByIndex(size_t clientIndex);
	void DropClient(client_t *client, bool breakConnection, const char *reasonFormat, ...);
	uint16_t MSG_ReadUInt16();
	void MSG_ReadBuf(void *pBuf, size_t bufSize);
	void MSG_WriteUInt8_UnSafe(sizebuf_t *buf, uintmax_t value);
	void MSG_WriteUInt16_UnSafe(sizebuf_t *buf, uintmax_t value);
	void MSG_WriteBuf_UnSafe(sizebuf_t *buf, void *binBuf, size_t byteCount);
	size_t MSG_GetRemainBytesCount(sizebuf_t *buf);
}