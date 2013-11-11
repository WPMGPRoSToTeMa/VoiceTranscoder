#pragma once

#include "VoiceCodec.h"

enum vcodec_t {
	VCODEC_NONE = 0,
	VCODEC_SPEEX,
	VCODEC_SILK
};

class CPlayer {
public:
	vcodec_t		m_vcodec;
	bool			m_fRequested;
	int				m_iRequestID;
	CVoiceCodec	*	m_pEncoder;
	CVoiceCodec *	m_pDecoder;
	double			m_dbLastReceivedVoice;

	void Connect();
	void CvarValue();
	void ReceivedVoice();
};