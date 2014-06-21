#include "Logger.h"
#include <extdll.h>
#include <meta_api.h>
#include "VoiceTranscoder.h"

bool CLogger::Init(const char * pszFilename) {
	m_pfile = NULL;
	//m_pfile = fopen(pszFilename, "at");

	//if (!m_pfile) {
	//	return false;
	//}

	return true;
}

CLogger::~CLogger() {
	if (m_pfile) {
		fclose(m_pfile);
	}
}

void CLogger::Printf(const char * pszFmt, ...) {
	char szTemp[32];
	time_t rawtime;
	tm * timeinfo;
	va_list args;

	if (g_pcvarVtcLog->value == 0) {
		return;
	}

	if (!m_pfile) {
		char szFileName[260];

		if (!g_pcvarVtcLogDir->string[0]) {
			const char *pszPlugPath = GET_PLUGIN_PATH(PLID);
			const char *pchLastSlash1 = strrchr(pszPlugPath, '\\');
			const char *pchLastSlash2 = strrchr(pszPlugPath, '/');
			const char *pchLastSlash = max(pchLastSlash1, pchLastSlash2);
			size_t sizeToCopy = (pchLastSlash + 1) - pszPlugPath;

			strncpy(szFileName, pszPlugPath, sizeToCopy);
			szFileName[sizeToCopy] = '\0';
			strcat(szFileName, "vtc.log");
		} else {
			char szGamedir[32];

			GET_GAME_DIR(szGamedir);

			sprintf(szFileName, "%s/%s/vtc.log", szGamedir, g_pcvarVtcLogDir->string);
		}

		m_pfile = fopen(szFileName, "at");

		if (!m_pfile) {
			return;
		}
	}

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(szTemp, sizeof(szTemp), "L %m/%d/%Y - %H:%M:%S: ", timeinfo);

	fputs(szTemp, m_pfile);

	va_start(args, pszFmt);
	vfprintf(m_pfile, pszFmt, args);
	va_end(args);

	fflush(m_pfile);
}