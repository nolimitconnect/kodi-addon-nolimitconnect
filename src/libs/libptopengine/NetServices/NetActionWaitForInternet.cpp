//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetActionWaitForInternet.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxSktUtil.h>

//============================================================================
NetActionWaitForInternet::NetActionWaitForInternet( NetServicesMgr& netServicesMgr )
: NetActionBase( netServicesMgr )
{
}

//============================================================================
void NetActionWaitForInternet::doAction( void )
{
	while( !checkInternetAvailable() )
	{
		VxSleep( 1000 );
	}
}

//============================================================================
bool NetActionWaitForInternet::checkInternetAvailable( void )
{
	return m_Engine.getNetStatusAccum().isInternetAvailable();
}
