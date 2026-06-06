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

#include "OpusCallbackInterface.h"
#include "MyOpusHeader.h"

#include <string>

class OpusCodec;
class OggStream;
class VFile;

class OpusFileEncoder
{
public:
	OpusFileEncoder( );
	virtual ~OpusFileEncoder();
 
	bool						beginFileEncode( const char* fileName, int sampleRate = AUDIO_DEVICE_SAMPLE_RATE, int channels = 1 );
	int							writeEncodedFrame( uint8_t * encodedFrameData, int32_t encodedLen );
	void						finishFileEncode( void );

protected:
	bool						createAudioEncoder( int sampleRate, int channels );
	int							encodePcmData( int16_t * pcmData, uint16_t pcmDataLen );
	bool						writeTotalSndFrames( VFile * fileHandle );

	//=== vars ===//
	OpusCodec *	     			m_OpusCodec{ nullptr };
	MyOpusHeader				m_OpusHeader;
	OggStream&					m_OggStream;
	std::string					m_FileName;
	VFile*						m_FileHandle{ nullptr };
	uint64_t					m_TotalSndFramesInFile;
	bool						m_EncoderInitialized;
};

