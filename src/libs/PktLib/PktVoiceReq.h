#pragma once

//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPktHdr.h"
#include <CoreLib/IsBigEndianCpu.h>
#include <GuiInterface/IAudioDefs.h>

#define VOICE_PACKET_MAX_COMPRESSED_LEN AUDIO_BUF_SIZE

#pragma pack(push)
#pragma pack(1)
class PktVoiceReq : public VxPktHdr
{
public:
	PktVoiceReq();

	void 						calcPktLen( void );

	void						setCompressedDataLen( uint16_t len )	{ m_u16CompressedDataLen = htons( len ); }
	int							getCompressedDataLen( void )			{ return ntohs( m_u16CompressedDataLen ); }

	uint8_t *					getCompressedData( void )				{ return m_CompressedData; }
	int							getMaxCompressedDataBufLen( void )		{ return VOICE_PACKET_MAX_COMPRESSED_LEN; }

	void						setTimeMs( int64_t time )				{ m_s64TimeMs = htonU64( time ); }
	int64_t 					getTimeMs( void )						{ return ntohU64( m_s64TimeMs ); }

private:
	//=== vars ===//
	uint16_t					m_u8CompressionType{ 1 };
	uint16_t					m_u8CompressionVersion{ 1 };
	int64_t					    m_s64TimeMs{ 0 };
	uint16_t					m_u16CompressedDataLen{ 0 };
	uint16_t					m_u16Res1{ 0 };
	uint16_t					m_u16Res2{ 0 };
	uint16_t					m_u16Res3{ 0 };
	uint8_t						m_CompressedData[ VOICE_PACKET_MAX_COMPRESSED_LEN ];
};

#pragma pack(pop)


