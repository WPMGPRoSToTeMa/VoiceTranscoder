#include "EngineUTIL.h"
#include "Engine.h"

size_t EngineUTIL::GetClientId(client_t *pClient) {
	return ((size_t)pClient - (size_t)g_psvs->m_pClients) / g_nClientStructSize + 1;
}