#pragma once

#include "UtilTypes.h"
#include "Section.h"
#include "List.h"
#include "Hook_Call.h"
#include <Windows.h>

class Library {
public:
	Library(dword dwAddr);
	Library(const char *pszName);
	void *Library::FindSymbol(const char *pszSymbol);
	void Library::FindSymbol(void *p, const char *pszSymbol);
	void *FindFunctionByString(const char *pszString);
	void FindFunctionByString(void *p, const char *pszString);
	List<Hook_Call> &HookFunctionCalls(void *pAddr, void *pCallback);
	template <typename T>
	dword SearchForTemplate(Template tpl, T startAddr, size_t nSize, size_t nAddOffset = 0);
	template <typename T>
	dword SearchForTemplateBackward(Template tpl, T startAddr, size_t nSize, size_t nAddOffset = 0);
	dword FindStringUsing(const char *pszString);
private:
	dword FindString(const char *pszString);
	dword FindFunctionBeginning(dword dwAddr);
	dword FindReference(dword dwAddr);
	dword FindAllReference(dword dwAddr, dword *pdwCur);
#ifdef WIN32
	dword **m_rgpdwRelocs;
	size_t m_nRelocs;

	//dword FindRefInRelocs(dword dwAddr);
#endif

	dword m_dwHandle;
	dword m_dwBase;
	Section m_code;
	Section *m_pRData;
	Section *m_pVData;
};

template <typename T>
dword Library::SearchForTemplate(Template tpl, T startAddr, size_t nSize, size_t nAddOffset /* = 0 */) {
	dword dwCur = (dword)startAddr;
	dword dwEnd = dwCur + nSize - tpl.m_nBytes;

	while (dwCur <= dwEnd) {
		if (tpl.IsEqual(dwCur)) {
			return dwCur + nAddOffset;
		}

		dwCur++;
	}

	return 0;
}

template <typename T>
dword Library::SearchForTemplateBackward(Template tpl, T startAddr, size_t nSize, size_t nAddOffset /* = 0 */) {
	dword dwCur = (dword)startAddr - tpl.m_nBytes;
	dword dwEnd = dwCur - nSize;

	while (dwCur >= dwEnd) {
		if (tpl.IsEqual(dwCur)) {
			return dwCur + nAddOffset;
		}

		dwCur--;
	}

	return 0;
}