#include <extdll.h>
#include <enginecallback.h>

#include "osdep.h"

void UTIL_LogPrintf( char *fmt, ... ) {
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf ( string, sizeof(string), fmt, argptr );
	va_end   ( argptr );

	ALERT( at_logged, "%s", string );
}
