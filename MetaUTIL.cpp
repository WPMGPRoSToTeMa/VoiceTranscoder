#include "MetaUTIL.h"
#include <meta_api.h>

size_t MetaUTIL::GetPluginRelPath(char *szPath, size_t nMaxPathLen)
{
	const char *pszPluginAbsPath = GET_PLUGIN_PATH(PLID);
	const char *pszGamedirAbsPath = GET_GAME_INFO(PLID, GINFO_GAMEDIR);
	const char *pszPluginRelPath = &pszPluginAbsPath[strlen(pszGamedirAbsPath) + 1]; // + slash

	// Find last slash
	const char *pszSearchChar = pszPluginRelPath + strlen(pszPluginRelPath) - 1; // go to string end

	while (true) {
		if (*pszSearchChar == '/' || *pszSearchChar == '\\') {
			break; // ok we found it
		}

		pszSearchChar--;
	}

	size_t nCharsToCopy = min((size_t)pszSearchChar - (size_t)pszPluginRelPath + 1, nMaxPathLen); // with slash
	strncpy(szPath, pszPluginRelPath, nCharsToCopy);
	szPath[nCharsToCopy] = '\0';

	return nCharsToCopy;
}