#pragma once

#include "UtilTypes.h"
#include "Section.h"
#include "BinaryPattern.h"
#include "AnyPointer.h"
#ifdef _WIN32
	#include <Windows.h>
#endif

class Module {
public:
	Module();
	Module(AnyPointer insideAddress);
	~Module();
	void Open(AnyPointer insideAddress);
	void Close();
	bool IsOpened() const;
	void Analyze();
	AnyPointer FindSymbol(const char *symbolName) const;
	AnyPointer FindFunctionByString(const char *str) const;
	AnyPointer SearchDownForBinaryPattern(AnyPointer startPtr, const BinaryPattern &pattern, size_t maxDepth = 0, ptrdiff_t addOffs = 0) const;
	AnyPointer SearchUpForBinaryPattern(AnyPointer startPtr, const BinaryPattern &pattern, size_t maxDepth = 0, ptrdiff_t addOffs = 0) const;
	AnyPointer FindStringReference(const char *str) const;
	AnyPointer FindReference(AnyPointer ptr) const;
	AnyPointer FindFunctionBeginning(AnyPointer ptr) const;
	AnyPointer FindString(const char *str) const;
private:
#ifdef _WIN32
	uint32_t **m_relocs;
	size_t m_relocsCount;
#endif

#ifdef _WIN32
	typedef HMODULE Handle;
#elif defined __linux__
	typedef void* Handle;
#endif
	static const Handle INVALID_HANDLE;

	Handle m_handle;
	uintptr_t m_baseAddress;
	Section m_codeSection;
	Section *m_pRData;
	Section *m_pVData;
	bool m_isAnalyzed;
};