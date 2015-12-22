#include "MetaUTIL.h"
#include <extdll.h>
#include <meta_api.h>
#include "UtilFunctions.h"

size_t MetaUTIL::GetPluginRelPath(char *path, size_t maxPathLength) {
	const char *pluginAbsPath = GET_PLUGIN_PATH(PLID);
	const char *gamedirAbsPath = GET_GAME_INFO(PLID, GINFO_GAMEDIR);
	const char *pluginRelPath = &pluginAbsPath[strlen(gamedirAbsPath) + 1]; // + slash

	// Find last slash
	const char *searchChar = pluginRelPath + strlen(pluginRelPath) - 1; // go to string end

	while (true) {
		if (*searchChar == '/' || *searchChar == '\\') {
			break; // ok we found it
		}

		searchChar--;
	}

	size_t charsToCopy = Min((size_t)searchChar - (size_t)pluginRelPath + 1, maxPathLength); // with slash
	strncpy(path, pluginRelPath, charsToCopy);
	path[charsToCopy] = '\0';

	return charsToCopy;
}