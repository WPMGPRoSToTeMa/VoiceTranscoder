#pragma once

#include "VoiceCodec.h"

#include "Util.h"

enum vcodec_t {
	VCODEC_NONE = 0,
	VCODEC_SPEEX,
	VCODEC_SILK
};

class CPlayer {
public:
	client_t *		m_pClient;
	vcodec_t		m_vcodec;
	bool			m_fRequested;
	int				m_iRequestID;
	CVoiceCodec	*	m_pEncoder;
	CVoiceCodec *	m_pDecoder;
	float			m_flLastReceivedVoice;

	void Connect();
	void CvarValue(int iRequestID, const char *pszCvar, const char *pszValue);
	void ReceivedVoice();
};