//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsPing.h"

#include <CoreLib/IsBigEndianCpu.h>

//============================================================================
PktPingReq::PktPingReq()
{
	setPktType( PKT_TYPE_PING_REQ );
	setPktLength( sizeof( PktPingReq ) );
}

//============================================================================
void PktPingReq::setTimestamp( uint64_t timeStamp )
{
	m_Timestamp = htonU64( timeStamp );
}

//============================================================================
uint64_t PktPingReq::getTimestamp( void )
{
	return ntohU64( m_Timestamp );
}

//============================================================================
PktPingReply::PktPingReply()
{
	setPktType( PKT_TYPE_PING_REPLY );
	setPktLength( sizeof( PktPingReq ) );
}

//============================================================================
void PktPingReply::setTimestamp( uint64_t timeStamp )
{
	m_Timestamp = htonU64( timeStamp );
}

//============================================================================
uint64_t PktPingReply::getTimestamp( void )
{
	return ntohU64( m_Timestamp );
}
