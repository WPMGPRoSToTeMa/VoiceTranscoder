#pragma once

#define FIRST_BUILD_DATE	"Aug 27 2013"

#define START_LEAP_YEAR		1900
#define DAYS_PER_YEAR		365.25f

extern int DateString2Days( const char *pszDate );
extern int GetBuildNumber( void );
extern char *GetBuildNumberAsString( void );
extern char *GetCompileTime( void );