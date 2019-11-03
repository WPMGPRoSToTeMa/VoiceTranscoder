#include "Library.h"
#include "Hook_Call.h"
#include "List.h"

void *Library::FindSymbol(const char *pszSymbol) {
#ifdef WIN32
	return GetProcAddress((HMODULE)m_dwHandle, pszSymbol);
#else
	return dlsym((void *)m_dwHandle, pszSymbol);
#endif
}

void Library::FindSymbol(void *p, const char *pszSymbol) {
	*(void **)p = FindSymbol(pszSymbol);
}

dword Library::FindReference(dword dwAddr) {
#ifdef WIN32
	for (size_t n = 0; n < m_nRelocs; n++) {
		if (*m_rgpdwRelocs[n] == dwAddr) {
			return (dword)m_rgpdwRelocs[n];
		}
	}

	return 0;
#else
	// LINUX CANT DO THIS SAFE
	return 0;
#endif
}

dword Library::FindAllReference(dword dwAddr, dword *pdwCur) {
#ifdef WIN32
	for (size_t n = *pdwCur; n < m_nRelocs; n++) {
		if (*m_rgpdwRelocs[n] == dwAddr) {
			*pdwCur = n;
			return (dword)m_rgpdwRelocs[n];
		}
	}
#endif
	return 0;
}

dword Library::FindString(const char *pszString) {
#ifdef WIN32
	Section *pSection = m_pRData;

	dword dwStrAddr;

	while (pSection) {
		dwStrAddr = pSection->FindString(pszString);

		if (dwStrAddr) {
			return dwStrAddr;
		}

		pSection = pSection->Next();
	}
#endif
	return 0;
}

dword Library::FindStringUsing(const char *pszString) {
#ifdef WIN32
	dword dwStr = FindString(pszString);
	return FindReference(dwStr);
#else
	return 0;
#endif
}

dword Library::FindFunctionBeginning(dword dwAddr) {
#ifdef WIN32
	dword dwCur = dwAddr - 3;
	dword dwEnd = m_code.Start();

	while (dwCur >= dwEnd) {
		if ((*(dword *)dwCur & 0xFFFFFF) == 0xEC8B55) {
			return dwCur;
		}

		dwCur--;
	}
#endif
	return 0;
}

void *Library::FindFunctionByString(const char *pszString) {
#ifdef WIN32
	dword dwStrUsingAddr = FindStringUsing(pszString);
	return (void *)FindFunctionBeginning(dwStrUsingAddr);
#else
	return NULL;
#endif
}

void Library::FindFunctionByString(void *p, const char *pszString) {
	*(void **)p = FindFunctionByString(pszString);
}

List<Hook_Call> &Library::HookFunctionCalls(void *pAddr, void *pCallback) {
	byte *pbCur = (byte *)m_code.Start();
	byte *pbEnd = (byte *)m_code.End() - sizeof(byte) - sizeof(dword);
	List<Hook_Call> listHooks;

	while (pbCur <= pbEnd) {
		if (*pbCur++ == 0xE8) {
			long lOffset = (long)pAddr - ((long)pbCur + sizeof(long));

			if (*(long *)pbCur == lOffset) {
				listHooks.Add(Hook_Call(pbCur, pCallback));
			}
		}
	}

	return listHooks;
}

Library::Library(const char *pszName) {
#ifdef WIN32
	m_dwBase = m_dwHandle = (dword)GetModuleHandleA(pszName);

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_dwBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(m_dwBase + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((dword)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader);

	m_pRData = NULL;
	m_pVData = NULL;
	Section *pRData = NULL, *pVData = NULL;

	for (size_t n = 0; n < pNtHeaders->FileHeader.NumberOfSections; n++, pSection++) {
		if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) {
			PIMAGE_BASE_RELOCATION pBaseReloc = (PIMAGE_BASE_RELOCATION)(m_dwBase + pSection->VirtualAddress);

			// upper bound size
			m_rgpdwRelocs = (dword **)malloc(pSection->Misc.VirtualSize / sizeof(word) * sizeof(dword));

			pBaseReloc = (PIMAGE_BASE_RELOCATION)(m_dwBase + pSection->VirtualAddress);
			size_t nTotalSize = 0;
			PWORD pReloc, pEnd;
			dword dwBase;
			size_t nReloc = 0;
			while (pBaseReloc->SizeOfBlock) {
				pEnd = (PWORD)((dword)pBaseReloc + pBaseReloc->SizeOfBlock);

				dwBase = m_dwBase + pBaseReloc->VirtualAddress;

				for (pReloc = (PWORD)(pBaseReloc + 1); pReloc < pEnd; pReloc++) {
					if (((*pReloc) >> 12) == 0) {
						continue;
					}

					nTotalSize += sizeof(dword);

					((dword *)m_rgpdwRelocs)[nReloc++] = ((*pReloc) & 0xFFF) + dwBase;
				}

				pBaseReloc = (PIMAGE_BASE_RELOCATION)((dword)pBaseReloc + pBaseReloc->SizeOfBlock);
			}

			// real size
			m_rgpdwRelocs = (dword **)realloc(m_rgpdwRelocs, nTotalSize);
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
#endif
}

Library::Library(dword dwAddr) {
#ifndef WIN32 // linux chtoli
	Dl_info dlinfo;

	dladdr((void *)dwAddr, &dlinfo);

	m_dwHandle = (dword)dlopen(dlinfo.dli_fname, RTLD_NOW);
	m_dwBase = (dword)dlinfo.dli_fbase;

	FILE *pFile = fopen(dlinfo.dli_fname, "rb");
	fseek(pFile, 0, SEEK_END);
	size_t nSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	void *pBuf = malloc(nSize);
	fread(pBuf, sizeof(byte), nSize, pFile);
	fclose(pFile);
	dword dwFileBase = (dword)pBuf;

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