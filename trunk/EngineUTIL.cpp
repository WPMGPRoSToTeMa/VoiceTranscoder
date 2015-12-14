#include "EngineUTIL.h"
#include "Engine.h"

size_t EngineUTIL::GetClientIndex(client_t *pClient) {
	return ((size_t)pClient - (size_t)g_firstClientPtr) / g_clientStructSize + 1;
}