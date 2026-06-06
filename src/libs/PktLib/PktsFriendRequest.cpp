//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsFriendRequest.h"

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
//============================================================================
PktFriendRequestBase::PktFriendRequestBase()
{
	m_au8Data[0] = 0;
}

//============================================================================
void PktFriendRequestBase::setRequestMsg( const char* msg )
{
	vx_assert( msg );
	uint16_t msgLen = ( uint16_t)strlen( msg );
	vx_assert( MAX_FRIEND_REQUEST_MSG_LEN > msgLen );
	strcpy( (char *)m_au8Data, msg );
	m_StrLen = htons( msgLen );
}

//============================================================================
const char* PktFriendRequestBase::getRequestMsg( void )
{
	return (const char*)m_au8Data;
}

//============================================================================
void PktFriendRequestBase::calcPktLen( void )
{
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( (sizeof( PktFriendRequestBase ) - PKT_BLOB_MAX_STORAGE_LEN + m_BlobEntry.getBlobLen() ) ) );
}

//============================================================================
//============================================================================
PktFriendRequestReq::PktFriendRequestReq()
{
    setPktType( PKT_TYPE_FRIEND_REQUEST_REQ );
}

//============================================================================
//============================================================================
PktFriendRequestReply::PktFriendRequestReply()
{
    setPktType( PKT_TYPE_FRIEND_REQUEST_REPLY );
}
