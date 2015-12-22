#include "Module.h"
#include "UtilFunctions.h"
#include <cstdlib>
#ifdef __linux__
	#include <cstdio>
	#include <cstring>
	#include <dlfcn.h> // dlopen, etc...
	#include <elf.h> // Elf32 headers
#endif

const Module::Handle Module::INVALID_HANDLE = (Module::Handle)0;

Module::Module() {
	m_handle = INVALID_HANDLE;
	m_baseAddress = (uintptr_t)nullptr;
	m_isAnalyzed = false;

#ifdef _WIN32
	m_relocs = nullptr;
	m_relocsCount = 0;
#endif

	m_pRData = nullptr;
	m_pVData = nullptr;
}

Module::Module(AnyPointer insideAddress) {
	Open(insideAddress);
}

Module::~Module() {
	if (IsOpened()) {
		Close();
	}
}

void Module::Open(AnyPointer insideAddress) {
	if (IsOpened()) {
		Close();
	}

#ifdef _WIN32
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, insideAddress, &m_handle);
	m_baseAddress = (uintptr_t)m_handle;
#elif defined __linux__
	Dl_info dlinfo;
	dladdr(insideAddress, &dlinfo);

	m_handle = dlopen(dlinfo.dli_fname, RTLD_NOW);
	m_baseAddress = (uintptr_t)dlinfo.dli_fbase;
#endif
	m_isAnalyzed = false;

#ifdef _WIN32
	m_relocs = nullptr;
	m_relocsCount = 0;
#endif

	m_pRData = nullptr;
	m_pVData = nullptr;
}

void Module::Close() {
#ifdef __linux__
	dlclose(m_handle);
#endif
	m_handle = INVALID_HANDLE;
	m_baseAddress = (uintptr_t)nullptr;

	if (m_isAnalyzed) {
#ifdef _WIN32
		free(m_relocs);
		m_relocs = nullptr;
		m_relocsCount = 0;
#endif
		while (m_pRData != nullptr) {
			auto old = m_pRData;
			m_pRData = m_pRData->GetNext();
			delete old;
		}
		while (m_pVData != nullptr) {
			auto old = m_pVData;
			m_pVData = m_pVData->GetNext();
			delete old;
		}

		m_isAnalyzed = false;
	}
}

bool Module::IsOpened() const {
	return m_handle != INVALID_HANDLE;
}

void Module::Analyze() {
	if (m_isAnalyzed) {
		return;
	}

#ifdef _WIN32
	PIMAGE_DOS_HEADER pDosHeader = AnyPointer(m_baseAddress);
	PIMAGE_NT_HEADERS pNtHeaders = AnyPointer(m_baseAddress + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = AnyPointer((uintptr_t)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader);

	Section *pRData = nullptr, *pVData = nullptr;

	for (size_t i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++, pSection++) {
		if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) {
			// upper bound size
			m_relocsCount = pSection->Misc.VirtualSize / sizeof(uint16_t);
			m_relocs = (AnyPointer)malloc(m_relocsCount * sizeof(uint32_t));
			m_relocsCount = 0;

			PIMAGE_BASE_RELOCATION pBaseReloc = (AnyPointer)(m_baseAddress + pSection->VirtualAddress);
			while (pBaseReloc->SizeOfBlock) {
				uintptr_t baseAddress = m_baseAddress + pBaseReloc->VirtualAddress;

				uint16_t *pReloc = (AnyPointer)((uintptr_t)pBaseReloc + sizeof(*pBaseReloc));
				uint16_t *pEnd = (AnyPointer)((uintptr_t)pBaseReloc + pBaseReloc->SizeOfBlock);
				for (; pReloc != pEnd; pReloc++) {
					if (((*pReloc) >> 12) == 0) {
						continue;
					}

					((uint32_t *)m_relocs)[m_relocsCount++] = baseAddress + ((*pReloc) & 0xFFF);
				}

				pBaseReloc = (AnyPointer)((uintptr_t)pBaseReloc + pBaseReloc->SizeOfBlock);
			}

			// real size
			m_relocs = (AnyPointer)realloc(m_relocs, m_relocsCount * sizeof(uint32_t));
		} else if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.BaseOfCode) {
			m_codeSection.Set(m_baseAddress + pSection->VirtualAddress, m_baseAddress + pSection->VirtualAddress + pSection->Misc.VirtualSize);
		} else if (pSection->VirtualAddress >= pNtHeaders->OptionalHeader.BaseOfData) {
			if (m_pRData == nullptr) {
				m_pRData = new Section(m_baseAddress + pSection->VirtualAddress, pSection->SizeOfRawData);
				m_pVData = new Section(m_baseAddress + pSection->VirtualAddress, pSection->Misc.VirtualSize);

				pRData = m_pRData;
				pVData = m_pVData;
			} else {
				pRData->SetNext(new Section(m_baseAddress + pSection->VirtualAddress, pSection->SizeOfRawData));
				pRData = pRData->GetNext();

				pVData->SetNext(new Section(m_baseAddress + pSection->VirtualAddress, pSection->Misc.VirtualSize));
				pVData = pVData->GetNext();
			}
		}
	}
#elif defined __linux__
	Dl_info dlinfo;
	dladdr((AnyPointer)m_baseAddress, &dlinfo);

	FILE *pFile = fopen(dlinfo.dli_fname, "rb");

	// Get file size
	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	
	// Read all file data
	void *pBuf = malloc(fileSize);
	fread(pBuf, sizeof(uint8_t), fileSize, pFile);

	fclose(pFile);

	uintptr_t fileBase = (AnyPointer)pBuf;

	Section *pRData = nullptr, *pVData = nullptr;

	Elf32_Ehdr *pEhdr = (AnyPointer)fileBase;

	Elf32_Phdr *pPhdr = (AnyPointer)(fileBase + pEhdr->e_phoff);
	for (size_t i = 0; i < pEhdr->e_phnum; i++) {
		if (m_pRData == nullptr) {
			m_pRData = new Section(m_baseAddress + pPhdr->p_vaddr, pPhdr->p_filesz);
			m_pVData = new Section(m_baseAddress + pPhdr->p_vaddr, pPhdr->p_memsz);

			pRData = m_pRData;
			pVData = m_pVData;
		}
		else {
			pRData->SetNext(new Section(m_baseAddress + pPhdr->p_vaddr, pPhdr->p_filesz));
			pRData = pRData->GetNext();

			pVData->SetNext(new Section(m_baseAddress + pPhdr->p_vaddr, pPhdr->p_memsz));
			pVData = pVData->GetNext();
		}

		pPhdr = (AnyPointer)((uintptr_t)pPhdr + pEhdr->e_phentsize);
	}

	Elf32_Shdr *pStringShdr = (AnyPointer)(fileBase + pEhdr->e_shoff + pEhdr->e_shstrndx * pEhdr->e_shentsize);
	char *pStringTable = (AnyPointer)(fileBase + pStringShdr->sh_offset);

	Elf32_Shdr *pShdr = (AnyPointer)(fileBase + pEhdr->e_shoff);
	for (size_t i = 0; i < pEhdr->e_shnum; i++) {
		const char *name = pStringTable + pShdr->sh_name;

		if (!strcmp(name, ".text")) {
			m_codeSection.Set(m_baseAddress + pShdr->sh_addr, m_baseAddress + pShdr->sh_addr + pShdr->sh_size);

			break;
		}

		pShdr = (AnyPointer)((uintptr_t)pShdr + pEhdr->e_shentsize);
	}

	free(pBuf);
#endif

	m_isAnalyzed = true;
}

AnyPointer Module::FindSymbol(const char *symbolName) const {
#ifdef _WIN32
	return GetProcAddress(m_handle, symbolName);
#elif defined __linux__
	return dlsym(m_handle, symbolName);
#endif
}

AnyPointer Module::FindReference(AnyPointer _ptr) const {
	uintptr_t ptr = _ptr;

	if (!m_isAnalyzed) {
		return nullptr;
	}

#ifdef _WIN32
	for (size_t i = 0; i < m_relocsCount; i++) {
		if (*m_relocs[i] == ptr) {
			return m_relocs[i];
		}
	}

	return nullptr;
#elif defined __linux__
	// LINUX CANT DO THIS SAFE
	return nullptr;
#endif
}

AnyPointer Module::FindString(const char *str) const {
	if (!m_isAnalyzed) {
		return nullptr;
	}

	Section *pSection = m_pRData;
	while (pSection) {
		void *ptr = pSection->FindString(str);

		if (ptr != nullptr) {
			return ptr;
		}

		pSection = pSection->GetNext();
	}

	return nullptr;
}

AnyPointer Module::FindStringReference(const char *str) const {
	if (!m_isAnalyzed) {
		return nullptr;
	}

#ifdef _WIN32
	return FindReference(FindString(str));
#elif defined __linux__
	return nullptr;
#endif
}

AnyPointer Module::FindFunctionBeginning(AnyPointer ptr) const {
	if (!m_isAnalyzed) {
		return nullptr;
	}

#ifdef _WIN32
	return SearchUpForBinaryPattern(ptr, "55 8B EC"/*Default function beginning generated by MSVC*/);
#elif defined __linux__
	return nullptr;
#endif
}

AnyPointer Module::FindFunctionByString(const char *str) const {
	if (!m_isAnalyzed) {
		return nullptr;
	}

#ifdef _WIN32
	return FindFunctionBeginning(FindStringReference(str));
#elif defined __linux__
	return nullptr;
#endif
}

AnyPointer Module::SearchDownForBinaryPattern(AnyPointer _startPtr, const BinaryPattern &pattern, size_t maxDepth/* = 0*/, ptrdiff_t addOffs/* = 0*/) const {
	uintptr_t startPtr = _startPtr;

	if (!m_isAnalyzed) {
		return nullptr;
	}

	if (startPtr < (uintptr_t)m_codeSection.GetStart()) {
		return nullptr;
	}

	uintptr_t endPtr = (uintptr_t)m_codeSection.GetEnd() - pattern.GetByteCount() + sizeof(uint8_t);
	if (maxDepth != 0) {
		endPtr = Min(startPtr + maxDepth, endPtr);
	}

	if (startPtr > endPtr) {
		return nullptr;
	}

	uintptr_t ptr = startPtr;

	while (ptr != endPtr) {
		if (pattern.IsMatch(ptr)) {
			return ptr + addOffs;
		}

		ptr++;
	}

	return nullptr;
}

AnyPointer Module::SearchUpForBinaryPattern(AnyPointer _startPtr, const BinaryPattern &pattern, size_t maxDepth/* = 0*/, ptrdiff_t addOffs/* = 0*/) const {
	uintptr_t startPtr = _startPtr;

	if (!m_isAnalyzed) {
		return nullptr;
	}

	if (startPtr + pattern.GetByteCount() > (uintptr_t)m_codeSection.GetEnd()) {
		return nullptr;
	}

	uintptr_t endPtr = (uintptr_t)m_codeSection.GetStart() - sizeof(uint8_t);
	if (maxDepth != 0) {
		endPtr = Max(startPtr - maxDepth, endPtr);
	}

	if (startPtr < endPtr) {
		return nullptr;
	}

	uintptr_t ptr = startPtr;

	while (ptr != endPtr) {
		if (pattern.IsMatch(ptr)) {
			return ptr + addOffs;
		}

		ptr--;
	}

	return nullptr;
}