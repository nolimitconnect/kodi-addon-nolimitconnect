//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxAudioFormat.h"


//============================================================================
VxAudioFormat::VxAudioFormat( VxAudioFormat& rhs )
	: m_Rate( rhs.m_Rate )
	, m_ChannelBytes( rhs.m_ChannelBytes )
	, m_ChannelCount( rhs.m_ChannelCount )
	, m_SampleFormat( rhs.m_SampleFormat )
{
}

//============================================================================
VxAudioFormat::VxAudioFormat( const VxAudioFormat& rhs )
	: m_Rate( rhs.m_Rate )
	, m_ChannelBytes( rhs.m_ChannelBytes )
	, m_ChannelCount( rhs.m_ChannelCount )
	, m_SampleFormat( rhs.m_SampleFormat )
{
}

//============================================================================
VxAudioFormat& VxAudioFormat::operator =( const VxAudioFormat& rhs )
{
	if( this != &rhs )
	{
		m_Rate = rhs.m_Rate;
		m_ChannelBytes = rhs.m_ChannelBytes;
		m_ChannelCount = rhs.m_ChannelCount;
		m_SampleFormat = rhs.m_SampleFormat;
	}

	return *this;
}

//============================================================================
bool VxAudioFormat::isValid( void ) const
{
	if( m_Rate < 8000 )
	{
		return false;
	}

	if( m_ChannelBytes < 1 || m_ChannelBytes > 8 )
	{
		return false;
	}

	if( m_ChannelCount < 1 || m_ChannelCount > 8 )
	{
		return false;
	}

	if( m_SampleFormat < UInt8 )
	{
		return false;
	}

	return true;
}

//=============================================================================
int64_t VxAudioFormat::bytesForDuration( int64_t durationUs ) const
{
	int64_t result = ( sampleRate() * channelCount() * bytesPerSample() )
		* durationUs / 1000000;
	result -= result % ( channelCount() * bytesPerSample() * 8 );
	return result;
}
