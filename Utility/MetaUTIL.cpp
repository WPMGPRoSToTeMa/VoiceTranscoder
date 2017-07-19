#include "MetaUTIL.h"
#include <extdll.h>
#include <meta_api.h>
#include "UtilFunctions.h"

#ifndef _WIN32
#include <sys/mman.h>
#ifndef PAGESIZE
constexpr auto PAGESIZE = 0x1000;
#endif
#endif

size_t MetaUTIL::GetPluginRelPath(char *path, size_t maxPathLength) {
#ifdef _WIN32
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
#else
	auto addr = (unsigned char*)gpMetaUtilFuncs->pfnGetPluginPath;
	while (!(addr[0] == 0x11 && addr[1] == 0x0 && addr[2] == 0x0)) {
		++addr;
	}
	--addr;
	size_t oldOffs = *(size_t*)addr;
	mprotect((void*)((uintptr_t)addr / PAGESIZE*PAGESIZE), 4 + (uintptr_t)addr%PAGESIZE, PROT_EXEC | PROT_READ | PROT_WRITE);
	*(size_t*)addr = oldOffs - 0x1104;
	mprotect((void*)((uintptr_t)addr / PAGESIZE*PAGESIZE), 4 + (uintptr_t)addr%PAGESIZE, PROT_EXEC | PROT_READ);
	const char* pluginRelPath = GET_PLUGIN_PATH(PLID);
	mprotect((void*)((uintptr_t)addr / PAGESIZE*PAGESIZE), 4 + (uintptr_t)addr%PAGESIZE, PROT_EXEC | PROT_READ | PROT_WRITE);
	*(size_t*)addr = oldOffs;
	mprotect((void*)((uintptr_t)addr / PAGESIZE*PAGESIZE), 4 + (uintptr_t)addr%PAGESIZE, PROT_EXEC | PROT_READ);

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
#endif
}