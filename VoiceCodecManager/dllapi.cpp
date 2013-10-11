#include <extdll.h>

#include <dllapi.h>
#include <meta_api.h>

#include "voicecodecmanager.h"

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	if ( !pFunctionTable ) {
		LOG_ERROR( PLID, "GetEntityAPI2 called with null pFunctionTable" );

		return FALSE;
	} else if ( *interfaceVersion != INTERFACE_VERSION ) {
		LOG_ERROR(PLID, "GetEntityAPI2 version mismatch; requested=%d ours=%d", *interfaceVersion, INTERFACE_VERSION);

		*interfaceVersion = INTERFACE_VERSION;

		return FALSE;
	}

	pFunctionTable->pfnClientConnect = ClientConnect_Pre;

	return TRUE;
}

C_DLLEXPORT int GetNewDLLFunctions( NEW_DLL_FUNCTIONS *pNewFunctionTable, int *interfaceVersion )
{
	if ( !pNewFunctionTable ) {
		LOG_ERROR( PLID, "GetNewDLLFunctions called with null pNewFunctionTable" );

		return FALSE;
	}

	pNewFunctionTable->pfnCvarValue2 = CvarValue2_Pre;

	return TRUE;
}