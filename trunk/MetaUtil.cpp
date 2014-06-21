#include "MetaUtil.h"
#include <extdll.h>
#include <meta_api.h>

size_t GetPluginDir(char *pszDest) {
	const char *pszPlugPath = GET_PLUGIN_PATH(PLID);
	const char *pchLastSlash1 = strrchr(pszPlugPath, '\\');
	const char *pchLastSlash2 = strrchr(pszPlugPath, '/');
	const char *pchLastSlash = max(pchLastSlash1, pchLastSlash2);
	size_t sizeToCopy = (pchLastSlash + 1) - pszPlugPath;

	strncpy(pszDest, pszPlugPath, sizeToCopy);
	pszDest[sizeToCopy] = '\0';

	return sizeToCopy;
}