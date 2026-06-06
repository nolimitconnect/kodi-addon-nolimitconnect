//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsRandConnect.h"

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
PktRandConnectReq::PktRandConnectReq()
{
    setPktType( PKT_TYPE_RAND_CONNECT_REQ );
    setPktLength( sizeof(PktRandConnectReq) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktRandConnectReply::PktRandConnectReply()
{
    setPktType( PKT_TYPE_RAND_CONNECT_REPLY );
    setPktLength( sizeof(PktRandConnectReply) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}
