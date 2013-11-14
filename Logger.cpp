#include "Logger.h"
#include <extdll.h>
#include <meta_api.h>

bool CLogger::Init(const char * pszFilename) {
	m_pfile = fopen(pszFilename, "at");

	if (!m_pfile) {
		return false;
	}

	return true;
}

CLogger::~CLogger() {
	fclose(m_pfile);
}

void CLogger::Printf(const char * pszFmt, ...) {
	char szTemp[32];
	time_t rawtime;
	tm * timeinfo;
	va_list args;

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(szTemp, sizeof(szTemp), "L %m/%d/%Y - %H:%M:%S: ", timeinfo);

	fputs(szTemp, m_pfile);

	va_start(args, pszFmt);
	vfprintf(m_pfile, pszFmt, args);
	va_end(args);

	fflush(m_pfile);
}