#include <extdll.h>

#include <meta_api.h>

#include "sdk_util.h"

#include "build.h"

#include "voicecodecmanager.h"

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
	"VoiceCodecManager",							// name
	GetBuildNumberAsString(),						// version
	GetCompileTime(),								// date
	"[WPMG]PRoSToTeM@ <wpmgprostotema@live.ru>",	// author
	"http://www.wpmg.ru/",							// url
	"VCM",											// logtag, all caps please
	PT_ANYTIME,										// (when) loadable
	PT_ANYPAUSE,									// (when) unloadable
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(char *, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs) {
	*pPlugInfo = &Plugin_info;

	gpMetaUtilFuncs = pMetaUtilFuncs;

	return TRUE;
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME , META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) {
	if ( !pMGlobals ) {
		LOG_ERROR( PLID, "Meta_Attach called with null pMGlobals" );

		return FALSE;
	}

	gpMetaGlobals = pMGlobals;

	if ( !pFunctionTable ) {
		LOG_ERROR( PLID, "Meta_Attach called with null pFunctionTable" );

		return FALSE;
	}

	memcpy( pFunctionTable, &gMetaFunctionTable, sizeof( META_FUNCTIONS ) );

	gpGamedllFuncs = pGamedllFuncs;

	if ( !VCM_Init( ) ) {
		LOG_ERROR( PLID, "Could not initialize a plugin" );
		
		return FALSE;
	}

	return TRUE;
}

C_DLLEXPORT int Meta_Detach( PLUG_LOADTIME , PL_UNLOAD_REASON ) {
	if ( !VCM_End() ) {
		LOG_ERROR( PLID, "Could not end a plugin" );

		return FALSE;
	}

	return TRUE;
}
