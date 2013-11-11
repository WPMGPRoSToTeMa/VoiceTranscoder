#include <extdll.h>

#include <h_export.h>

enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

void WINAPI GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals ) {
	memcpy( &g_engfuncs, pengfuncsFromEngine, sizeof( enginefuncs_t ) );

	gpGlobals = pGlobals;
}
