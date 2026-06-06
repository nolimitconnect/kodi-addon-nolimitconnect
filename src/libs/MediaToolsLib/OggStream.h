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

#include "OggBuffer.h"

#include <libogg/include/ogg/ogg.h>

#include <CoreLib/VxDefs.h>

#include <string>
#include <stdio.h>

class MyOpusHeader;
class VFile;

class OggStream
{
public:
	OggStream();
	~OggStream();

	void						setTitle( const char* title );
	void						setArtist( const char* artist );
	void						setAlbum( const char* album );
	void						setDate( const char* date );
	void						setGenre( const char* genre );
	void						setUserComment( const char* userComment );

	bool						openOggStream( VFile * fileHandle, int streamSerialNumber = 0 ); // if streamSerialNumber is zero then generate random serial number
	int							writeHeader( MyOpusHeader& opusHeader, uint8_t * packetBuf, int packetBufLen );
	int							writeEncodedFrame( uint8_t * encodedFrame, int32_t encodedLen );
	uint64_t					closeOggStream( void ); // returns total bytes written

	void						setOpusHeader( MyOpusHeader& opusHeader )	{ m_OpusHeader = &opusHeader; }

protected:
	int							flushStreamToFile( void );
	int							writePageToOutput( ogg_page * page );

	bool						m_StreamInitialized{ false };
	VFile*						m_FileHandle{ nullptr };
	OggBuffer					m_OggBuffer; 
	ogg_stream_state			m_StreamState;		// os
	ogg_packet					m_OggPkt;			// op;
	ogg_page					m_OggPage;			// og
	ogg_int64_t					m_LastGranulePos{ 0 };	// ogg_int64_t last_granulepos=0;
	ogg_int64_t					m_EncGranulePos{ 0 };
	ogg_int64_t					m_OriginalSamples{ 0 };
	ogg_int32_t					m_id{ 0 };
	int							m_LastSegments{ 0 };
	int							m_Eos{ 0 };
	uint64_t					m_TotalBytesWritten{ 0 };
	int							m_PagesOut{ 0 };
	std::string					m_Title;
	std::string					m_Artist;
	std::string					m_Album;
	std::string					m_Date;
	std::string					m_Genre;
	std::string					m_UserTag;
	int							m_LastSizeSegments{ 0 };
	int							m_MaxOggDelay{ 48000 };
	MyOpusHeader *				m_OpusHeader{ nullptr };

};