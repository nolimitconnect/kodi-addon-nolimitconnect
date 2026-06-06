#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetActionBase.h"

class NetActionResolveConnectTestUrl : public NetActionBase
{
public:
	NetActionResolveConnectTestUrl( NetServicesMgr& netServicesMgr );
	virtual ~NetActionResolveConnectTestUrl() = default;

	ENetActionType				getNetActionType( void ) override			{ return eNetActionResolveConnectTestUrl; }
	void						doAction( void ) override;

protected:
	//=== vars ===//
};


