//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsSession.h"

//============================================================================
PktSessionStartReq::PktSessionStartReq()
    : VxPktHdr()
{
	setPktLength( sizeof( PktSessionStartReq ) ); 
	setPktType(  PKT_TYPE_SESSION_START_REQ );
}

//============================================================================
PktSessionStartReply::PktSessionStartReply()
    : VxPktHdr()
{ 
	setPktLength( sizeof( PktSessionStartReply ) ); 
	setPktType(  PKT_TYPE_SESSION_START_REPLY );
}

//============================================================================
PktSessionStopReq::PktSessionStopReq()
    : VxPktHdr()
{
	setPktLength( sizeof( PktSessionStopReq ) ); 
	setPktType(  PKT_TYPE_SESSION_STOP_REQ );
}

//============================================================================
PktSessionStopReply::PktSessionStopReply()
    : VxPktHdr()
{ 
	setPktLength( sizeof( PktSessionStopReply ) ); 
	setPktType(  PKT_TYPE_SESSION_STOP_REPLY );
}

