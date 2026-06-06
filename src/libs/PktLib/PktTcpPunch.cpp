//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTcpPunch.h"

//============================================================================
PktTcpPunch::PktTcpPunch()
: m_ConnectInfo()
, m_u32Res1( 0 )
, m_u32Res2( 0 )
, m_u32Res3( 0 )
, m_u32Res4( 0 )
, m_u32Res5( 0 )
, m_u32Res6( 0 )
{
	setPktLength( sizeof(PktTcpPunch) );		
	setPktType( PKT_TYPE_TCP_PUNCH ); 
};
