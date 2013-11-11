#include "VoiceCodec.h"
#include "VoiceEncoder_Speex.h"
#include <stdio.h>

#define SAMPLERATE			8000
#define RAW_FRAME_SIZE		160

const int ENCODED_FRAME_SIZE [11] = {6,6,15,15,20,20,28,28,38,38,38};	

extern IVoiceCodec* CreateVoiceCodec_Frame(IFrameEncoder *pEncoder);

IVoiceCodec* CreateSpeexVoiceCodec()
{
	IFrameEncoder *pEncoder = new VoiceEncoder_Speex;
	return CreateVoiceCodec_Frame( pEncoder );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VoiceEncoder_Speex::VoiceEncoder_Speex()
{
	m_EncoderState = NULL;
	m_DecoderState = NULL;
	m_Quality = 0;
}

VoiceEncoder_Speex::~VoiceEncoder_Speex()
{
	TermStates();
}

bool VoiceEncoder_Speex::Init(int quality, int &rawFrameSize, int &encodedFrameSize)
{
	if ( !InitStates() )
		return false;

	rawFrameSize = RAW_FRAME_SIZE * BYTES_PER_SAMPLE;

	// map gerneral voice quality 1-5 to speex quality levels
	switch ( quality )
	{
		case 1 :	m_Quality = 0; break;
		case 2 :	m_Quality = 2; break;
		case 3 :	m_Quality = 4; break;
		case 4 :	m_Quality = 6; break;
		case 5 : 	m_Quality = 8; break;
		default	:	m_Quality = 0; break;
	}

	encodedFrameSize = ENCODED_FRAME_SIZE[m_Quality];

	speex_encoder_ctl( m_EncoderState, SPEEX_SET_QUALITY, &m_Quality);
	//speex_decoder_ctl( m_DecoderState, SPEEX_SET_QUALITY, &m_Quality);

	int postfilter = 1; // Set the perceptual enhancement on
	speex_decoder_ctl( m_DecoderState, SPEEX_SET_ENH, &postfilter);

	int samplerate = SAMPLERATE;
	speex_decoder_ctl( m_DecoderState, SPEEX_SET_SAMPLING_RATE, &samplerate );
	speex_encoder_ctl( m_EncoderState, SPEEX_SET_SAMPLING_RATE, &samplerate );

	return true;
}

void VoiceEncoder_Speex::Release()
{
	delete this;
}

void VoiceEncoder_Speex::EncodeFrame(const char *pUncompressedBytes, char *pCompressed)
{
	float input[RAW_FRAME_SIZE];
	short * in = (short*)pUncompressedBytes;

	/*Copy the 16 bits values to float so Speex can work on them*/
	for (int i=0;i<RAW_FRAME_SIZE;i++)
	{
		input[i]=(float)*in;
		in++;
	}

	/*Flush all the bits in the struct so we can encode a new frame*/
	speex_bits_reset( &m_Bits );

	/*Encode the frame*/
	speex_encode( m_EncoderState, input, &m_Bits );

	/*Copy the bits to an array of char that can be written*/
	int size = speex_bits_write(&m_Bits, pCompressed, ENCODED_FRAME_SIZE[m_Quality] );

	// char text[255];	_snprintf(text, 255, "outsize %i,", size ); OutputDebugStr( text );
}

void VoiceEncoder_Speex::DecodeFrame(const char *pCompressed, char *pDecompressedBytes)
{
	float output[RAW_FRAME_SIZE];
	short * out = (short*)pDecompressedBytes;

	/*Copy the data into the bit-stream struct*/
	speex_bits_read_from(&m_Bits, (char *)pCompressed, ENCODED_FRAME_SIZE[m_Quality] );

	/*Decode the data*/
	speex_decode(m_DecoderState, &m_Bits, output);
	
	/*Copy from float to short (16 bits) for output*/
	for (int i=0;i<RAW_FRAME_SIZE;i++)
	{
		*out = (short)output[i];
		out++;
	}
}

bool VoiceEncoder_Speex::ResetState()
{
	speex_encoder_ctl(m_EncoderState, SPEEX_RESET_STATE , NULL );
	speex_decoder_ctl(m_DecoderState, SPEEX_RESET_STATE , NULL );
	return true;
}

bool VoiceEncoder_Speex::InitStates()
{
	speex_bits_init(&m_Bits);
	
	m_EncoderState = speex_encoder_init( &speex_nb_mode );	// narrow band mode 8kbp

	m_DecoderState = speex_decoder_init( &speex_nb_mode );
	
	return m_EncoderState && m_DecoderState;
}

void VoiceEncoder_Speex::TermStates()
{
	if(m_EncoderState)
	{
		speex_encoder_destroy( m_EncoderState );
		m_EncoderState = NULL;
	}

	if(m_DecoderState)
	{
		speex_decoder_destroy( m_DecoderState );
		m_DecoderState = NULL;
	}

	speex_bits_destroy( &m_Bits );
}