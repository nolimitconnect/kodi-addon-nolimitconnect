//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetActionResolveNetworkHostUrl.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxSktUtil.h>

//============================================================================
NetActionResolveNetworkHostUrl::NetActionResolveNetworkHostUrl( NetServicesMgr& netServicesMgr )
: NetActionBase( netServicesMgr )
{
}

//============================================================================
void NetActionResolveNetworkHostUrl::doAction( void )
{
	// it may already be resolved
	VxGUID hostOnlineId;
	if( m_Engine.getConnectionMgr().getDefaultHostOnlineId( eHostTypeNetwork, hostOnlineId ) && hostOnlineId.isValid() )
	{
		m_Engine.getNetStatusAccum().setNetHostAvail( true );
		return;
	}

	std::string networkHostUrl;
	m_Engine.getEngineSettings().getNetworkHostUrl( networkHostUrl );
	if( networkHostUrl.empty() )
	{
		m_Engine.getNetStatusAccum().setNetHostAvail( false );
		LogMsg( LOG_ERROR, "NetActionResolveNetworkHostUrl:%s Empty networkHostUrl url", __func__ );
		return;
	}

	// check if url resolves to our ip address
	// if our local address is ipv6 then prefer ipv6 resolve
	bool preferIpv6Resolve = VxGetIpAddrType( m_Engine.getNetStatusAccum().getLocalIpAddress().c_str() ) == eIpAddrTypeIpv6;
	std::string resolvedIpAddr;
	uint16_t resovedPort{ 0 };
	bool wasResolved = VxResolvePtopUrl( networkHostUrl, resolvedIpAddr, resovedPort, preferIpv6Resolve );
	if( wasResolved )
	{
		if( m_Engine.getNetStatusAccum().getExternalIpAddress() == resolvedIpAddr ||
			m_Engine.getNetStatusAccum().getLocalIpAddress() == resolvedIpAddr )
		{
			LogMsg( LOG_INFO, "NetActionResolveNetworkHostUrl:%s we are the resolved url", __func__ );
			m_Engine.getNetStatusAccum().setNetHostAvail( true );
			return;
		}
	}

	m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeNetwork, networkHostUrl );

	int waitCnt = 0;

	while( waitCnt < 22 )
	{
        if( m_Engine.getConnectionMgr().getDefaultHostOnlineId( eHostTypeNetwork, hostOnlineId ) )
		{
			if( hostOnlineId.isValid() )
			{
                LogMsg( LOG_INFO, "NetActionResolveNetworkHostUrl::%s resolved with id %s", __func__, hostOnlineId.toOnlineIdString().c_str() );
				m_Engine.getNetStatusAccum().setNetHostAvail( true );
				return;
			}
		}

		VxSleep( 1000 );
		waitCnt++;
	}

	LogMsg( LOG_ERROR, "NetActionResolveNetworkHostUrl:%s timed out waiting to resolve %s", __func__, networkHostUrl.c_str() );
	m_Engine.getNetStatusAccum().setNetHostAvail( false );
}


