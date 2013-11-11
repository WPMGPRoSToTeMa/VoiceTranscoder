#include "Build.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "UtilTypes.h"

//////////////////////////////////////////////////////////////////////////
// DateString2Days
// 
// pszDate - example: "Jul 14 2001"
//////////////////////////////////////////////////////////////////////////
int DateString2Days( const char *pszDate ) {
	static char *s_pszMonths[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static byte s_bDaysForMonths[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	int iYear, iMonth, iDays = NULL;

	for ( iMonth = 0; iMonth < 12; iMonth++ ) {
		// Parse month
		// "Jul 14 2001"
		// First 3 symbols
		if ( strncmp( &pszDate[0], s_pszMonths[ iMonth ], 3 ) == NULL ) {
			break;
		}

		iDays += s_bDaysForMonths[iMonth];
	}

	// Parse days
	// "Jul 14 2001"
	// 2 symbols after first 4 symbols
	// We need sub one cause we count from zero :D
	iDays += atoi( &pszDate[4] ) - 1;

	// Parse years
	// "Jul 14 2001"
	// 4 symbols after first 7 symbols (at the end)
	// Sub the first leap year
	iYear = atoi( &pszDate[7] ) - START_LEAP_YEAR;

	// Count total days...
	iDays += (int)( ( iYear - 1 ) * DAYS_PER_YEAR );

	// Extra day in leap year
	if ( ( ( iYear % 4 ) == 0 ) && iMonth > 1 ) {
		iDays++;
	}

	return iDays;
}

int GetBuildNumber( void ) {
	static char *s_pszDate = __DATE__;
	static int s_iBuild = 0;

	if (s_iBuild != 0) {
		return s_iBuild;
	}

	s_iBuild = DateString2Days(s_pszDate) - DateString2Days(FIRST_BUILD_DATE) + 1;

	return s_iBuild;
}

char *GetBuildNumberAsString( void ) {
	static char s_szBuild[11] = {0};

	if (s_szBuild[0] != 0) {
		return s_szBuild;
	}

	sprintf(s_szBuild, "%d", GetBuildNumber());

	return s_szBuild;
}

char *GetCompileTime( void ) {
	static char s_szCompileTime[64] = {0};

	if (s_szCompileTime[0] != 0) {
		return s_szCompileTime;
	}

	sprintf(s_szCompileTime, "%s %s", __TIME__, __DATE__);

	return s_szCompileTime;
}