//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsMultiSession.h"
#include "PktTypes.h"

//============================================================================
PktMultiSessionReq::PktMultiSessionReq()
: m_u32Action( 0 )
, m_u32ActionParam( 0 )
, m_u32Res3( 0 )
, m_u32Res4( 0 )
, m_u32Res5( 0 )
, m_u32Res6( 0 )
{
	setPktType( PKT_TYPE_MSESSION_REQ ); 
	setPktLength( sizeof( PktMultiSessionReq ) ); 
}

//============================================================================
PktMultiSessionReply::PktMultiSessionReply()
: m_u32Action( 0 )
, m_u32ActionParam( 0 )
, m_u32Res3( 0 )
, m_u32Res4( 0 )
, m_u32Res5( 0 )
, m_u32Res6( 0 )
{
	setPktType( PKT_TYPE_MSESSION_REPLY ); 
	setPktLength( sizeof( PktMultiSessionReply ) ); 
}

