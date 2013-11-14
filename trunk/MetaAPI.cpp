#include <extdll.h>

#include <meta_api.h>

#include "sdk_util.h"

#include "Build.h"

#include "VoiceTranscoder.h"

#include "Logging.h"

// Must provide at least one of these..
static META_FUNCTIONS gMetaFunctionTable = {
	NULL,				// pfnGetEntityAPI
	NULL,				// pfnGetEntityAPI_Post
	GetEntityAPI2,		// pfnGetEntityAPI2
	NULL,				// pfnGetEntityAPI2_Post
	GetNewDLLFunctions,	// pfnGetNewDLLFunctions
	NULL,				// pfnGetNewDLLFunctions_Post
	NULL,				// pfnGetEngineFunctions
	NULL,				// pfnGetEngineFunctions_Post
};

// Description of plugin
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION,							// ifvers
	"VoiceTranscoder",								// name
	GetBuildNumberAsString(),						// version
	GetCompileTime(),								// date
	"[WPMG]PRoSToTeM@ <wpmgprostotema@live.ru>",	// author
	"http://www.wpmg.ru/",							// url
	"VTC",											// logtag, all caps please
	PT_ANYTIME,										// (when) loadable
	PT_ANYTIME,										// (when) unloadable
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(char *, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs) {
	*pPlugInfo = &Plugin_info;

	gpMetaUtilFuncs = pMetaUtilFuncs;

	if (!LoggingInitialize()) {
		LOG_ERROR(PLID, "Couldn't initialize logging");

		return FALSE;
	}

	g_pLog->Printf("Logging started (Meta_Query)\n");

	return TRUE;
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME , META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) {
	g_pLog->Printf("Start Meta_Attach\n");

	if ( !pMGlobals ) {
		LOG_ERROR( PLID, "Meta_Attach called with null pMGlobals" );
		g_pLog->Printf("ERROR: Meta_Attach called with null pMGlobals\n");

		return FALSE;
	}

	gpMetaGlobals = pMGlobals;

	if ( !pFunctionTable ) {
		LOG_ERROR( PLID, "Meta_Attach called with null pFunctionTable" );
		g_pLog->Printf("ERROR: Meta_Attach called with null pFunctionTable\n");

		return FALSE;
	}

	memcpy( pFunctionTable, &gMetaFunctionTable, sizeof( META_FUNCTIONS ) );

	gpGamedllFuncs = pGamedllFuncs;

	if ( !VTC_Init( ) ) {
		LOG_ERROR( PLID, "Could not initialize a plugin" );
		g_pLog->Printf("ERROR: Could not initialize a plugin\n");
		
		return FALSE;
	}

	g_pLog->Printf("End Meta_Attach\n");

	return TRUE;
}

C_DLLEXPORT int Meta_Detach( PLUG_LOADTIME , PL_UNLOAD_REASON ) {
	g_pLog->Printf("Start Meta_Detach\n");

	if ( !VTC_End() ) {
		LOG_ERROR( PLID, "Could not end a plugin" );
		g_pLog->Printf("ERROR: Could not end a plugin\n");

		return FALSE;
	}
	
	g_pLog->Printf("End Meta_Detach\n");

	LoggingDeinitialize();

	return TRUE;
}
