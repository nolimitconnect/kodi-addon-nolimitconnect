//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OggStream.h"
#include "SndDefs.h"
#include "MyOpusHeader.h"

#include <opus/include/opus_defines.h>

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTimeUtil.h>

#include <stdlib.h>
#include <memory.h>

//============================================================================
OggStream::OggStream()
{
}

//============================================================================
OggStream::~OggStream()
{
}

//============================================================================
void OggStream::setTitle( const char* title )
{
	m_Title = "title=";
	m_Title += title;
}

//============================================================================
void OggStream::setArtist( const char* artist )
{
	m_Artist = "artist=";
	m_Artist += artist;
}

//============================================================================
void OggStream::setAlbum( const char* album )
{
	m_Album = "album=";
	m_Album += album;
}

//============================================================================
void OggStream::setDate( const char* date )
{
	m_Date = "date=";
	m_Date += date;
}

//============================================================================
void OggStream::setGenre( const char* genre )
{
	m_Genre = "genre=";
	m_Genre += genre;
}

//============================================================================
void OggStream::setUserComment( const char* userComment )
{
	m_UserTag = "comment=";
	m_UserTag += userComment;
}

//============================================================================
bool OggStream::openOggStream( VFile * fileHandle, int streamSerialNumber )
{
	m_StreamInitialized		= false;
	m_TotalBytesWritten		= 0;

	m_LastSizeSegments		= 0;
	m_LastGranulePos		= 0;
	m_EncGranulePos			= 0;

	m_FileHandle = fileHandle; 
	if( 0 == streamSerialNumber )
	{
		streamSerialNumber = rand();
	}

	if( -1 == ogg_stream_init( &m_StreamState, streamSerialNumber ) )
	{
		LogMsg( LOG_ERROR, "Error: stream init failed");
		return false;
	}

	m_StreamInitialized = true;
	return true;
}

//============================================================================
int OggStream::writeHeader( MyOpusHeader& opusHeader, uint8_t* packetBuf, int bufLen )
{
	if( !m_StreamInitialized )
		return 0;

	m_OpusHeader = &opusHeader;
	int result		= m_OggBuffer.stuffOpusHeaderIntoPacket( opusHeader, packetBuf, bufLen );
	if( 19 != result )
	{
		LogMsg( LOG_ERROR, "OggStream::writeHeader failed" );
		return 0;
	}

	m_OggPkt.packet		= packetBuf;
	m_OggPkt.bytes		= result;
	m_OggPkt.b_o_s		= 1;
	m_OggPkt.e_o_s		= 0;
	m_OggPkt.granulepos	= 0;
	m_OggPkt.packetno	= 0;
	ogg_stream_packetin( &m_StreamState, &m_OggPkt );
	if( 0 == flushStreamToFile() )
	{
		return 0;
	}

	if( 0 == m_Date.length() )
	{
		m_Date = VxTimeUtil::getLocalDateAndTimeWithTextMonths();
	}

	char commentBuf[ 770 + 1];
	memset( commentBuf, 0, sizeof( commentBuf ) );
	m_OggBuffer.setBuffer( (uint8_t *)commentBuf, 770 );
	m_OggBuffer.writeChars( "OpusTags", 8 );
	m_OggBuffer.writeStringLengthThenString( opus_get_version_string() );
	m_OggBuffer.writeStringLengthThenString( " " );
	m_OggBuffer.writeStringLengthThenString( "ENCODER=SndToolsLib from nolimitconnect.org v0000000000000000-01v " );
	m_OggBuffer.writeStringLengthThenString( m_Title );
	m_OggBuffer.writeStringLengthThenString( m_Artist );
	m_OggBuffer.writeStringLengthThenString( m_Album );
	m_OggBuffer.writeStringLengthThenString( m_Date );
	m_OggBuffer.writeStringLengthThenString( m_Genre );
	m_OggBuffer.writeStringLengthThenString( m_UserTag );

	m_OggPkt.packet		= (uint8_t *)commentBuf;
	m_OggPkt.bytes		= 770;
	m_OggPkt.b_o_s		= 0;
	m_OggPkt.e_o_s		= 0;
	m_OggPkt.granulepos	= 0;
	m_OggPkt.packetno++;
	ogg_stream_packetin( &m_StreamState, &m_OggPkt );
	if( 0 == flushStreamToFile() )
	{
		return 0;
	}

	return 1;
}

//============================================================================
int OggStream::flushStreamToFile( void )
{
	if( !m_StreamInitialized )
		return 0;

	int result;
	int pageBytesWritten	= 0;
	int totalBytesWritten	= 0;

	while(( result = ogg_stream_flush( &m_StreamState, &m_OggPage ) ) )
	{
		if( !result )
			break;
		pageBytesWritten = writePageToOutput( &m_OggPage );
		if( pageBytesWritten != m_OggPage.header_len + m_OggPage.body_len)
		{
			LogMsg( LOG_ERROR, "Error: flushStreamToFile failed writing to output stream" );
			return 0;
		}

		totalBytesWritten += pageBytesWritten;
	}

	return totalBytesWritten;
}

//============================================================================
int OggStream::writePageToOutput( ogg_page * page )
{
	if( !m_StreamInitialized )
		return 0;

	int written = VFileWrite( page->header, 1, page->header_len, m_FileHandle );
	written += VFileWrite( page->body, 1, page->body_len, m_FileHandle );
	m_TotalBytesWritten += written;
	m_PagesOut++;
	return written;
}

//============================================================================
int OggStream::writeEncodedFrame( uint8_t * encodedFrame, int32_t encodedLen )
{
	if( !m_StreamInitialized )
		return 0;

	int retVal = 0;
	//enc_granulepos+=cur_frame_size*48000/coding_rate;
	int sizeSegments = ( encodedLen + 255 ) / 255;
	int encGranuleStep = m_OpusHeader->m_OpusFrameRate * 48000 / m_OpusHeader->m_InputSampleRate;
	m_EncGranulePos += encGranuleStep;

	// Flush early if adding this packet would make us end up with a
	// continued page which we wouldn't have otherwise.
	while(( ( ( sizeSegments <= 255 ) && ( m_LastSizeSegments + sizeSegments > 255 ) )
		|| ( m_EncGranulePos - m_LastGranulePos > m_MaxOggDelay ) ) 
		&& ogg_stream_flush_fill( &m_StreamState, &m_OggPage, 255*255 ) )
	{
		if( 0 != ogg_page_packets( &m_OggPage ) )
			m_LastGranulePos = ogg_page_granulepos( &m_OggPage );
		m_LastSizeSegments -= m_OggPage.header[26];
		retVal = writePageToOutput( &m_OggPage );
		if( retVal != m_OggPage.header_len + m_OggPage.body_len )
		{
			LogMsg( LOG_ERROR, "Error: failed writing data to output stream\n");
			return 0;
		}
	}

	

	/*The downside of early reading is if the input is an exact
	  multiple of the frame_size you'll get an extra frame that needs
	  to get cropped off. The downside of late reading is added delay.
	  If your ogg_delay is 120ms or less we'll assume you want the
	  low delay behavior.*/
	int nb_samples = OPUS_FRAME_RATE;
	if( ( m_OggPkt.e_o_s ) || m_MaxOggDelay <= 5760 )
	{
		nb_samples = -1;
	}

	m_OggPkt.packet		= (unsigned char *)encodedFrame;
	m_OggPkt.bytes		= encodedLen;
	m_OggPkt.b_o_s		= 0;
	m_OggPkt.granulepos = m_EncGranulePos;
	if( m_OggPkt.e_o_s )
	{
		/*We compute the final GP as ceil(len*48k/input_rate). When a resampling
		  decoder does the matching floor(len*input/48k) conversion the length will
		  be exactly the same as the input.*/
		m_OggPkt.granulepos = ((OPUS_FRAME_RATE * 48000 + m_OpusHeader->m_InputSampleRate - 1 ) / m_OpusHeader->m_InputSampleRate ) + m_OpusHeader->m_Preskip;
	}

	m_OggPkt.packetno = 2 + m_id;
	ogg_stream_packetin( &m_StreamState, &m_OggPkt );
	m_LastSizeSegments += sizeSegments;

	/*If the stream is over or we're sure that the delayed flush will fire,
	  go ahead and flush now to avoid adding delay.*/
	while( ( m_OggPkt.e_o_s 
		|| ( m_EncGranulePos + encGranuleStep - m_LastGranulePos > m_MaxOggDelay ) 
		|| ( m_LastSizeSegments >= 255 ) ) ?
				ogg_stream_flush_fill( &m_StreamState, &m_OggPage, 255 * 255 ) :
				ogg_stream_pageout_fill( &m_StreamState, &m_OggPage, 255 * 255 ) )
	{

		if( 0 != ogg_page_packets( &m_OggPage ) )
			m_LastGranulePos = ogg_page_granulepos( &m_OggPage );
		m_LastSizeSegments -= m_OggPage.header[26];
		retVal = writePageToOutput( &m_OggPage );
		if( retVal != m_OggPage.header_len + m_OggPage.body_len )
		{
			LogMsg( LOG_ERROR, "Error: failed writing data to output stream\n");
			return 0;
		}
	}

	return 1;
}

//============================================================================
uint64_t OggStream::closeOggStream( void )
{
	if( !m_StreamInitialized )
		return 0;

	flushStreamToFile();
	VFileClose( m_FileHandle );
	m_FileHandle = nullptr;
	return m_TotalBytesWritten;
}
