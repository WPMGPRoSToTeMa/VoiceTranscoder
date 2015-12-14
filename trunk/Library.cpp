#include "Library.h"

AnyPointer Library::FindSymbol(const char *pszSymbol) {
#ifdef WIN32
	return GetProcAddress((HMODULE)m_dwHandle, pszSymbol);
#else
	return dlsym((void *)m_dwHandle, pszSymbol);
#endif
}

uint32_t Library::FindReference(uint32_t dwAddr) {
#ifdef WIN32
	for (size_t n = 0; n < m_nRelocs; n++) {
		if (*m_rgpdwRelocs[n] == dwAddr) {
			return (uint32_t)m_rgpdwRelocs[n];
		}
	}

	return 0;
#else
	// LINUX CANT DO THIS SAFE
	return 0;
#endif
}

uint32_t Library::FindAllReference(uint32_t dwAddr, uint32_t *pdwCur) {
#ifdef WIN32
	for (size_t n = *pdwCur; n < m_nRelocs; n++) {
		if (*m_rgpdwRelocs[n] == dwAddr) {
			*pdwCur = n;
			return (uint32_t)m_rgpdwRelocs[n];
		}
	}
#endif
	return 0;
}

uint32_t Library::FindString(const char *pszString) {
	Section *pSection = m_pRData;

	uint32_t dwStrAddr;

	while (pSection) {
		dwStrAddr = pSection->FindString(pszString);

		if (dwStrAddr) {
			return dwStrAddr;
		}

		pSection = pSection->Next();
	}

	return 0;
}

uint32_t Library::FindStringUsing(const char *pszString) {
#ifdef WIN32
	uint32_t dwStr = FindString(pszString);
	return FindReference(dwStr);
#else
	return 0;
#endif
}

uint32_t Library::FindFunctionBeginning(uint32_t dwAddr) {
#ifdef WIN32
	uint32_t dwCur = dwAddr - 3;
	uint32_t dwEnd = m_code.Start();

	while (dwCur >= dwEnd) {
		if ((*(uint32_t *)dwCur & 0xFFFFFF) == 0xEC8B55) {
			return dwCur;
		}

		dwCur--;
	}
#endif
	return 0;
}

AnyPointer Library::FindFunctionByString(const char *pszString) {
#ifdef WIN32
	uint32_t dwStrUsingAddr = FindStringUsing(pszString);
	return FindFunctionBeginning(dwStrUsingAddr);
#else
	return nullptr;
#endif
}

Hook_Begin *Library::HookFunction(void *pfnAddr, void *pfnCallback) {
	Hook_Begin *hook = new Hook_Begin(pfnAddr, pfnCallback);

	return hook;
}

Library::Library(AnyPointer dwAddr) {
#ifdef WIN32
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)dwAddr, (HMODULE *)&m_dwHandle);
	m_dwBase = m_dwHandle;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_dwBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(m_dwBase + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((uint32_t)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader);

	m_pRData = NULL;
	m_pVData = NULL;
	Section *pRData = NULL, *pVData = NULL;

	for (size_t n = 0; n < pNtHeaders->FileHeader.NumberOfSections; n++, pSection++) {
		if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) {
			PIMAGE_BASE_RELOCATION pBaseReloc = (PIMAGE_BASE_RELOCATION)(m_dwBase + pSection->VirtualAddress);

			// upper bound size
			m_rgpdwRelocs = (uint32_t **)malloc(pSection->Misc.VirtualSize / sizeof(uint16_t) * sizeof(uint32_t));

			pBaseReloc = (PIMAGE_BASE_RELOCATION)(m_dwBase + pSection->VirtualAddress);
			size_t nTotalSize = 0;
			PWORD pReloc, pEnd;
			uint32_t dwBase;
			size_t nReloc = 0;
			while (pBaseReloc->SizeOfBlock) {
				pEnd = (PWORD)((uint32_t)pBaseReloc + pBaseReloc->SizeOfBlock);

				dwBase = m_dwBase + pBaseReloc->VirtualAddress;

				for (pReloc = (PWORD)(pBaseReloc + 1); pReloc < pEnd; pReloc++) {
					if (((*pReloc) >> 12) == 0) {
						continue;
					}

					nTotalSize += sizeof(uint32_t);

					((uint32_t *)m_rgpdwRelocs)[nReloc++] = ((*pReloc) & 0xFFF) + dwBase;
				}

				pBaseReloc = (PIMAGE_BASE_RELOCATION)((uint32_t)pBaseReloc + pBaseReloc->SizeOfBlock);
			}

			// real size
			m_rgpdwRelocs = (uint32_t **)realloc(m_rgpdwRelocs, nTotalSize);
			m_nRelocs = nReloc - 1;
		} else if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.BaseOfCode) {
			m_code.Set(m_dwBase + pSection->VirtualAddress, m_dwBase + pSection->VirtualAddress + pSection->Misc.VirtualSize);
		} else if (pSection->VirtualAddress >= pNtHeaders->OptionalHeader.BaseOfData) {
			if (m_pRData == NULL) {
				m_pRData = new Section(m_dwBase + pSection->VirtualAddress, pSection->SizeOfRawData);
				m_pRData->SetNext(NULL);

				m_pVData = new Section(m_dwBase + pSection->VirtualAddress, pSection->Misc.VirtualSize);
				m_pVData->SetNext(NULL);

				pRData = m_pRData;
				pVData = m_pVData;
			} else {
				pRData->SetNext(new Section(m_dwBase + pSection->VirtualAddress, pSection->SizeOfRawData));
				pRData = pRData->Next();
				pRData->SetNext(NULL);

				pVData->SetNext(new Section(m_dwBase + pSection->VirtualAddress, pSection->Misc.VirtualSize));
				pVData = pVData->Next();
				pVData->SetNext(NULL);
			}
		}
	}
#else // linux chtoli
	Dl_info dlinfo;

	dladdr((void *)dwAddr, &dlinfo);

	m_dwHandle = (uint32_t)dlopen(dlinfo.dli_fname, RTLD_NOW);
	m_dwBase = (uint32_t)dlinfo.dli_fbase;

	FILE *pFile = fopen(dlinfo.dli_fname, "rb");
	fseek(pFile, 0, SEEK_END);
	size_t nSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	void *pBuf = malloc(nSize);
	fread(pBuf, sizeof(uint8_t), nSize, pFile);
	fclose(pFile);
	uint32_t dwFileBase = (uint32_t)pBuf;

	m_pRData = NULL;
	m_pVData = NULL;
	Section *pRData = NULL, *pVData = NULL;

	Elf32_Ehdr *pEhdr = (Elf32_Ehdr *)dwFileBase;
	Elf32_Phdr *pPhdr = (Elf32_Phdr *)(dwFileBase + pEhdr->e_phoff);
	for (size_t n = 0; n < pEhdr->e_phnum; n++) {
		if (m_pRData == NULL) {
			m_pRData = new Section(m_dwBase + pPhdr->p_vaddr, pPhdr->p_filesz);
			m_pRData->SetNext(NULL);

			m_pVData = new Section(m_dwBase + pPhdr->p_vaddr, pPhdr->p_memsz);
			m_pVData->SetNext(NULL);

			pRData = m_pRData;
			pVData = m_pVData;
		} else {
			pRData->SetNext(new Section(m_dwBase + pPhdr->p_vaddr, pPhdr->p_filesz));
			pRData = pRData->Next();
			pRData->SetNext(NULL);

			pVData->SetNext(new Section(m_dwBase + pPhdr->p_vaddr, pPhdr->p_memsz));
			pVData = pVData->Next();
			pVData->SetNext(NULL);
		}

		pPhdr = (Elf32_Phdr *)((size_t)pPhdr + pEhdr->e_phentsize);
	}

	Elf32_Shdr *pStringShdr = (Elf32_Shdr *)(dwFileBase + pEhdr->e_shoff + pEhdr->e_shstrndx * pEhdr->e_shentsize);
	char *pszStringTable = (char *)(dwFileBase + pStringShdr->sh_offset);

	Elf32_Shdr *pShdr = (Elf32_Shdr *)(dwFileBase + pEhdr->e_shoff);
	for (size_t n = 0; n < pEhdr->e_shnum; n++) {
		const char *pszName = pszStringTable + pShdr->sh_name;

		if (!strcmp(pszName, ".text")) {
			m_code.Set(m_dwBase + pShdr->sh_addr, m_dwBase + pShdr->sh_addr + pShdr->sh_size);

			break;
		}

		pShdr = (Elf32_Shdr *)((size_t)pShdr + pEhdr->e_shentsize);
	}

	free(pBuf);
#endif
}