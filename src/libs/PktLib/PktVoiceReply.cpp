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
#include "PktVoiceReply.h"

PktVoiceReply::PktVoiceReply()
: m_u16Res1(0)
, m_u32TimeMs(0)
, m_u32Res2(0)
, m_u32Res3(0)
, m_u32Res4(0)
{
	setPktType( PKT_TYPE_VOICE_REPLY );
	setPktLength( sizeof( PktVoiceReply ) );
}
