#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxDefs.h>
#include <CoreLib/AppErr.h>

class Firewall
{
public:
	Firewall();
	~Firewall();

	EAppErr						addGlobalPort( uint16_t u16Port );
};


