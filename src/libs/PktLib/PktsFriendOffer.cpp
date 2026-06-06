//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsFriendOffer.h"

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
PktFriendOfferReq::PktFriendOfferReq()
{
	setPktType( PKT_TYPE_FRIEND_OFFER_REQ );
	m_au8Data[0] = 0;
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktFriendOfferReq ) - sizeof(m_au8Data) + 1) ); 
}

//============================================================================
void PktFriendOfferReq::setOfferMsg( const char* msg )
{
	vx_assert( msg );
	uint16_t msgLen = ( uint16_t)strlen( msg );
	vx_assert( MAX_FRIEND_OFFER_MSG_LEN > msgLen );
	strcpy( (char *)m_au8Data, msg );
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktFriendOfferReq ) - sizeof(m_au8Data) + msgLen + 1) ); 
	m_StrLen = htons( msgLen );
}

//============================================================================
const char* PktFriendOfferReq::getOfferMsg( void )
{
	return (const char*)m_au8Data;
}

//============================================================================
//============================================================================
PktFriendOfferReply::PktFriendOfferReply()
{
    setPktType( PKT_TYPE_FRIEND_OFFER_REPLY );
    m_au8Data[0] = 0;
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktFriendOfferReply ) - sizeof(m_au8Data) + 1) ); 
}

//============================================================================
void PktFriendOfferReply::setOfferMsg( const char* msg )
{
    vx_assert( msg );
    uint16_t msgLen = ( uint16_t)strlen( msg );
    vx_assert( MAX_FRIEND_OFFER_MSG_LEN > msgLen );
    strcpy( (char *)m_au8Data, msg );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktFriendOfferReply ) - sizeof(m_au8Data) + msgLen + 1) ); 
    m_StrLen = htons( msgLen );
}

//============================================================================
const char* PktFriendOfferReply::getOfferMsg( void )
{
    return (const char*)m_au8Data;
}
