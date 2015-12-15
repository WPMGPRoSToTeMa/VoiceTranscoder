#pragma once

#include "Util.h"

namespace EngineUTIL {
	size_t GetClientIndex(client_t *pClient);
	client_t *GetClientByIndex(size_t clientIndex);
	void DropClient(client_t *client, bool breakConnection, const char *reasonFormat, ...);
	void MSG_WriteUInt8_UnSafe(sizebuf_t *buf, uintmax_t value);
	void MSG_WriteUInt16_UnSafe(sizebuf_t *buf, uintmax_t value);
	void MSG_WriteBuf_UnSafe(sizebuf_t *buf, void *binBuf, size_t byteCount);
	size_t MSG_GetRemainBytesCount(sizebuf_t *buf);
}