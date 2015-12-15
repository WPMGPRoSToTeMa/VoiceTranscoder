#include "EngineUTIL.h"
#include <rehlds_api.h>
#include "Main.h"

size_t EngineUTIL::GetClientIndex(client_t *pClient) {
	if (g_isUsingRehldsAPI) {
		return g_pRehldsAPI->GetServerStatic()->GetIndexOfClient_t(pClient) + 1;
	}
	return ((size_t)pClient - (size_t)g_firstClientPtr) / g_clientStructSize + 1;
}

client_t *EngineUTIL::GetClientByIndex(size_t clientIndex) {
	if (g_isUsingRehldsAPI) {
		return g_pRehldsAPI->GetServerStatic()->GetClient_t(clientIndex - 1);
	}
	return (client_t *)((uintptr_t)g_firstClientPtr + g_clientStructSize * (clientIndex - 1));
}

void EngineUTIL::DropClient(client_t *client, bool breakConnection, const char *reasonFormat, ...) {
	char reason[1024];
	
	va_list valist;
	va_start(valist, reasonFormat);
	vsnprintf(reason, sizeof(reason), reasonFormat, valist);
	va_end(valist);

	if (g_isUsingRehldsAPI) {
		return g_pRehldsAPI->GetFuncs()->DropClient(g_pRehldsAPI->GetServerStatic()->GetClient(g_pRehldsAPI->GetServerStatic()->GetIndexOfClient_t(client)), breakConnection, "%s", reason);
	}
	return g_pfnSvDropClient(client, breakConnection, "%s", reason);
}

void EngineUTIL::MSG_WriteUInt8_UnSafe(sizebuf_t *buf, uintmax_t value) {
	if (g_isUsingRehldsAPI) {
		g_pRehldsAPI->GetFuncs()->MSG_WriteByte(buf, (uint8_t)value);
	} else {
		*(uint8_t *)&buf->data[buf->cursize] = (uint8_t)value;

		buf->cursize += sizeof(uint8_t);
	}
}

void EngineUTIL::MSG_WriteUInt16_UnSafe(sizebuf_t *buf, uintmax_t value) {
	if (g_isUsingRehldsAPI) {
		g_pRehldsAPI->GetFuncs()->MSG_WriteShort(buf, (uint16_t)value);
	}
	else {
		*(uint16_t *)&buf->data[buf->cursize] = (uint16_t)value;

		buf->cursize += sizeof(uint16_t);
	}
}

void EngineUTIL::MSG_WriteBuf_UnSafe(sizebuf_t *buf, void *binBuf, size_t byteCount) {
	if (g_isUsingRehldsAPI) {
		g_pRehldsAPI->GetFuncs()->MSG_WriteBuf(buf, byteCount, binBuf);
	}
	else {
		memcpy(&buf->data[buf->cursize], binBuf, byteCount);

		buf->cursize += byteCount;
	}
}

size_t EngineUTIL::MSG_GetRemainBytesCount(sizebuf_t *buf) {
	return buf->maxsize - buf->cursize;
}