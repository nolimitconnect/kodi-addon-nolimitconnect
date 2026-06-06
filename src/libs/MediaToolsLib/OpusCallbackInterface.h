#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxDefs.h>

class OpusCallbackInterface
{
public:
	virtual void				opusStatus( int errorCode ){};
	virtual void				opusProgress( uint64_t bytesProcessed ){};
};
