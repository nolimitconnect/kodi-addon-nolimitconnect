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

#include <CoreLib/VxMutex.h>

#include <vector>

// implements circular buffer and delays available data for anti jitter reasons
// buffers are AUDIO_BUF_SIZE long
class AudioJitterBuffer 
{
public:
	AudioJitterBuffer( int queDepth );
	~AudioJitterBuffer();

	void						lockResource( void )		{ m_QueMutex.lock(); }
	void						unlockResource( void )		{ m_QueMutex.unlock(); }

	void						clearBuffers( void );  // resets data avail and starts over

	// gets buffer to write to.. if null then overflowed.. increments index if buffer given
	char *						getBufToFill( void );
	// gets buffer to read.. if null then no data available.. increments index if buffer given
	char *						getBufToRead( void );

protected:
	//=== vars ===//
	VxMutex						m_QueMutex;
	std::vector<char *>			m_BufList;
	unsigned int				m_HeadIdx{ 0 };
	unsigned int				m_TailIdx{ 0 };
	unsigned int				m_BufsUsed{ 0 };
};
