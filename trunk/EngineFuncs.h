#pragma once

#include "Util.h"
#include "DProtoAPI.h"

#define CLC_VOICEDATA	8

#define SVC_VOICEDATA	53

typedef void * (* PSZ_GETSPACE)(sizebuf_t *, size_t);
typedef void (* PCLCFUNCS_CALLBACK)(client_t *);

struct sv_clcfuncs_t {
	unsigned int iMsgId;
	const char *pszMsgName;
	void (* pfnCallback)(client_t *pClient);
};

extern PCLCFUNCS_CALLBACK g_pfnSVParseVoiceData;
extern PSZ_GETSPACE g_pfnSZGetSpace;
extern size_t g_sizeClientStruct;
extern server_static_t *g_psvs;
extern size_t *g_pMsgReadcount;
extern bool *g_pMsgBadread;
extern sizebuf_t *g_pNetMessage;

#define msg_readcount (*g_pMsgReadcount)
#define msg_badread (*g_pMsgBadread)
#define net_message (*g_pNetMessage)

extern void SV_ParseVoiceData( client_t *pClient );
template <typename TypeName> TypeName * SZ_GetSpace(sizebuf_t *pNetchan) {
		return (TypeName *)g_pfnSZGetSpace(pNetchan, sizeof(TypeName));
}
extern void * SZ_GetSpace(sizebuf_t *pDatagram, size_t size);
extern short MSG_ReadShort(void);
extern int MSG_ReadBuf(size_t size, void *pDest);
extern void MSG_WriteByte(sizebuf_t *pDatagram, unsigned char ucVal);
extern void MSG_WriteShort(sizebuf_t *pDatagram, short sVal);
extern void MSG_WriteBuf(sizebuf_t *pDatagram, size_t size, void *pBuf);