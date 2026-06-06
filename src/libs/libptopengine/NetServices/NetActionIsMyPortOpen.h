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

#include "NetActionBase.h"

class NetActionIsMyPortOpen : public NetActionBase
{
public:
	NetActionIsMyPortOpen( NetServicesMgr& netServicesMgr );
	virtual ~NetActionIsMyPortOpen() = default;

	ENetActionType				getNetActionType( void ) override			{ return eNetActionIsPortOpen; }
	void						doAction( void ) override;
};

