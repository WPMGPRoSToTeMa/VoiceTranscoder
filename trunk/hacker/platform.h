#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
	#include <windows.h>
	#include <psapi.h>
#elif __linux__
	#include <dlfcn.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
	
#ifndef PAGE_SIZE
	#define	PAGE_SIZE	4096 // Размер страницы
#endif
#endif

#endif