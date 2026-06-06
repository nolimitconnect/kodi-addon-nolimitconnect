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

class NetActionWaitForInternet : public NetActionBase
{
public:
	NetActionWaitForInternet( NetServicesMgr& netServicesMgr );
	virtual ~NetActionWaitForInternet() = default;

	ENetActionType				getNetActionType( void ) override			{ return eNetActionWaitForInternet; }
	void						doAction( void ) override;

protected:
	bool						checkInternetAvailable( void );

	//=== vars ===//
};


