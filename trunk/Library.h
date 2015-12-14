#pragma once

#include "UtilTypes.h"
#include "Section.h"
#include "Hook_Begin.h"
#include "BinaryPattern.h"
#include "AnyPointer.h"
#include <Windows.h>

class Library {
public:
	Library(AnyPointer funcPtr);
	AnyPointer FindSymbol(const char *symbolName);
	AnyPointer FindFunctionByString(const char *pszString);
	Hook_Begin *HookFunction(void *pfnAddr, void *pfnCallback);
	template <typename T>
	uint32_t SearchForTemplate(BinaryPattern tpl, T startAddr, size_t nSize, size_t nAddOffset = 0);
	template <typename T>
	uint32_t SearchForTemplateBackward(BinaryPattern tpl, T startAddr, size_t nSize, size_t nAddOffset = 0);
	uint32_t FindStringUsing(const char *pszString);
private:
	uint32_t FindString(const char *pszString);
	uint32_t FindFunctionBeginning(uint32_t dwAddr);
	uint32_t FindReference(uint32_t dwAddr);
	uint32_t FindAllReference(uint32_t dwAddr, uint32_t *pdwCur);
#ifdef WIN32
	uint32_t **m_rgpdwRelocs;
	size_t m_nRelocs;

	//dword FindRefInRelocs(dword dwAddr);
#endif

	uint32_t m_dwHandle;
	uint32_t m_dwBase;
	Section m_code;
	Section *m_pRData;
	Section *m_pVData;
};

template <typename T>
uint32_t Library::SearchForTemplate(BinaryPattern tpl, T startAddr, size_t nSize, size_t nAddOffset /* = 0 */) {
	uint32_t dwCur = (uint32_t)startAddr;
	uint32_t dwEnd = dwCur + nSize - tpl.m_nBytes;

	while (dwCur <= dwEnd) {
		if (tpl.IsEqual(dwCur)) {
			return dwCur + nAddOffset;
		}

		dwCur++;
	}

	return 0;
}

template <typename T>
uint32_t Library::SearchForTemplateBackward(BinaryPattern tpl, T startAddr, size_t nSize, size_t nAddOffset /* = 0 */) {
	uint32_t dwCur = (uint32_t)startAddr - tpl.m_nBytes;
	uint32_t dwEnd = dwCur - nSize;

	while (dwCur >= dwEnd) {
		if (tpl.IsEqual(dwCur)) {
			return dwCur + nAddOffset;
		}

		dwCur--;
	}

	return 0;
}