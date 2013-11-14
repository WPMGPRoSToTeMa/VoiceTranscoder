#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

class CLogger {
	FILE * m_pfile;

public:
	~CLogger();

	bool Init(const char * pszFilename);
	void Printf(const char * pszFmt, ...);
};

