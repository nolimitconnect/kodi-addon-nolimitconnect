#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <NlcDependLibrariesConfig.h>
#include "SndDefs.h"

#define	    NO_LIMIT_OPUS_SIGNITURE "nolimitconnect.org v"
#define	    NO_LIMIT_OPUS_SIGNITURE_LEN 20
#define	    NO_LIMIT_OPUS_SIGNITURE_OFFS 0x9c

class MyOpusHeader 
{
public:
	MyOpusHeader();

	uint8_t							m_Version{ 1 };
	uint8_t							m_Channels{ 1 };			// Number of channels: 1..255 
	uint16_t						m_Preskip{ 0 };			// calculated from look ahead and 
	uint32_t						m_InputSampleRate{ AUDIO_DEVICE_SAMPLE_RATE };	// rate in frequency
	uint16_t						m_Gain{ 0 };				// in dB S7.8 should be zero whenever possible 
	uint8_t							m_ChannelMapping{ 0 };
	// These used only used if channel_mapping != 0 
	uint8_t							m_StreamCnt{ 1 };
	uint8_t							m_CoupledCnt{ 0 };
	uint8_t							m_StreamMap[255];
	// Other required for ogg stream etc
	int32_t							m_MaxOpusPktSize{ (1275 * 3 + 7) * AUDIO_CHANNELS };
	int32_t							m_LookAhead{ 0 };			// from OPUS_GET_LOOKAHEAD
	int32_t							m_ExtraOut{ 0 };
	int32_t							m_OpusFrameRate{ OPUS_FRAME_RATE };
};
