#include "EngineFuncs.h"

PSZ_GETSPACE g_pfnSZGetSpace;
size_t *g_pMsgReadcount;
bool *g_pMsgBadread;
sizebuf_t *g_pNetMessage;

void * SZ_GetSpace(sizebuf_t *pNetchan, size_t size) {
	return g_pfnSZGetSpace(pNetchan, size);
}

short MSG_ReadShort(void) {
	short iResult;

	if ( msg_readcount + sizeof(short) > net_message.cursize )
	{
		msg_badread = true;

		iResult = -1;
	}
	else
	{
		iResult = *(short *)&(net_message.data[msg_readcount]);

		msg_readcount += sizeof(short);
	}

	return iResult;
}

int MSG_ReadBuf(size_t size, void *pDest) {
	int iResult;

	if ( msg_readcount + size > net_message.cursize )
	{
		msg_badread = true;

		iResult = -1;
	}
	else
	{
		memcpy(pDest, (void *)&net_message.data[msg_readcount], size);

		msg_readcount += size;

		iResult = 1;
	}

	return iResult;
}

void MSG_WriteByte(sizebuf_t *pDatagram, unsigned char ucVal) {
	unsigned char *pucData = SZ_GetSpace<unsigned char>(pDatagram);

	*pucData = ucVal;
}

void MSG_WriteShort(sizebuf_t *pDatagram, short sVal) {
	short *psData = SZ_GetSpace<short>(pDatagram);

	*psData = sVal;
}

void MSG_WriteBuf(sizebuf_t *pDatagram, size_t size, void *pBuf) {
	void *pData = SZ_GetSpace(pDatagram, size);

	memcpy(pData, pBuf, size);
}