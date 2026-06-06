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
#include "NetServicesMgr.h"

#include <P2PEngine/P2PEngine.h>

//============================================================================
NetActionBase::NetActionBase( NetServicesMgr& netServicesMgr )
: m_NetServicesMgr( netServicesMgr )
, m_NetServiceUtils( netServicesMgr.getNetUtils() )
, m_Engine( netServicesMgr.getEngine() )
{
}

//============================================================================
std::string NetActionBase::getNetworkKey( void )
{
    return m_NetServicesMgr.getNetworkKey();
}

//============================================================================
VxGUID& NetActionBase::getMyOnlineId( void )
{
	return m_Engine.getMyOnlineId();
}
