//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MyOpusHeader.h"
#include <memory.h>

//============================================================================
MyOpusHeader::MyOpusHeader()
{
	memset( m_StreamMap, 0, sizeof( m_StreamMap ) );
}
