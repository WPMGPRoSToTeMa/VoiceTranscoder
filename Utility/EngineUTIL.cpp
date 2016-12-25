#include "EngineUTIL.h"
#include <rehlds_api.h>
#include <extdll.h>
#include <meta_api.h>
#include "Module.h"

bool g_isUsingRehldsAPI;
void (* g_pfnSvDropClient)(client_t *pClient, bool fCrash, char *pszFormat, ...);
sizebuf_t *g_pNetMessage;
size_t *g_pMsgReadCount;
bool *g_pMsgBadRead;

IRehldsApi *g_pRehldsAPI;

IRehldsApi *GetRehldsAPI(const Module &engineModule) {
	void *(* CreateInterface)(const char *interfaceName, size_t *returnCode) = engineModule.FindSymbol("CreateInterface");

	if (CreateInterface == nullptr) {
		return nullptr;
	}

	return (AnyPointer)CreateInterface("VREHLDS_HLDS_API_VERSION001", nullptr);
}

void EngineUTIL::Init(Module &engineModule) {
	g_pRehldsAPI = GetRehldsAPI(engineModule);

	if (g_pRehldsAPI != nullptr) {
		g_isUsingRehldsAPI = true;

		// TODO: add error messages
		if (g_pRehldsAPI->GetMajorVersion() != REHLDS_API_VERSION_MAJOR) {
			return;
		}
		if (g_pRehldsAPI->GetMinorVersion() < REHLDS_API_VERSION_MINOR) {
			return;
		}

		g_pMsgReadCount = (AnyPointer)g_pRehldsAPI->GetFuncs()->GetMsgReadCount();
		g_pMsgBadRead = (AnyPointer)g_pRehldsAPI->GetFuncs()->GetMsgBadRead();
		g_pNetMessage = g_pRehldsAPI->GetFuncs()->GetNetMessage();
	} else {
#ifdef _WIN32
		engineModule.Analyze();

		g_pfnSvDropClient = engineModule.FindFunctionByString("Dropped %s from server\nReason:  %s\n");

		// TODO: we can simple find next reloc
		uintptr_t ptr = engineModule.FindStringReference("Failed command checksum for %s:%s\n");
		ptr = *(uintptr_t *)engineModule.SearchDownForBinaryPattern(ptr, "C7 05 ?? ?? ?? ?? 01 00 00 00", 0x20, 2);
		g_pMsgReadCount = (AnyPointer)(ptr - 4);
		g_pMsgBadRead = (AnyPointer)ptr;

		ptr = engineModule.FindStringReference("net_message");
		ptr = *(uintptr_t *)(ptr - 4);
		g_pNetMessage = (AnyPointer)(ptr - offsetof(sizebuf_t, buffername));
#elif defined __linux__
		g_pfnSvDropClient = engineModule.FindSymbol("SV_DropClient");
		g_pMsgReadCount = engineModule.FindSymbol("msg_readcount");
		g_pMsgBadRead = engineModule.FindSymbol("msg_badread");
		g_pNetMessage = engineModule.FindSymbol("net_message");
#endif
	}
}

bool EngineUTIL::IsReHLDS() {
	return g_isUsingRehldsAPI;
}

IRehldsApi *EngineUTIL::GetRehldsAPI() {
	return g_pRehldsAPI;
}

size_t GetSizeOfClientStruct() {
	return (size_t)GET_INFOKEYBUFFER(INDEXENT(2)) - (size_t)GET_INFOKEYBUFFER(INDEXENT(1));
}

client_t *GetFirstClientPtr() {
	return (AnyPointer)((size_t)GET_INFOKEYBUFFER(INDEXENT(1)) - offsetof(client_t, m_szUserInfo));
}

size_t EngineUTIL::GetClientIndex(const client_t *pClient) {
	if (g_isUsingRehldsAPI) {
		return g_pRehldsAPI->GetServerStatic()->GetIndexOfClient_t((client_t *)pClient) + 1;
	}
	return ((size_t)pClient - (size_t)GetFirstClientPtr()) / GetSizeOfClientStruct() + 1;
}

client_t *EngineUTIL::GetClientByIndex(size_t clientIndex) {
	if (g_isUsingRehldsAPI) {
		return g_pRehldsAPI->GetServerStatic()->GetClient_t(clientIndex - 1);
	}
	return (client_t *)((uintptr_t)GetFirstClientPtr() + GetSizeOfClientStruct() * (clientIndex - 1));
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

uint16_t EngineUTIL::MSG_ReadUInt16() {
	if (g_isUsingRehldsAPI) {
		return (uint16_t)g_pRehldsAPI->GetFuncs()->MSG_ReadShort();
	}
	
	if (*g_pMsgReadCount + sizeof(uint16_t) > g_pNetMessage->cursize) {
		*g_pMsgBadRead = true;
		return 0;
	}
	
	uint16_t ret = *(uint16_t *)&g_pNetMessage->data[*g_pMsgReadCount];
	*g_pMsgReadCount += sizeof(uint16_t);

	return ret;
}

void EngineUTIL::MSG_ReadBuf(void *pBuf, size_t bufSize) {
	if (g_isUsingRehldsAPI) {
		g_pRehldsAPI->GetFuncs()->MSG_ReadBuf(bufSize, pBuf);
		return;
	}

	if (*g_pMsgReadCount + bufSize > g_pNetMessage->cursize) {
		*g_pMsgBadRead = true;
		return;
	}

	memcpy(pBuf, &g_pNetMessage->data[*g_pMsgReadCount], bufSize);
	*g_pMsgReadCount += bufSize;
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
	} else {
		*(uint16_t *)&buf->data[buf->cursize] = (uint16_t)value;

		buf->cursize += sizeof(uint16_t);
	}
}

void EngineUTIL::MSG_WriteBuf_UnSafe(sizebuf_t *buf, void *binBuf, size_t byteCount) {
	if (g_isUsingRehldsAPI) {
		g_pRehldsAPI->GetFuncs()->MSG_WriteBuf(buf, byteCount, binBuf);
	} else {
		memcpy(&buf->data[buf->cursize], binBuf, byteCount);

		buf->cursize += byteCount;
	}
}

size_t EngineUTIL::MSG_GetRemainBytesCount(sizebuf_t *buf) {
	return buf->maxsize - buf->cursize;
}