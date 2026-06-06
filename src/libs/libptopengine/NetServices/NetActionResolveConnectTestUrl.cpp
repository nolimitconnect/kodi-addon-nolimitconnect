//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetActionResolveConnectTestUrl.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxSktUtil.h>

//============================================================================
NetActionResolveConnectTestUrl::NetActionResolveConnectTestUrl( NetServicesMgr& netServicesMgr )
: NetActionBase( netServicesMgr )
{
}

//============================================================================
void NetActionResolveConnectTestUrl::doAction( void )
{
	bool ipv6 = m_Engine.getNetStatusAccum().getUseIpv6();
    EFirewallTestType testType = m_Engine.getEngineSettings().getFirewallTestSetting();
    if( eFirewallTestAssumeNoFirewall == testType )
    {
		std::string externIp;
        m_Engine.getEngineSettings().getUserSpecifiedExternIpAddr( externIp, ipv6 );
        if( !externIp.empty() )
        {
			m_Engine.getNetStatusAccum().setExternalIpAddress( externIp );
			m_Engine.getNetStatusAccum().setConnectionTestAvail( true );
            LogMsg( LOG_INFO, "NetActionResolveConnectTestUrl::%s: user specified fixed ip %s",
					__func__, externIp.c_str() );
			return;
        }
    }

	std::string connectTestUrl;
	m_Engine.getEngineSettings().getConnectTestUrl( connectTestUrl );
	if( connectTestUrl.empty() )
	{
		m_Engine.getNetStatusAccum().setConnectionTestAvail( false );
		LogMsg( LOG_ERROR, "NetActionResolveConnectTestUrl:%s Empty connectTestUrl url", __func__ );
		return;
	}

	// check if url resolves to our ip address
	bool preferIpv6Resolve = VxGetIpAddrType( m_Engine.getNetStatusAccum().getLocalIpAddress().c_str() ) == eIpAddrTypeIpv6;
	std::string resolvedIpAddr;
	uint16_t resovedPort{ 0 };
	bool wasResolved = VxResolvePtopUrl( connectTestUrl, resolvedIpAddr, resovedPort, preferIpv6Resolve );
	if( wasResolved )
	{
		if( m_Engine.getNetStatusAccum().getExternalIpAddress() == resolvedIpAddr ||
			m_Engine.getNetStatusAccum().getLocalIpAddress() == resolvedIpAddr )
		{
			LogMsg( LOG_INFO, "NetActionResolveConnectTestUrl:%s we are the resolved url", __func__ );
			m_Engine.getNetStatusAccum().setConnectionTestAvail( true );
			return;
		}
	}

	std::vector<HostUrlInfo> hostUrls;
	m_Engine.getHostUrlListMgr().getHostUrls( eHostTypeConnectTest, hostUrls );
	if( !hostUrls.empty() )
	{
		HostUrlInfo urlInfo = hostUrls.at( 0 );
		if( urlInfo.getHostUrl() == connectTestUrl )
		{
			if( urlInfo.getOnlineId().isValid() )
			{
				// already resolved
				m_Engine.getNetStatusAccum().setConnectionTestAvail( true );
				return;
			}
		}
	}

	m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeConnectTest, connectTestUrl );

	// try to speed up network bring up by starting the network host query now in hopes it will be ready before we resolve network host
	std::string networkHostUrl;
	m_Engine.getEngineSettings().getNetworkHostUrl( networkHostUrl );
	if( !networkHostUrl.empty() )
	{
		m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeNetwork, networkHostUrl );
	}

	wasResolved = false;
	int waitCnt = 0;
	VxGUID hostOnlineId;
	while( waitCnt < 20 )
	{
		if( m_Engine.getConnectionMgr().getDefaultHostOnlineId( eHostTypeConnectTest, hostOnlineId ) )
		{
			if( hostOnlineId.isValid() )
			{
				LogMsg( LOG_INFO, "NetActionResolveConnectTestUrl:%s resolved with id %s", __func__, hostOnlineId.toOnlineIdString().c_str() );
				m_Engine.getNetStatusAccum().setConnectionTestAvail( true );
				return;
			}
		}

		VxSleep( 1000 );
		waitCnt++;
	}

	LogMsg( LOG_ERROR, "NetActionResolveConnectTestUrl:%s timed out waiting to resolve %s", __func__, connectTestUrl.c_str() );
	m_Engine.getNetStatusAccum().setConnectionTestAvail( false );
}


