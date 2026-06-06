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
#include "PktsPushToTalk.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PktPushToTalkReq::PktPushToTalkReq()
{
	vx_assert( 0 == (sizeof( PktPushToTalkReq ) & 0x0f) );
	setPktType( PKT_TYPE_PUSH_TO_TALK_REQ );
	setPktLength( sizeof( PktPushToTalkReq ) );
}

//============================================================================
PktPushToTalkReply::PktPushToTalkReply()
{
    vx_assert( 0 == (sizeof( PktPushToTalkReply ) & 0x0f) );
	setPktType( PKT_TYPE_PUSH_TO_TALK_REPLY );
	setPktLength( sizeof( PktPushToTalkReply ) );
}

//============================================================================
PktPushToTalkStart::PktPushToTalkStart()
{
	vx_assert( 0 == (sizeof( PktPushToTalkStart ) & 0x0f) );
	setPktType( PKT_TYPE_PUSH_TO_TALK_START );
	setPktLength( sizeof( PktPushToTalkStart ) );
}

//============================================================================
PktPushToTalkStop::PktPushToTalkStop()
{
	vx_assert( 0 == (sizeof( PktPushToTalkStop ) & 0x0f) );
	setPktType( PKT_TYPE_PUSH_TO_TALK_STOP );
	setPktLength( sizeof( PktPushToTalkStop ) );
}