#include "DProtoAPI.h"
#include "EngineFuncs.h"
// Metamod API
#include <extdll.h>
#include <dllapi.h>
#include <meta_api.h>

dp_enginfo_api_t *g_pDprotoAPI;

bool DProtoAPI_Init(void) {
	// Get dproto API
	const char *pszDpApi = CVAR_GET_STRING(DPROTO_API_CVAR_NAME);

#ifdef VTC_DEBUG
	LOG_MESSAGE(PLID, "%s = %s", DPROTO_API_CVAR_NAME, pszDpApi);
#endif

	// Check for dpapi
	if (pszDpApi[0] == '\0' || pszDpApi[0] == '0') {
#ifdef VTC_DEBUG
		LOG_ERROR(PLID, "Can't get DPAPI");
#endif

		return false;
	}

	// Scan for addr
	sscanf(pszDpApi, "%u", &g_pDprotoAPI);

#ifdef VTC_DEBUG
	LOG_MESSAGE(PLID, "Found DprotoAPI at %.8X", g_pDprotoAPI);
#endif

	// Check major ver
	if (g_pDprotoAPI->version_major != DPROTO_ENGINFO_API_VERSION_MAJOR) {
#ifdef VTC_DEBUG
		LOG_ERROR(PLID, "DPAPI major version checking failed: current %d (need %d)", g_pDprotoAPI->version_major, DPROTO_ENGINFO_API_VERSION_MAJOR);
#endif

		return false;
	}
	// Check minor ver
	if (g_pDprotoAPI->version_minor < DPROTO_ENGINFO_API_VERSION_MINOR) {
#ifdef VTC_DEBUG
		LOG_ERROR(PLID, "DPAPI minor version checking failed: current %d (need minimal %d)", g_pDprotoAPI->version_minor, DPROTO_ENGINFO_API_VERSION_MINOR);
#endif

		return false;
	}

	return true;
}

bool DProtoAPI_Deinit(void) {
	return true;
}