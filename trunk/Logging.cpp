#include "Logging.h"

CLogger * g_pLog;

bool LoggingInitialize(void) {
	char szLogPath[260], szGamedir[32];

	GET_GAME_DIR(szGamedir);
	snprintf(szLogPath, sizeof(szLogPath), "%s/logs/", szGamedir);
	mkdir(szLogPath);
	strncat(szLogPath, "vtc.log", sizeof(szLogPath));

	g_pLog = new CLogger;
	if (!g_pLog->Init(szLogPath)) {
		return false;
	}

	return true;
}