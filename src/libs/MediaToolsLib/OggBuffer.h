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

#include <CoreLib/VxDefs.h>

#include <string>

class MyOpusHeader;

class OggBuffer
{
public:
	OggBuffer();
	~OggBuffer() = default;

	void						setDataBuf( uint8_t * dataBuf )				{ m_DataBuf = dataBuf; }
	uint8_t *					getDataBuf( void )							{ return m_DataBuf; }
	void						setMaxLen( int maxLen )						{ m_MaxLen = maxLen; }
	int							getMaxLen( void )							{ return m_MaxLen; }
	void						setBuffer( uint8_t * dataBuf, int len )		{ m_DataBuf = dataBuf; m_MaxLen = len; }

	void						setPos( int pos )							{ m_Pos = pos; }
	int							getPos( void )								{ return m_Pos; }

	int							writeU32( uint32_t val );
	int							writeU16( uint16_t val );
	int							writeChars( const uint8_t * str, int charCount );
	int							writeChars( const char* str, int charCount );
	int							writeStringLengthThenString( const char* str );
	int							writeStringLengthThenString( std::string& str );


	int							readU32( uint32_t * retVal );
	int							readU16( uint16_t * retVal );
	int							readChars( uint8_t * str, int charCount );

	int							stuffOpusHeaderIntoPacket(	MyOpusHeader&			opusHeader, 
															uint8_t *				packetBuf, 
															int						pktBufLen );

	int							parsePktIntoOpusHeader(		uint8_t *				packetBuf, 
															int						pktBufLen, 
															MyOpusHeader&			opusHeader );


	uint8_t *					m_DataBuf;
	int							m_MaxLen;
	int							m_Pos;
};
