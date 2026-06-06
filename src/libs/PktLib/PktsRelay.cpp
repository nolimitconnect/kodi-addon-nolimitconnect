//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsRelay.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PktRelayUserDisconnect::PktRelayUserDisconnect()
{
	setPktLength( sizeof(PktRelayUserDisconnect) );		
	setPktType( PKT_TYPE_RELAY_USER_DISCONNECT ); 
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
};
