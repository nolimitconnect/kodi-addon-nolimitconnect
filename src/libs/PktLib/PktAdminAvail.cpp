//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktAdminAvail.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PktAdminAvail::PktAdminAvail()
{
	setPktType( PKT_TYPE_ADMIN_AVAIL );
	setPktLength( sizeof( PktAdminAvail ) );
	vx_assert( 0 == (getPktLength() & 0x0f) )
}
