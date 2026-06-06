//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AudioJitterBuffer.h"

#include <CoreLib/VxDebug.h>
#include <MediaToolsLib/SndDefs.h>

// implements circular buffer and delays available data for anti jitter reasons
// buffers are AUDIO_BUF_SIZE long

// #define DEBUG_AUDIO_JITTER_BUF

//============================================================================
AudioJitterBuffer::AudioJitterBuffer( int queDepth )
{
	for( int i = 0; i < queDepth; i++ )
	{
		m_BufList.push_back( new char[ AUDIO_BUF_SIZE] );
	}
}

//============================================================================
AudioJitterBuffer::~AudioJitterBuffer()
{
	for( char* buf : m_BufList)
	{
		delete buf;
	}

	m_BufList.clear();
}

//============================================================================
void AudioJitterBuffer::clearBuffers( void )  // resets data avail and starts over
{
	m_HeadIdx	= 0;
	m_TailIdx	= 0;
	m_BufsUsed	= 0;
}

//============================================================================
// gets buffer to write to.. if null then overflowed.. increments index if buffer given
char * AudioJitterBuffer::getBufToFill( void )
{
	char * retBuf = nullptr;
	if( m_BufsUsed < m_BufList.size() )
	{
		retBuf = m_BufList[ m_HeadIdx ];
		m_HeadIdx++;
		if( m_HeadIdx >= m_BufList.size() )
		{
			m_HeadIdx = 0;
		}

		m_BufsUsed++;
	}

	if( !retBuf )
	{
		// throw away a buffer
		LogModule( eLogStreams, LOG_VERBOSE, "AudioJitterBuffer::getBufToFill overflow this %p throw away idx %d", this, m_TailIdx );
		getBufToRead();
	}
#ifdef DEBUG_AUDIO_JITTER_BUF
	else
	{
		LogMsg( LOG_INFO, "AudioJitterBuffer::getBufToFill success %p next idx %d used %d", this, m_HeadIdx, m_BufsUsed );
	}
#endif // DEBUG_AUDIO_JITTER_BUF

	return retBuf;
}

//============================================================================
// gets buffer to read.. if null then no data available.. increments index if buffer given
char * AudioJitterBuffer::getBufToRead( void )
{
	char * retBuf = nullptr;
	if( m_BufsUsed )
	{
		retBuf = m_BufList[ m_TailIdx ];
		m_TailIdx++;
		if( m_TailIdx >= m_BufList.size() )
		{
			m_TailIdx = 0;
		}

		m_BufsUsed--;
	}

	if( !retBuf )
	{
		LogModule( eLogStreams, LOG_VERBOSE, "AudioJitterBuffer::getBufToRead underflow this %p", this );
	}
#ifdef DEBUG_AUDIO_JITTER_BUF
	else
	{
		LogMsg( LOG_INFO, "AudioJitterBuffer::getBufToRead success %p next idx %d used %d", this, m_TailIdx, m_BufsUsed );
	}
#endif // DEBUG_AUDIO_JITTER_BUF

	return retBuf;
}

