#include "Logging.h"

CLogger * g_pLog;

bool LoggingInitialize(void) {
	char szLogPath[260], szGamedir[32];

	GET_GAME_DIR(szGamedir);
	snprintf(szLogPath, sizeof(szLogPath), "%s/logs/", szGamedir);
#ifdef __linux__
	mkdir(szLogPath, 0700);
#else
	mkdir(szLogPath);
#endif
	strncat(szLogPath, "vtc.log", sizeof(szLogPath));

	g_pLog = new CLogger;
	if (!g_pLog->Init(szLogPath)) {
		return false;
	}

	return true;
}

bool LoggingDeinitialize(void) {
	delete g_pLog;

	return true;
}