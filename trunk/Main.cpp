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
#include "MultiThread.h"
#include "NetMsgBuf.h"
#include "Util.h"
#include "Timer.h"
#include "CRC32.h"
#include "Buffer.h"
#include "ThreadMode.h"
#include "Main.h"
#include "MetaUTIL.h"
#include "EngineUTIL.h"
#include "SteamID.h"
#include "Library.h"

#ifdef WIN32
	#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#endif

// Constants
const size_t MAX_CLIENTS = 32;
const size_t SVC_STUFFTEXT = 9;
const size_t MAX_VOICEPACKET_SIZE = 2048;
const size_t MAX_DECOMPRESSED_VOICEPACKET_SIZE = 4096;
const char *VTC_CONFIGNAME = "vtc.cfg";
// VoicePacketCommand
// Now work only raw and silk
enum : size_t {
	VPC_VDATA_RAW = 0,
	VPC_VDATA_SPEEX = 1, // Deprecated
	VPC_VDATA_UNKNOWN = 2, // Deprecated
	VPC_VDATA_RAW2 = 3,
	VPC_VDATA_SILK = 4,
	VPC_SETSAMPLERATE = 11,
};
// qword (steamid) + byte (VPC_SETSAMPLERATE) + word (samplerate) + byte (VPC_VDATA_SILK/RAW) + word (vdatasize) + ... + dword (crc32 checksum)
const size_t MIN_VOICEPACKET_SIZE = sizeof(qword) + sizeof(byte) + sizeof(word) + sizeof(byte) + sizeof(word) + sizeof(dword);
const size_t WANTED_SAMPLERATE = 16000;
#define SV_DropClient	(*g_pfnSvDropClient)
#define svs				(*g_psvs)
#define msg_readcount	(*g_pnMsgReadCount)
#define msg_badread		(*g_pfMsgBadRead)
#define net_message		(*g_pNetMessage)

// Externs
extern size_t *g_pnMsgReadCount;
extern bool *g_pfMsgBadRead;

// Variables
clientData_t g_rgClientData[MAX_CLIENTS + 1];

char g_szExecConfigCmd[300];

bool g_fThreadModeEnabled;

// Hacked
Library *g_pEngine;
size_t g_nClientStructSize;
server_static_t *g_psvs;
size_t *g_pnMsgReadCount;
bool *g_pfMsgBadRead;
NetMsgBuf *g_pNetMessage;
SV_DROPCLIENT *g_pfnSvDropClient;
List<Hook_Call> g_phooksSvParseVoiceData;

// Cvars
cvar_t g_cvarVersion = {"VTC_Version", Plugin_info.version, FCVAR_EXTDLL | FCVAR_SERVER, 0, NULL};
cvar_t *g_pcvarVersion;
cvar_t g_cvarDefaultCodec = {"VTC_DefaultCodec", "old", FCVAR_EXTDLL, 0, NULL};
cvar_t *g_pcvarDefaultCodec;
cvar_t g_cvarHltvCodec = {"VTC_HltvCodec", "old", FCVAR_EXTDLL, 0, NULL};
cvar_t *g_pcvarHltvCodec;
cvar_t g_cvarThreadMode = {"VTC_ThreadMode", "0", FCVAR_EXTDLL, 0, NULL};
cvar_t *g_pcvarThreadMode;
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
	if (g_pcvarThreadMode->value != 0.0f) {
		g_fThreadModeEnabled = true;

		VTC_ThreadInit();
	}

	EngineParser();

	return TRUE;
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
	if (g_fThreadModeEnabled) {
		VTC_ThreadDeinit();
	}

	return TRUE;
}

// AMXX API (TODO)

// Another code
/* forcehltv
 * forcesendvoicecodec*/
void VTC_ParseVoiceData(client_t *pClient) {
	// Voice disabled? WTF!?!?!?!?
	if (g_pcvarVoiceEnable->value == 0.0f) {
		return;
	}
	// Spawn before speak =)
	if (!pClient->m_fActive) {
		return;
	}

	size_t nClientIndex = EngineUTIL::GetClientId(pClient);
	clientData_t *pClientData = &g_rgClientData[nClientIndex];

	// Fucking flooder
	longlong llCurTime = GetCurTime();

	// Try to flood ?
	if (llCurTime < pClientData->m_llNextPacketUsec) {
		return;
	}

	size_t nDataSize = g_pNetMessage->ReadWord();

	// Check oversize
	if (nDataSize > MAX_VOICEPACKET_SIZE) {
		LOG_MESSAGE(PLID, "Oversize voice packet, size = %u (max = %u) from %s", nDataSize, MAX_VOICEPACKET_SIZE, pClient->m_szPlayerName);
		SV_DropClient(pClient, false, "Oversize voice packet, size = %u (max = %u)", nDataSize, MAX_VOICEPACKET_SIZE);

		return;
	}

	byte bRecvd[MAX_VOICEPACKET_SIZE];
	void *pRecvd = bRecvd;
	g_pNetMessage->ReadBuf(bRecvd, nDataSize);

	short bDecompressed[MAX_DECOMPRESSED_VOICEPACKET_SIZE];
	size_t nDecompressedSize;

	// Validate new codec packet
	if (pClientData->m_fHasNewCodec) {
		if (nDataSize < MIN_VOICEPACKET_SIZE) {
			LOG_MESSAGE(PLID, "Too small voice packet (real = %u, min = %u) from %s", nDataSize, MIN_VOICEPACKET_SIZE, pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Too small voice packet (real = %u, min = %u)", nDataSize, MIN_VOICEPACKET_SIZE);

			return;
		}

		Buffer buf(bRecvd, nDataSize);

		CRC32 checksum;
		checksum.Init();
		checksum.Update(buf.PeekRead(), buf.Size() - sizeof(dword));
		checksum.Final();

		buf.SeekRead(sizeof(dword), SEEK_END);
		dword dwRecvdChecksum = buf.ReadDWord();

		if (checksum.ConvertToDWord() != dwRecvdChecksum) {
			LOG_MESSAGE(PLID, "Voice packet checksum validation failed for %s", pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Voice packet checksum validation failed");

			return;
		}

		// Go to start
		buf.RewindRead();

		// SteamID
		qword qwSteamID = buf.ReadQWord();

		// Validate SteamID
		SteamID steamid(qwSteamID);
		if (!steamid.IsValid()) {
			LOG_MESSAGE(PLID, "Invalid steamid (%llu) in voice packet from %s", steamid.ConvertToQWord(), pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Invalid steamid (%llu) in voice packet", steamid.ConvertToQWord());

			return;
		}

		// Next should be setupdaterate
		byte bCmd = buf.ReadByte();

		if (bCmd != VPC_SETSAMPLERATE) {
			LOG_MESSAGE(PLID, "Voice packet unwanted command (cur = %u, want = %u) for %s", bCmd, VPC_SETSAMPLERATE, pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Voice packet unwanted command (cur = %u, want = %u)", bCmd, VPC_SETSAMPLERATE);

			return;
		}

		// Read samplerate
		word wSampleRate = buf.ReadWord();

		if (wSampleRate != WANTED_SAMPLERATE) {
			LOG_MESSAGE(PLID, "Voice packet unwanted samplerate (cur = %u, want = %u) for %s", wSampleRate, WANTED_SAMPLERATE, pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Voice packet unwanted samplerate (cur = %u, want = %u)", wSampleRate, WANTED_SAMPLERATE);

			return;
		}

		// Read last cmd should be vdata_silk
		bCmd = buf.ReadByte();

		if (bCmd != VPC_VDATA_SILK) {
			LOG_MESSAGE(PLID, "Voice packet unwanted vdatacodec (cur = %u, want = %u) for %s", bCmd, VPC_VDATA_SILK, pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Voice packet unwanted vdatacodec (cur = %u, want = %u)", bCmd, VPC_VDATA_SILK);

			return;
		}

		// Read length now
		nDataSize = buf.ReadWord();

		size_t nWantedDataSize = buf.Size() - buf.TellRead() - sizeof(dword);
		if (nDataSize != nWantedDataSize) {
			LOG_MESSAGE(PLID, "Voice packet unwanted vdata size (cur = %u, want = %u) for %s", nDataSize, nWantedDataSize, pClient->m_szPlayerName);
			SV_DropClient(pClient, false, "Voice packet unwanted vdata size (cur = %u, want = %u)", nDataSize, nWantedDataSize);

			return;
		}

		// Move recvd pointer
		pRecvd = buf.PeekRead();

		nDecompressedSize = pClientData->m_pNewCodec->Decode(pRecvd, nDataSize, bDecompressed, ARRAYSIZE(bDecompressed));
		// pClientData->m_pSilkCodec->Decompress();
	} else {
		nDecompressedSize = pClientData->m_pOldCodec->Decode(pRecvd, nDataSize, bDecompressed, ARRAYSIZE(bDecompressed));
	}

	// Length
	pClientData->m_llNextPacketUsec = llCurTime + (nDecompressedSize * 1000000) / 8000;

	// Ok only thread
	if (g_fThreadModeEnabled) {
		// TODO
		VTC_ThreadAddVoicePacket(pClient, nClientIndex, pClientData, bDecompressed, nDataSize);

		return;
	}

	// After some manipulations...
	// Non-thread
	byte bRecompressed[MAX_VOICEPACKET_SIZE];
	size_t nRecompressedSize;
	if (pClientData->m_fHasNewCodec) {
		nRecompressedSize = pClientData->m_pOldCodec->Encode(bDecompressed, nDecompressedSize, bRecompressed, ARRAYSIZE(bRecompressed));
	} else {
		nRecompressedSize = pClientData->m_pNewCodec->Encode(bDecompressed, nDecompressedSize, bRecompressed, ARRAYSIZE(bRecompressed));
	}

	
}

/* Thanks list:
Lev - help with thread
Crock - help with thread
*/

void VTC_InitCodecs(void) {

}

void VTC_InitCvars(void) {
	CVAR_REGISTER(&g_cvarVersion);
	g_pcvarVersion = CVAR_GET_POINTER(g_cvarVersion.name);
	CVAR_REGISTER(&g_cvarDefaultCodec);
	g_pcvarDefaultCodec = CVAR_GET_POINTER(g_cvarDefaultCodec.name);
	CVAR_REGISTER(&g_cvarHltvCodec);
	g_pcvarHltvCodec = CVAR_GET_POINTER(g_cvarHltvCodec.name);
	CVAR_REGISTER(&g_cvarThreadMode);
	g_pcvarThreadMode = CVAR_GET_POINTER(g_cvarThreadMode.name);
	g_pcvarVoiceEnable = CVAR_GET_POINTER("sv_voiceenable");
}

void VTC_InitConfig(void) {
	char szRelPath[260];
	MetaUTIL::GetPluginRelPath(szRelPath, sizeof(szRelPath));
	snprintf(g_szExecConfigCmd, sizeof(g_szExecConfigCmd), "exec \"%s%s\"\n", szRelPath, VTC_CONFIGNAME);
}

void VTC_ExecConfig(void) {
	SERVER_COMMAND(g_szExecConfigCmd);
	SERVER_EXECUTE();
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

// Engine searches, patches and hooks...
/*
class Memory {
public:
	template <typename T>
	Memory(T dwAddr, size_t nSize) : m_dwAddr((dword)dwAddr), m_nSize(nSize) {}
	void Unprotect();
	void Reprotect();
private:
	dword m_dwAddr;
	size_t m_nSize;
#ifdef WIN32
	dword m_dwOldProtect;
#endif
};

void Memory::Unprotect() {
#ifdef WIN32
	VirtualProtect((LPVOID)m_dwAddr, m_nSize, PAGE_EXECUTE_READWRITE, &m_dwOldProtect);
#else
	dword dwAlignedAddr = m_dwAddr & ~(PAGESIZE - 1);
	dword dwAlignedEndAddr = ((m_dwAddr + m_nSize) | (PAGESIZE - 1)) + 1;
	mprotect((void *)dwAlignedAddr, dwAlignedEndAddr - dwAlignedAddr, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
}

void Memory::Reprotect() {
#ifdef WIN32
	VirtualProtect((LPVOID)m_dwAddr, m_nSize, m_dwOldProtect, &m_dwOldProtect);
#endif
}

class Template {
public:
	Template(const char *pszBytes, const char *pszMask, size_t nBytes) : m_pBytes((const byte *)pszBytes), m_pMask((const byte *)pszMask), m_nBytes(nBytes) {}
	bool IsEqual(dword dwBytes);
private:
	friend class Library;

	const byte *m_pBytes;
	const byte *m_pMask;
	size_t m_nBytes;
};

bool Template::IsEqual(dword dwBytes) {
	byte *pBytes = (byte *)dwBytes;

	for (size_t n = 0; n < m_nBytes; n++) {
		if ((pBytes[n] & m_pMask[n]) != (m_pBytes[n] & m_pMask[n])) {
			return false;
		}
	}

	return true;
}

class Section {
public:
	Section() {}
	Section(dword dwStart, dword dwSize) : m_dwStart(dwStart), m_dwEnd(dwStart + dwSize) {}
	void Set(dword dwStart, dword dwEnd) {
		m_dwStart = dwStart;
		m_dwEnd = dwEnd;
	}
	dword FindString(const char *pszString);
	Section *Next() { return m_pNext; }
	dword Start() { return m_dwStart; }
	dword End() { return m_dwEnd; }
	void SetNext(Section *pSection) { m_pNext = pSection; }
private:
	dword m_dwStart;
	dword m_dwEnd;
	Section *m_pNext;
};

dword Section::FindString(const char *pszString) {
	dword dwCur = m_dwStart;
	dword dwEnd = m_dwEnd - strlen(pszString) - 1;

	while (dwCur <= dwEnd) {
		if (!strcmp((char *)dwCur, pszString)) {
			return dwCur;
		}

		dwCur++;
	}

	return 0;
}

class Library {
public:
	Library(dword dwAddr);
	Library(const char *pszName);
	void *Library::FindSymbol(const char *pszSymbol);
	void Library::FindSymbol(void *p, const char *pszSymbol);
	void *FindFunctionByString(const char *pszString);
	void FindFunctionByString(void *p, const char *pszString);
	void HookFunctionCalls(void *pAddr, void *pCallback);
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
dword Library::SearchForTemplate(Template tpl, T startAddr, size_t nSize, size_t nAddOffset / * = 0 * /) {
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
dword Library::SearchForTemplateBackward(Template tpl, T startAddr, size_t nSize, size_t nAddOffset / * = 0 * /) {
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

void Library::HookFunctionCalls(void *pAddr, void *pCallback) {
	dword dwCur = m_code.Start();
	dword dwEnd = m_code.End() - sizeof(byte) - sizeof(dword);

	while (dwCur <= dwEnd) {
		if (*(byte *)dwCur == 0xE8 && *(long *)(dwCur + 1) == ((long)pAddr - (long)dwCur - sizeof(byte) - sizeof(dword))) {
			Memory mem(dwCur + 1, sizeof(dword));
			mem.Unprotect();
			*(long *)(dwCur + 1) = (long)pCallback - (long)dwCur - sizeof(byte) - sizeof(dword);
			mem.Reprotect();
		}

		dwCur++;
	}
}

Library *g_pEngine;

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

#ifdef WIN32
// Ok i will search...
void *Find_InfoRemovePrefixedKeys() {
	dword dwStr = g_pEngine->FindStringUsing("Force Update");

	dword dwCallToSvFullClientUpdate = g_pEngine->SearchForTemplate(Template("\xE8\x00\x00\x00\x00\x83\xC4", "\xFF\x00\x00\x00\x00\xFF\xFF", 7), dwStr, 0x80);
	dword dwSvFullClientUpdateAddr = (long)dwCallToSvFullClientUpdate + *(long *)(dwCallToSvFullClientUpdate + 1) + sizeof(dword) + sizeof(byte);

	dword dwCallToInfoRemovePrefixedKeys = g_pEngine->SearchForTemplate(Template("\x6A\x5F\x00\x00\x00\x00\x00\xE8", "\xFF\xFF\x00\x00\x00\x00\x00\xFF", 8), dwSvFullClientUpdateAddr, 0x80, 7);

	return (void *)((long)dwCallToInfoRemovePrefixedKeys + *(long *)(dwCallToInfoRemovePrefixedKeys + 1) + sizeof(dword) + sizeof(byte));
}
#endif*/

/*What we need:
svs
sizeof(client_t)
SV_DropClient ?
msg_readcount
msg_badread
net_message
SZ_GetSpace
SV_ParseVoiceData*/
// Init base?
void Hacks_Init() {
	void *pfnSvParseVoiceData;

#ifdef WIN32
	g_pEngine = new Library("swds.dll");
	g_pEngine->FindFunctionByString(&g_pfnSvDropClient, "Dropped %s from server\nReason:  %s\n");
	pfnSvParseVoiceData = g_pEngine->FindFunctionByString("SV_ParseVoiceData: invalid incoming packet.\n");
#else
	g_pEngine = new Library((dword)g_engfuncs.pfnPrecacheModel);
	g_pEngine->FindSymbol(&g_psvs, "svs");
	g_pEngine->FindSymbol(&g_pfnSvDropClient, "SV_DropClient");
	g_pEngine->FindSymbol(&g_pnMsgReadCount, "msg_readcount");
	g_pEngine->FindSymbol(&g_pfMsgBadRead, "msg_badread");
	g_pEngine->FindSymbol(&g_pNetMessage, "net_message");
	g_pEngine->FindSymbol(&g_pfnSzGetSpace, "SZ_GetSpace");
	pfnSvParseVoiceData = g_pEngine->FindSymbol("SV_ParseVoiceData");
#endif

	//g_pEngine->HookFunctionCalls(pfnInfoRemovePrefixedKeys, &Info_RemovePrefixedKeys_Pre);
	//g_phookSvParseVoiceData = g_pEngine->HookFunction(pfnSvParseVoiceData, &SV_ParseVoiceData);
}

void Hacks_Deinit() {
	// if (g_phookSvParseVoiceData != NULL) {
	//	g_phookSvParseVoiceData->Unhook();
	// }
	delete g_pEngine;
}