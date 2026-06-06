//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktChatReq.h"

#include <CoreLib/VxParse.h>

#include <memory.h>

//============================================================================
PktChatReq::PktChatReq()
{
	setPktType(  PKT_TYPE_CHAT_REQ ); 
}

//============================================================================
int PktChatReq::emptyLen( void )
{ 
	return sizeof( PktChatReq ) - sizeof( m_au8Additional ); 
} 

//============================================================================
int PktChatReq::addImages( int iImageCnt, uint16_t * pu16Images )
{
	setPktLength( emptyLen() );
	m_u16ImageCnt = (uint16_t)iImageCnt;
	if( iImageCnt )
	{
		memcpy( m_au8Additional, pu16Images, iImageCnt * 2 * sizeof( uint16_t ) );
		return 0;
	}
	return -1;
}

//============================================================================
void PktChatReq::addMsg( const char* pMsg )
{
	int iLen = (int)strlen( pMsg );
	if( iLen < PKT_CHAT_MAX_MSG_LEN )
	{
		strcpy( (char *)m_au8Additional, pMsg );
	}
	else
	{
		iLen = PKT_CHAT_MAX_MSG_LEN - 1;
		strncpy( (char *)m_au8Additional, pMsg,  iLen );
		m_au8Additional[ PKT_CHAT_MAX_MSG_LEN ] = 0;
	}

	setPktLength( ROUND_TO_16BYTE_BOUNDRY( emptyLen() + iLen + 1 ) );
}
