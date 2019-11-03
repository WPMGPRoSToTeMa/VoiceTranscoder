/*
1. Hook SV_ParseVoiceData
2. Hook ClientConnect
3. Hook ClientCommand
4. Hook MSG_WriteByte in SV_New_f
What we need:
svs
sizeof(client_t)
SV_DropClient ?
msg_readcount
msg_badread
net_message
SZ_GetSpace
SV_ParseVoiceData
*/
#include <extdll.h>
#include <meta_api.h>
#include "Util.h"

#ifdef _WIN32
	#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#endif

// Constants and macros
#define SVC_STUFFTEXT	9
const size_t c_nMaxVoicePacketSize = 2048;

const char *c_szConfigName = "vtc.cfg";

typedef void SV_DROPCLIENT(client_t *pClient, bool fCrash, char *pszFormat, ...);

// Externs
extern void ClientCommand_Pre(edict_t *pClient);
extern qboolean ClientConnect_Post(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason);
extern void ServerActivate_Post(edict_t *pEdictList, int nEdictCount, int nClientMax);
extern size_t MetaUTIL_GetPluginRelPath(char *szPath, size_t nMaxPathLen);
extern void VTC_InitCvars(void);
extern void VTC_ExecConfig(void);
extern void VTC_InitConfig(void);
extern void VTC_InitCodecs(void);
extern bool EngineParser(void);
extern size_t *g_pnMsgReadCount;
extern bool *g_pfMsgBadRead;

// Structs and classes
struct clientData_t {
	bool m_fHasNewCodec;
	bool m_fIsChecking;
	bool m_fIsLogSecretRecvd;
	qword m_qwNextPacketNs;
	qword m_qwNextPacketNs2;
};

struct netmsgbuf_t : public sizebuf_t {
	void *Get(void) {
		return &data[*g_pnMsgReadCount];
	}
	bool BoundRead(size_t n) {
		if (*g_pnMsgReadCount + n > cursize) {
			*g_pfMsgBadRead = true;

			return false;
		}

		return true;
	}
	template <typename T>
	T ReadType(void) {
		if (!BoundRead(sizeof(T))) {
			return -1;
		}

		T tResult = *(T *)Get();

		*g_pnMsgReadCount += sizeof(T);

		return tResult;
	}
	word ReadWord(void) {
		return ReadType<word>();
	}
	void ReadMem(void *pMem, size_t nSize) {
		if (!BoundRead(nSize)) {
			return;
		}

		memcpy(pMem, Get(), nSize);

		*g_pnMsgReadCount += nSize;
	}
};

struct voicebuf_t {
	void *m_pBuf;
	size_t m_nSize;
	size_t m_nPlayerIndex;
	bool m_fIsNewCodec;
	bool m_fCompleted;
	voicebuf_t *m_pNext;
};

struct voicebufqueue_t {
	voicebuf_t *m_pFirst;
	voicebuf_t *m_pFirstUnworked;
	voicebuf_t *m_pLast;
};

// Variables
clientData_t g_rgClientData[33];

char g_szExecConfigCmd[300];

voicebuf_t *g_pFirstVoiceBuf, g_pLastVoiceBuf;
voicebufqueue_t g_pVoiceBufQueue;
HANDLE g_hEvent;

// Hacked
size_t g_nClientStructSize;
server_static_t *g_psvs;
size_t *g_pnMsgReadCount;
bool *g_pfMsgBadRead;
netmsgbuf_t *g_pNetMessage;
SV_DROPCLIENT *g_pfnSvDropClient;

// Cvars
cvar_t g_cvarVersion = {"VTC_Version", Plugin_info.version, FCVAR_EXTDLL | FCVAR_SERVER, 0, NULL};
cvar_t *g_pcvarVersion;
cvar_t g_cvarDefaultCodec = {"VTC_DefaultCodec", "old", FCVAR_EXTDLL, 0, NULL};
cvar_t *g_pcvarDefaultCodec;
cvar_t g_cvarHltvCodec = {"VTC_HltvCodec", "old", FCVAR_EXTDLL, 0, NULL};
cvar_t *g_pcvarHltvCodec;
cvar_t *g_pcvarVoiceEnable;

// Engine API
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEngFuncs, globalvars_t *pGlobalVars) {
	memcpy(&g_engfuncs, pEngFuncs, sizeof(g_engfuncs));
	gpGlobals = pGlobalVars;
}

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientCommand = &ClientCommand_Pre;

	return TRUE;
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) {
	// Clear
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	pFunctionTable->pfnClientConnect = &ClientConnect_Post;
	pFunctionTable->pfnServerActivate = &ServerActivate_Post;

	return TRUE;
}

// Entity API
void ClientCommand_Pre(edict_t *pClient) {
	const char *pszCmd = CMD_ARGV(0);

	int nClientIndex = ENTINDEX(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex];

	if (FStrEq(pszCmd, "VTC_CheckStart")) {
		pClientData->m_fIsChecking = true;
		pClientData->m_fHasNewCodec = false;
		pClientData->m_fIsLogSecretRecvd = false;
	} else if (pClientData->m_fIsChecking) {
		if (FStrEq(pszCmd, "sv_logsecret")) {
			pClientData->m_fIsLogSecretRecvd = true;
		} else if (FStrEq(pszCmd, "VTC_CheckEnd")) {
			pClientData->m_fIsChecking = false;
			pClientData->m_fHasNewCodec = pClientData->m_fIsLogSecretRecvd ? false : true;
			pClientData->m_fIsLogSecretRecvd = false;

			LOG_MESSAGE(PLID, "Client with %s codec connected", pClientData->m_fHasNewCodec ? "new" : "old");
		}
	}

	RETURN_META(MRES_IGNORED);
}

qboolean ClientConnect_Post(edict_t *pClient, const char *pszName, const char *pszAddress, char *pszRejectReason) {
	int nClientIndex = ENTINDEX(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex];

	// Default client codec
	if (FStrEq(GETPLAYERAUTHID(pClient), "HLTV")) {
		pClientData->m_fHasNewCodec = FStrEq(g_pcvarHltvCodec->string, "old") ? false : true;
	} else {
		pClientData->m_fHasNewCodec = FStrEq(g_pcvarDefaultCodec->string, "old") ? false : true;
	}
	pClientData->m_fIsChecking = false;

	RETURN_META_VALUE(MRES_IGNORED, META_RESULT_ORIG_RET(qboolean));
}

void ServerActivate_Post(edict_t *pEdictList, int nEdictCount, int nClientMax) {
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("VTC_CheckStart\n");
	MESSAGE_END();
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("sv_logsecret\n");
	MESSAGE_END();
	MESSAGE_BEGIN(MSG_INIT, SVC_STUFFTEXT);
	WRITE_STRING("VTC_CheckEnd\n");
	MESSAGE_END();

	VTC_ExecConfig();
	VTC_InitCodecs();

	RETURN_META(MRES_IGNORED);
}

// MetaMod API
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION,							// ifvers
	"VoiceTranscoder",								// name
	"2.0 Reloaded",									// version
	"Dec 21 2014",									// date
	"[WPMG]PRoSToTeM@ <wpmgprostotema@live.ru>",	// author
	"http://vtc.wpmg.ru/",							// url
	"VTC",											// logtag, all caps please
	PT_ANYTIME,										// (when) loadable
	PT_ANYTIME,										// (when) unloadable
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(char *pchInterfaceVersion, plugin_info_t **pPluginInfo, mutil_funcs_t *pMetaUtilFuncs) {
	*pPluginInfo = &Plugin_info;
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return TRUE;
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) {
	// Reset
	memset(pFunctionTable, 0, sizeof(*pFunctionTable));

	gpMetaGlobals = pMGlobals;
	gpGamedllFuncs = pGamedllFuncs;

	pFunctionTable->pfnGetEntityAPI2 = &GetEntityAPI2;
	pFunctionTable->pfnGetEntityAPI2_Post = &GetEntityAPI2_Post;

	VTC_InitCvars();
	VTC_InitConfig();
	VTC_ExecConfig();
	VTC_InitCodecs();

	EngineParser();

	return TRUE;
}

// AMXX API (TODO)

// Another code
size_t Engine_GetClientId(client_t *pClient) {
	return ((size_t)pClient - (size_t)g_psvs->m_pClients) / g_nClientStructSize + 1;
}

void VTC_ParseVoiceData(client_t *pClient) {
	// Voice disabled? WTF!?!?!?!?
	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}

	size_t nClientIndex = Engine_GetClientId(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex];

	// Spawn before speak =)
	if (!pClient->m_fActive) {
		return;
	}

	byte bRecvd[c_nMaxVoicePacketSize];
	size_t nDataSize = g_pNetMessage->ReadWord();

	if (nDataSize > sizeof(bRecvd)) {
		LOG_MESSAGE(PLID, "Oversize voice packet, size = %u (max = %u)", nDataSize, c_nMaxVoicePacketSize);

		(*g_pfnSvDropClient)(pClient, false, "Oversize voice packet, size = %u (max = %u)", nDataSize, c_nMaxVoicePacketSize);

		return;
	}

	/* There must be checks for flooding */

	voicebuf_t *pVoiceBuf = new voicebuf_t;
	pVoiceBuf->m_nSize = nDataSize;
	pVoiceBuf->m_nPlayerIndex = nClientIndex;
	pVoiceBuf->m_fIsNewCodec = pClientData->m_fHasNewCodec;
	pVoiceBuf->m_pBuf = new byte[nDataSize];
	pVoiceBuf->m_fCompleted = false;
	pVoiceBuf->m_pNext = NULL;
	g_pNetMessage->ReadMem(pVoiceBuf->m_pBuf, nDataSize);

	if (g_pVoiceBufQueue.m_pFirstUnworked != NULL) {
		g_pVoiceBufQueue.m_pLast->m_pNext = pVoiceBuf;
		g_pVoiceBufQueue.m_pLast = pVoiceBuf;
	}

	if (g_pVoiceBufQueue.m_pFirstUnworked == NULL) {
		if (!pVoiceBuf->m_fCompleted) {
			if (g_pVoiceBufQueue.m_pFirst == NULL) {
				g_pVoiceBufQueue.m_pFirst = pVoiceBuf;
			}

			g_pVoiceBufQueue.m_pFirstUnworked = g_pVoiceBufQueue.m_pLast = pVoiceBuf;
		}
	}

	// Wake
	SetEvent(g_hEvent);
}

/* Thanks list:
Lev - help with thread
Crock - help with thread
*/

void InitThread() {
	// Create
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void ThreadTranscoding(void) {
	voicebuf_t *pVoiceBuf;

	while (true) {
		if (g_pVoiceBufQueue.m_pFirstUnworked == NULL) {
			ResetEvent(g_hEvent);
			if (g_pVoiceBufQueue.m_pFirstUnworked == NULL) {
				// Try to sleep
				WaitForSingleObject(g_hEvent, INFINITE);

				continue;
			}
		}

		pVoiceBuf = g_pVoiceBufQueue.m_pFirstUnworked;
		
		// Work with it
		// Some code
		// End
		g_pVoiceBufQueue.m_pFirstUnworked = pVoiceBuf->m_pNext;
		pVoiceBuf->m_fCompleted = true;
	}
}

void VoiceFlusher(void) {
	voicebuf_t *pVoiceBuf = g_pVoiceBufQueue.m_pFirst;

	while (pVoiceBuf != NULL && pVoiceBuf->m_fCompleted) {
		g_pVoiceBufQueue.m_pFirst = pVoiceBuf->m_pNext;

		delete pVoiceBuf;

		if (pVoiceBuf == g_pVoiceBufQueue.m_pLast) {
			g_pVoiceBufQueue.m_pLast = NULL;

			break;
		}

		pVoiceBuf = g_pVoiceBufQueue.m_pFirst;
	}
}

void VTC_InitCodecs(void) {

}

void VTC_InitCvars(void) {
	CVAR_REGISTER(&g_cvarVersion);
	g_pcvarVersion = CVAR_GET_POINTER(g_cvarVersion.name);
	CVAR_REGISTER(&g_cvarDefaultCodec);
	g_pcvarDefaultCodec = CVAR_GET_POINTER(g_cvarDefaultCodec.name);
	CVAR_REGISTER(&g_cvarHltvCodec);
	g_pcvarHltvCodec = CVAR_GET_POINTER(g_cvarHltvCodec.name);
	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");
}

void VTC_InitConfig(void) {
	char szRelPath[260];
	MetaUTIL_GetPluginRelPath(szRelPath, sizeof(szRelPath));
	snprintf(g_szExecConfigCmd, sizeof(g_szExecConfigCmd), "exec \"%s%s\"\n", szRelPath, c_szConfigName);
}

void VTC_ExecConfig(void) {
	SERVER_COMMAND(g_szExecConfigCmd);
	SERVER_EXECUTE();
}

size_t MetaUTIL_GetPluginRelPath(char *szPath, size_t nMaxPathLen)
{
	const char *pszPluginAbsPath = GET_PLUGIN_PATH(PLID);
	const char *pszGamedirAbsPath = GET_GAME_INFO(PLID, GINFO_GAMEDIR);
	const char *pszPluginRelPath = &pszPluginAbsPath[strlen(pszGamedirAbsPath) + 1]; // + slash

	// Find last slash
	const char *pszSearchChar = pszPluginRelPath + strlen(pszPluginRelPath) - 1; // go to string end

	while (true) {
		if (*pszSearchChar == '/' || *pszSearchChar == '\\') {
			break; // ok we found it
		}

		pszSearchChar--;
	}

	size_t nCharsToCopy = min((size_t)pszSearchChar - (size_t)pszPluginRelPath + 1, nMaxPathLen); // with slash
	strncpy(szPath, pszPluginRelPath, nCharsToCopy);
	szPath[nCharsToCopy] = '\0';

	return nCharsToCopy;
}

/*
How we can search in windows dlls ?
1) We can search in code
2) We can search in data
3) We can search in relocs
Ok
What we can search ?
1) We can search strings in code (or another usage data in code):
- find it in data
- go search in relocs (if we have it) or code
- PROFIT!
2) We can search code signatures
- make signature and mask (or masked signature WTF?!?!?!)
- find it in code (yeah we can do this forward and backward)
- PROFIT!!!
3) We can search minicode signatures from some position ... (see 2nd point)
AND NOW!!!
4) We can search function calls!!!
- first of all we need address of needed function
- go search in relocs (if we have it) or code
- PROFIT!!!!!!!!!!!!!!!!!!!
5) Also we can search in import table (and also export)
TODO:
some remarks
*/

/*
 django, konstantin, hobbit, matrix*/

struct section_t {
	dword dwStart, dwEnd;
};

struct sectionList_t {
	section_t section;
	section_t *pNext;
};

struct dataSection_t {
	dword dwStart;
	dword dwVirtualEnd;
	dword dwRawEnd;
};

struct dll_t {
	dword m_dwBase;
	section_t m_code;

	// Only win32
	dword *m_pRelocs;
	dword m_nRelocCount;
	bool m_fRelocProvided;

	dataSection_t m_data[2];
	size_t m_nDataSections;
};

dll_t g_engine;



bool EngineParser(void) {
	// WIN32
	HMODULE hEngine = GetModuleHandle(L"swds.dll");
	dword dwEngineBase = (dword)hEngine;
	g_engine.m_dwBase = dwEngineBase;

	PIMAGE_DOS_HEADER pHeader = (PIMAGE_DOS_HEADER)hEngine;

	if (pHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		LOG_ERROR(PLID, "Bad engine signature");

		return false;
	}

	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(dwEngineBase + pHeader->e_lfanew);

	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
		LOG_ERROR(PLID, "Bad engine signature 2");

		return false;
	}

	// We need code, data, reloc and (import) sections
	// .text - code section
	// .data, .rdata - data sections
	// .reloc - relocation section
	PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((dword)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader);

	LOG_MESSAGE(PLID, "Code section start-end: %.8X-%.8X %.8X %.8X", pNtHeaders->OptionalHeader.BaseOfData, pNtHeaders->OptionalHeader.BaseOfData + pNtHeaders->OptionalHeader.SizeOfInitializedData + pNtHeaders->OptionalHeader.SizeOfUninitializedData, pNtHeaders->OptionalHeader.SizeOfInitializedData, pNtHeaders->OptionalHeader.SizeOfUninitializedData);

	g_engine.m_nDataSections = 0;

	for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++, pSection++) {
		LOG_MESSAGE(PLID, "Section %d", i + 1);

		if (pSection->VirtualAddress == pNtHeaders->OptionalHeader.BaseOfCode) {
			LOG_MESSAGE(PLID, "This is code section");
		}
		if (pSection->VirtualAddress >= pNtHeaders->OptionalHeader.BaseOfData) {
			LOG_MESSAGE(PLID, "This is data section");
		}

		LOG_MESSAGE(PLID, "Characteristics: %.8X", pSection->Characteristics);
		LOG_MESSAGE(PLID, "Misc: %.8X", pSection->Misc.PhysicalAddress);
		LOG_MESSAGE(PLID, "Name: %s", pSection->Name);
		LOG_MESSAGE(PLID, "NumberOfLinenumbers: %d", pSection->NumberOfLinenumbers);
		LOG_MESSAGE(PLID, "NumberOfRelocations: %d", pSection->NumberOfRelocations);
		LOG_MESSAGE(PLID, "PointerToLinenumbers: %.8X", pSection->PointerToLinenumbers);
		LOG_MESSAGE(PLID, "PointerToRelocations: %.8X", pSection->PointerToRelocations);
		LOG_MESSAGE(PLID, "SizeOfRawData: %.8X", pSection->SizeOfRawData);
		LOG_MESSAGE(PLID, "VirtualAddress: %.8X", pSection->VirtualAddress);

		// .text\0
		if (*(dword *)pSection->Name == 'xet.' && *(word *)&pSection->Name[4] == '\0t') {
			g_engine.m_code.dwStart = g_engine.m_dwBase + pSection->VirtualAddress;
			g_engine.m_code.dwEnd = g_engine.m_code.dwStart + pSection->Misc.VirtualSize;

			LOG_MESSAGE(PLID, "THIS IS CODE SECTION: start %.8X end %.8X", g_engine.m_code.dwStart, g_engine.m_code.dwEnd);
		} /* .data\0 || .rdata\0 */ else if (*(dword *)pSection->Name == 'tad.' && *(word *)&pSection->Name[4] == '\0a' || *(dword *)pSection->Name == 'adr.' && *(word *)&pSection->Name[4] == 'at' && pSection->Name[6] == '\0') {
			g_engine.m_data[g_engine.m_nDataSections].dwStart = pSection->VirtualAddress;
			g_engine.m_data[g_engine.m_nDataSections].dwRawEnd = pSection->VirtualAddress + pSection->SizeOfRawData;
			g_engine.m_data[g_engine.m_nDataSections].dwVirtualEnd = pSection->VirtualAddress + pSection->Misc.VirtualSize;
			
			LOG_MESSAGE(PLID, "THIS IS DATA SECTION");
		} /* .reloc\0 */ else if (*(dword *)pSection->Name == 'ler.' && *(word *)&pSection->Name[4] == 'co' && pSection->Name[6] == '\0') {
			PIMAGE_BASE_RELOCATION pBaseReloc = (PIMAGE_BASE_RELOCATION)(dwEngineBase + pSection->VirtualAddress);

			// upper bound size
			dword *pRelocations = (dword *)malloc(pSection->Misc.VirtualSize / sizeof(word) * sizeof(dword));

			if (!pRelocations) {
				LOG_ERROR(PLID, "Can't allocate memory for relocations");

				return false;
			}

			pBaseReloc = (PIMAGE_BASE_RELOCATION)(dwEngineBase + pSection->VirtualAddress);
			dword dwTotalSize = 0;
			PWORD pReloc, pEnd;
			dword dwBase;
			dword dwReloc = 0;
			while (pBaseReloc->SizeOfBlock) {
				pEnd = (PWORD)((dword)pBaseReloc + pBaseReloc->SizeOfBlock);

				dwBase = dwEngineBase + pBaseReloc->VirtualAddress;

				for (pReloc = (PWORD)(pBaseReloc + 1); pReloc < pEnd; pReloc++) {
					if (((*pReloc) >> 12) == 0) {
						continue;
					}

					dwTotalSize += 4;

					pRelocations[dwReloc++] = ((*pReloc) & 0xFFF) + dwBase;
				}

				pBaseReloc = (PIMAGE_BASE_RELOCATION)((dword)pBaseReloc + pBaseReloc->SizeOfBlock);
			}

			// real size
			pRelocations = (dword *)realloc(pRelocations, dwTotalSize);

			if (pRelocations == NULL) {
				LOG_ERROR(PLID, "Can't reallocate memory for relocations");

				return false;
			}
		}
	}

	// We have:
	// 1 code section, always 1
	// C (A+B) data sections:
	// A RData
	// B VData
	// Reloc
}