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

class NetServicesMgr;

class NetActionIdle : public NetActionBase
{
public:
	NetActionIdle( NetServicesMgr& netServicesMgr );
	virtual ~NetActionIdle();

	ENetActionType				getNetActionType( void ) override			{ return eNetActionIdle; }
	void						doAction( void )  override;
};

