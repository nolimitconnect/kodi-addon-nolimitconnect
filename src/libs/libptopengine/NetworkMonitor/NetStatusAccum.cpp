//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetStatusAccum.h"

#include <P2PEngine/P2PEngine.h>
#include <UrlMgr/UrlMgr.h>
#include <Plugins/PluginMgr.h>

#include <GuiInterface/IToGui.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxSktUtil.h>

#include <NetLib/NetHostSettingDefs.h>
#include <NetLib/VxPeerMgr.h>

namespace
{
	//============================================================================
    void * NetStatusAccumThreadFunc( void * pvContext )
	{
		VxThread* poThread = (VxThread*)pvContext;
		poThread->setIsThreadRunning( true );

		NetStatusAccum * poMgr = (NetStatusAccum *)poThread->getThreadUserParam();
        if( poMgr && false == poThread->isAborted() )
        {
            poMgr->threadedSetupListen();
        }

		poThread->threadAboutToExit();
        return nullptr;
	}
}

//============================================================================
NetStatusAccum::NetStatusAccum( P2PEngine& engine )
    : m_Engine(engine)
{
}

//============================================================================
void NetStatusAccum::resetConnectionTestStatus( void )
{
    m_ConnectionTestAvail = false;
    m_DirectConnectTested = false;
    m_RequriesRelay = true;

    m_IsExternalIpValid = false;

    onNetStatusChange();
}

//============================================================================
void NetStatusAccum::resetNetworkStatusAll( void )
{
    m_InternetAvail = false;
    m_NetworkHostAvail = false;

    m_ConnectedToRelay = false;
    m_GroupListHostAvail = false;
    m_GroupHostAvail = false;
    m_IsConnectedGroupHost = false;

    resetConnectionTestStatus();
}

//============================================================================
void NetStatusAccum::fromGuiNetworkSettingsChanged( void )
{
    resetNetworkStatusAll();
}

//============================================================================
void NetStatusAccum::onNetStatusChange( void )
{
    ENetAvailStatus netAvailStatus = eNetAvailNoInternet;
    EInternetStatus internetStatus = m_InternetAvail ? eInternetInternetAvailable : eInternetNoInternet;
    if( eFirewallTestAssumeNoFirewall == m_FirewallTestType )
    {
        internetStatus = eInternetAssumeDirectConnect;
        netAvailStatus = eNetAvailP2PAvail; // must be at least P2P available or will not accept incomming accept sockets

        bool ipv6 = m_Engine.getEngineSettings().getUseIpv6();
        if( m_ExternAddr.empty() )
        {
            setExternalIpAddress( m_Engine.getEngineSettings().getUserSpecifiedExternIpAddr( ipv6 ) );
        }

        if( isDirectConnectTested() )
        {
            // direct connect tested is set when first packet announce is recieved when have fixed ip
            if( m_Engine.isNetworkHostEnabled() )
            {
                // we are the host
                netAvailStatus = eNetAvailDirectGroupHost; // 7 bars green
            }
            else
            {
                netAvailStatus = eNetAvailFullOnlineDirectConnect;  // 5 bars green
                if( m_GroupHostAvail )
                {
                    netAvailStatus = eNetAvailDirectGroupHost; // 7 bars green
                }
            }
        }
        else
        {
            if( m_Engine.isNetworkHostEnabled() )
            {
                // we are the host
                netAvailStatus = eNetAvailRelayGroupHost; // 6 bars orange
            }
            else
            {
                netAvailStatus = eNetAvailFullOnlineWithRelay; // 4 bars orange
                if( m_GroupHostAvail )
                {
                    netAvailStatus = eNetAvailRelayGroupHost; // 6 bars orange
                }
            }
        }
    }
    else if( eFirewallTestAssumeFirewalled == m_FirewallTestType )
    {
        if( m_InternetAvail )
        {
            internetStatus = eInternetRequiresRelay;
        }
    }
    else if( eFirewallTestUrlConnectionTest == m_FirewallTestType )
    {
        if( m_InternetAvail )
        {
            if( m_ConnectionTestAvail )
            {
                internetStatus = eInternetTestHostAvailable;
                if( m_DirectConnectTested )
                {
                    if( m_RequriesRelay )
                    {
                        internetStatus = eInternetRequiresRelay;
                    }
                    else
                    {
                        internetStatus = eInternetCanDirectConnect;
                    }
                }
            }
            else
            {
                internetStatus = eInternetTestHostUnavailable;
            }
        }
    }

    if( m_InternetStatus != internetStatus )
    {
        m_AccumMutex.lock();
        m_InternetStatus = internetStatus;
        m_AccumMutex.unlock();

        m_AccumCallbackMutex.lock();
        for( auto callback : m_CallbackList )
        {
            callback->callbackInternetStatusChanged( internetStatus );
        }

        m_AccumCallbackMutex.unlock();

        LogModule( eLogNetworkState, LOG_VERBOSE, "Internet Status %s", DescribeInternetStatus( internetStatus ) );
    }

    if( !( eFirewallTestAssumeNoFirewall == m_FirewallTestType ) )
    {
        if( m_NetworkHostAvail )
        {
            netAvailStatus = eNetAvailHostAvail;
            if( m_DirectConnectTested )
            {
                netAvailStatus = eNetAvailP2PAvail;
                if( requiresRelay() )
                {
                    if( m_ConnectedToRelay )
                    {
                        netAvailStatus = eNetAvailFullOnlineWithRelay;
                    }
                    else
                    {
                        netAvailStatus = eNetAvailOnlineButNoRelay;
                    }
                }
                else
                {
                    netAvailStatus = eNetAvailFullOnlineDirectConnect;
                }

                if( eNetAvailFullOnlineDirectConnect == netAvailStatus )
                {
                    // fully connected
                    if( m_GroupHostAvail )
                    {
                        netAvailStatus = eNetAvailDirectGroupHost;
                    }
                }

                if( eNetAvailFullOnlineWithRelay == netAvailStatus )
                {
                    // fully connected
                    if( m_GroupHostAvail )
                    {
                        netAvailStatus = eNetAvailRelayGroupHost;
                    }
                }
            }
        }
    }

    if( m_NetAvailStatus != netAvailStatus )
    {
        m_AccumMutex.lock();
        m_NetAvailStatus = netAvailStatus;
        m_AccumMutex.unlock();

        m_AccumCallbackMutex.lock();
        for( auto callback : m_CallbackList )
        {
            callback->callbackNetAvailStatusChanged( netAvailStatus );
        }

        m_AccumCallbackMutex.unlock();

        if( eNetAvailFullOnlineDirectConnect == netAvailStatus )
        {
            // just in case direct connect test was skipped by assume no firewall
            m_Engine.getMyPktAnnounce().setRequiresRelay( false );
        }

        LogModule( eLogNetworkState, LOG_VERBOSE, "Net Avail Status %s", DescribeNetAvailStatus( netAvailStatus ) );
        m_Engine.getToGui().toGuiNetAvailableStatus( netAvailStatus );
    }   
}

//============================================================================
void NetStatusAccum::addNetStatusCallback( NetAvailStatusCallbackInterface* callbackInt )
{
    if( callbackInt )
    {
        ENetAvailStatus netAvailStatus = getNetAvailStatus();
        m_AccumCallbackMutex.lock();
        bool alreadyExist = false;
        for( auto callback : m_CallbackList )
        {
            if( callback == callbackInt )
            {
                alreadyExist = true;
                break;
            }
        }

        if( !alreadyExist )
        {
            m_CallbackList.push_back( callbackInt );
            callbackInt->callbackNetAvailStatusChanged( netAvailStatus );
        }

        m_AccumCallbackMutex.unlock();
    }
}

//============================================================================
void NetStatusAccum::removeNetStatusCallback( NetAvailStatusCallbackInterface* callbackInt )
{
    if( callbackInt )
    {
        m_AccumCallbackMutex.lock();
        for( auto iter = m_CallbackList.begin(); iter != m_CallbackList.begin(); ++iter )
        {
            if( *iter == callbackInt )
            {
                m_CallbackList.erase( iter );
                break;
            }
        }

        m_AccumCallbackMutex.unlock();
    }
}

//============================================================================
void NetStatusAccum::setInternetAvail( bool avail )
{
    if( avail != m_InternetAvail )
    {
        m_InternetAvail = avail;
        LogModule( eLogNetworkState, LOG_VERBOSE, "Internet available %d", avail );

        onNetStatusChange();
    }
}

//============================================================================
void NetStatusAccum::setNetHostAvail( bool avail )
{
    if( avail != m_NetworkHostAvail )
    {
        m_NetworkHostAvail = avail;
        LogModule( eLogNetworkState, LOG_VERBOSE, "Network Host available %d", avail );
        onNetStatusChange();
    }
}

//============================================================================
void NetStatusAccum::setConnectionTestAvail( bool avail )
{
    if( avail != m_ConnectionTestAvail )
    {
        m_ConnectionTestAvail = avail;
        LogModule( eLogNetworkState, LOG_VERBOSE, "Connection Test available %d", avail );
        onNetStatusChange();
    }
}

//============================================================================
void NetStatusAccum::setDirectConnectTested( bool isTested, bool requiresRelay, std::string& myExternalIp )
{
    if( !VxIsIpValid( myExternalIp ) )
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::setDirectConnectTested invalid external Ip" );
        return;
    }

    if( !isTested )
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::setDirectConnectTested isTested should not be false" );
        return;
    }

    bool requresRelayChanged{ false };
    bool directConnectTestedChanged{ false };
    bool externIpChanged{ false };

    std::string prevExternIp = getExternalIpAddress();
    if( prevExternIp != myExternalIp )
    {
        externIpChanged = true;
        setExternalIpAddress( myExternalIp );
    }

    if( m_RequriesRelay != requiresRelay )
    {
        requresRelayChanged = true;
        m_RequriesRelay = requiresRelay;
    }

    if( m_DirectConnectTested != isTested )
    {
        directConnectTestedChanged = true;
        m_ConnectionTestAvail = isTested;
        m_DirectConnectTested = isTested;
    }

    if( requresRelayChanged || directConnectTestedChanged || externIpChanged )
    {
        // if assumed no firewall then network ready was called when extern ip was set
        if( m_Engine.getEngineSettings().getFirewallTestSetting() != eFirewallTestAssumeNoFirewall )
        {
            // no need to wait because open port is assumed
            m_Engine.onNetworkConnectionReady( requiresRelay, myExternalIp, m_IpPort );
        } 

        onNetStatusChange();
    }
}

//============================================================================
void NetStatusAccum::setConnectToRelay( bool connectedToRelay )
{
    if( connectedToRelay != m_ConnectedToRelay )
    {
        m_ConnectedToRelay = connectedToRelay;
        LogModule( eLogNetworkState, LOG_VERBOSE, "Connected to relay %d", connectedToRelay );
        onNetStatusChange();
    }
}

//============================================================================
void NetStatusAccum::setFirewallTestType( EFirewallTestType firewallTestType )
{
    if( firewallTestType != m_FirewallTestType )
    {
        m_AccumMutex.lock();
        m_FirewallTestType = firewallTestType;
        m_AccumMutex.unlock();

        onNetStatusChange();
    }
}

//============================================================================
EFirewallTestType NetStatusAccum::getFirewallTestType( void )
{
    m_AccumMutex.lock();
    EFirewallTestType firewallTestType = m_FirewallTestType;
    m_AccumMutex.unlock();
    return firewallTestType;
}

//============================================================================
void NetStatusAccum::getNodeUrl( std::string& retNodeUrl )
{
    retNodeUrl = "";

    if( isInternetAvailable() )
    {
        std::string ipAddr = getExternalIpAddress();
        EIpAddrType addrType = VxGetIpAddrType( ipAddr.c_str() );
        if( addrType != eIpAddrTypeUnknown && VxIsPortValid( m_IpPort ) )
        {
            VxMakePtopUrl( ipAddr, m_IpPort, retNodeUrl );
        }
    }
}

//============================================================================
uint16_t NetStatusAccum::getIpPort( void )
{
    uint16_t port = 0;
    m_AccumMutex.lock();
    port = m_IpPort;
    m_AccumMutex.unlock();
    return port;
}

//============================================================================
void NetStatusAccum::setJoinedHost( EHostType hostType, std::string hostUrl, VxGUID& connectId )
{
    m_AccumMutex.lock();
    auto item = m_HostConnectionMap.find( hostType );
    if( item != m_HostConnectionMap.end() ) 
    {
        item->second = HostConnection( hostType, hostUrl, connectId );
    }
    else 
    {
        m_HostConnectionMap.emplace( std::make_pair( hostType, HostConnection( hostType, hostUrl, connectId ) ) );
    }

    m_AccumMutex.unlock();
}

//============================================================================
bool NetStatusAccum::isConnectedToHost( EHostType hostType )
{
    bool isConnected = false;
    m_AccumMutex.lock();
    auto item = m_HostConnectionMap.find( hostType );
    if( item != m_HostConnectionMap.end() )
    {
        if( item->second.getConnectionId().isValid() )
        {
            isConnected = true;
        }
    }

    m_AccumMutex.unlock();
    return isConnected;
}

//============================================================================
void NetStatusAccum::onConnectionLost( VxGUID& connectId )
{
    m_AccumMutex.lock();
    for( auto& hostConn : m_HostConnectionMap )
    {
        if( hostConn.second.getConnectionId() == connectId )
        {
            hostConn.second.clearConnectionId();
        }
    }
    
    m_AccumMutex.unlock();
}

//============================================================================
std::string NetStatusAccum::getConnectedHostUrl( EHostType hostType )
{
    std::string hostUrl;
    m_AccumMutex.lock();
    auto item = m_HostConnectionMap.find( hostType );
    if( item != m_HostConnectionMap.end() )
    {
        if( item->second.getConnectionId().isValid() )
        {
            hostUrl = item->second.getHostUrl();
        }
    }

    m_AccumMutex.unlock();
    return hostUrl;
}

//============================================================================
void NetStatusAccum::setIpPort( uint16_t ipPort )
{
    if( VxIsPortValid( ipPort ) )
    {
        bool changedPort{false};
        m_AccumMutex.lock();
        if( m_IpPort != ipPort )
        {
            m_IpPort = ipPort;
            changedPort = true;
        }

        m_AccumMutex.unlock();
        if( changedPort )
        {
            m_Engine.getPeerMgr().setListenPort( ipPort );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::setIpPort invalid port %s", ipPort );
    }
}

//============================================================================
void NetStatusAccum::setExternalIpAddress( std::string ipAddr )
{
    if( VxIsIpValid( ipAddr ) )
    {
        bool changedIp{false};
        m_AccumMutex.lock();
        if( m_ExternAddr != ipAddr )
        {
            m_ExternAddr = ipAddr;
            changedIp = true;
        }

        m_AccumMutex.unlock();
        if( changedIp )
        {
            m_Engine.lockAnnouncePktAccess();
            m_Engine.getMyPktAnnounce().setOnlineIpAddress( ipAddr.c_str() );
            std::string myNodeUrl = m_Engine.getMyPktAnnounce().getMyOnlineUrl();
            m_Engine.unlockAnnouncePktAccess();

            m_Engine.getUrlMgr().setMyOnlineNodeUrl( myNodeUrl );
            m_Engine.getPluginMgr().onMyOnlineUrlIsValid( true );
            if( m_Engine.getEngineSettings().getFirewallTestSetting() == eFirewallTestAssumeNoFirewall )
            {
                // no need to wait because open port is assumed
                m_Engine.onNetworkConnectionReady( false, ipAddr, m_IpPort );
            }          
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::%s invalid ip %s", __func__, ipAddr.c_str() );
    }
}

//============================================================================
std::string NetStatusAccum::getExternalIpAddress( void )
{
    m_AccumMutex.lock();
    std::string ipAddr = m_ExternAddr;
    m_AccumMutex.unlock();
    return ipAddr;
}

//============================================================================
void NetStatusAccum::setLocalIpAddress( std::string ipAddr )
{
    if( VxIsIpValid( ipAddr ) )
    {
        bool changedIp{false};
        m_AccumMutex.lock();
        if( m_LocalAddr != ipAddr )
        {
            m_LocalAddr = ipAddr;
            changedIp = true;
        }

        m_AccumMutex.unlock();
    }
    else
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::%s invalid ip %s", __func__, ipAddr.c_str() );
    }
}

//============================================================================
std::string NetStatusAccum::getLocalIpAddress( void )
{
    m_AccumMutex.lock();
    std::string ipAddr = m_LocalAddr;
    m_AccumMutex.unlock();
    return ipAddr;
}

//============================================================================
std::string NetStatusAccum::getLocalIpAddress( bool ipv6 )
{
    m_AccumMutex.lock();
    std::string ipAddr = ipv6 ? m_LocalAddrIpv6 : m_LocalAddrIpv4;
    m_AccumMutex.unlock();
    return ipAddr;
}

//============================================================================
std::string NetStatusAccum::getLocalIpv4( void )
{
    m_AccumMutex.lock();
    std::string ipAddr = m_LocalAddrIpv4;
    m_AccumMutex.unlock();
    return ipAddr;
}

//============================================================================
std::string NetStatusAccum::getLocalIpv6( void )
{
    m_AccumMutex.lock();
    std::string ipAddr = m_LocalAddrIpv6;
    m_AccumMutex.unlock();
    return ipAddr;
}

//============================================================================
std::string NetStatusAccum::getMyNetServiceIpAddress( bool isIpv6Connection )
{
    std::string rxIpAddr = getExternalIpAddress();
    bool isIpv6 = VxIsIPv6Address(rxIpAddr.c_str());
    if( isIpv6 != isIpv6Connection )
    {
        std::string localIp = isIpv6Connection ? getLocalIpv6() : getLocalIpv4();
        if( localIp.empty() )
        {
            LogMsg( LOG_WARN, "NetStatusAccum::%s: alternative net serve ip is empty", __func__ );
        }
        else
        {
            InetAddress inetAddress;
            inetAddress.setIp( localIp.c_str() );
            if(inetAddress.isLocalAddress())
            {
                LogMsg( LOG_WARN, "NetStatusAccum::%s: alternative net serve ip is LAN only", __func__ );
            }
            else
            {
                rxIpAddr = localIp;
            }
        }
    }

    return rxIpAddr;
}

//============================================================================
void NetStatusAccum::setUseFixedIp( std::string externIp )
{
    m_AccumMutex.lock();
    m_HasFixedIpAddr = true;
    m_ExternAddr = externIp;
    m_AccumMutex.unlock();
}

//============================================================================
void NetStatusAccum::setUseIpv6( bool useIpv6, uint16_t ipPort )
{
    // this function will only get called on startup or engine settings changed

    // use it to get default local addresses

    bool portChanged{ false };
    bool useIpv6Changed{ false };
    bool hostServicesChanged{ false };
    lockAccum();
    if( ipPort != m_IpPort )
    {
        m_IpPort = ipPort;
        portChanged = true;
    }

    if( useIpv6 != m_UseIpv6 )
    {
        m_UseIpv6 = useIpv6;
        useIpv6Changed = true;
    }

    bool isConnectTestHost = m_Engine.getMyPktAnnounce().getPluginPermission( ePluginTypeHostConnectTest ) != eFriendStateIgnore;
    if( isConnectTestHost != m_IsConnectTestHost )
    {
        m_IsConnectTestHost = isConnectTestHost;
        hostServicesChanged = true;
    }

    bool isNetworkHost = m_Engine.getMyPktAnnounce().getPluginPermission( ePluginTypeHostNetwork ) != eFriendStateIgnore;
    if( isNetworkHost != m_IsNetworkHost )
    {
        m_IsNetworkHost = isNetworkHost;
        hostServicesChanged = true;
    }

    unlockAccum();
    if( portChanged )
    {
        m_Engine.getMyPktAnnounce().setOnlinePort( ipPort );
        m_Engine.setPktAnnLastModTime( GetTimeStampMs() );
    }

    if( hostServicesChanged || portChanged || useIpv6Changed || m_LocalAddrIpv6.empty() || m_LocalAddrIpv4.empty() )
    {
        // launch thread that will detect local ips and start listen thread(s)
        lockAccum();
        m_InternetAvail = false;
        unlockAccum();
        startSetupListenThread();
    }
}

//============================================================================
void NetStatusAccum::startSetupListenThread( void )
{
    if( !m_SetupListenThread.isThreadRunning() )
    {
        m_SetupListenThread.startThread( ( VX_THREAD_FUNCTION_T )NetStatusAccumThreadFunc, this, "NetStatusAccumThread" );
    }
}

//============================================================================
void NetStatusAccum::threadedSetupListen( void )
{
    lockAccum();
    uint16_t ipPort = m_IpPort;
    bool useIpv6 = m_UseIpv6;
    bool isConnectTestHost = m_IsConnectTestHost;
    bool isNetworkHost = m_IsNetworkHost;
    std::string origLocalIp = m_LocalAddr;
    std::string origLocalIp4 = m_LocalAddrIpv4;
    std::string origLocalIp6 = m_LocalAddrIpv6;
    unlockAccum();

    std::string localAddrIpv4;
    std::string localAddrIpv6;
    std::string useThisLocaIpAddr;

    bool addrChanged{ false };
    bool foundLclIp{ false };
    for( int i = 0; i < 3; i++ )
    {
        if( useIpv6 || isConnectTestHost || isNetworkHost )
        {
            // some systems do not support ipv6 and take a long time to fail
            // so only retrieve ipv6 if needed
            VxGetDefaultLocalIp( true, localAddrIpv6 );
        }

        VxGetDefaultLocalIp( false, localAddrIpv4 );

        useThisLocaIpAddr = useIpv6 ? localAddrIpv6 : localAddrIpv4;
        if( !useThisLocaIpAddr.empty() )
        {
            foundLclIp = true;
            break;
        }
        else
        {
            LogMsg( LOG_ERROR, "NetStatusAccum::%s: FAILED to retrieve local ip retrying", __func__ );
            VxSleep( 1000 );
        }
    }

    addrChanged = origLocalIp != useThisLocaIpAddr || origLocalIp4 != localAddrIpv4 || origLocalIp6 != localAddrIpv6;
    if( addrChanged && foundLclIp )
    {
        lockAccum();
        m_LocalAddr = useThisLocaIpAddr;
        m_LocalAddrIpv4 = localAddrIpv4;
        m_LocalAddrIpv6 = localAddrIpv6;
        unlockAccum();
    }

    if( !useThisLocaIpAddr.empty() )
    {
        m_Engine.getPeerMgr().setLocalIp( useThisLocaIpAddr );

        bool needPortForward = GetPtoPEngine().getEngineSettings().getUseUpnp();
        if( needPortForward && GetPtoPEngine().getNetStatusAccum().getFirewallTestType() == eFirewallTestAssumeNoFirewall )
        {
            std::string externalIp = GetPtoPEngine().getEngineSettings().getUserSpecifiedExternIpAddr( useIpv6 );
            if( useThisLocaIpAddr == externalIp )
            {
                needPortForward = false;
            }
        }

        m_Engine.getPeerMgr().startListening( useIpv6, ipPort, needPortForward );

        // if we have a local ip address we have internet
        setInternetAvail( true );
        LogMsg( LOG_VERBOSE, "NetStatusAccum::%s: Listening on port %d local ip %s", __func__, ipPort, useThisLocaIpAddr.c_str() );

        // listen on both ipv4 and ipv6 if possible
        if( isConnectTestHost || isNetworkHost )
        {
            // if we are a connection test host then try to listen for both ipv4 and ipv6
            // this is not required but convenient for testing
            
            if( needPortForward )
            {
                std::string otherLclIp = useIpv6 ? localAddrIpv4 : localAddrIpv6;
                if( !otherLclIp.empty() )
                {
                    m_Engine.getPeerMgr().startListening( !useIpv6, ipPort, needPortForward );
                    LogMsg( LOG_VERBOSE, "NetStatusAccum::%s: Also listening on port %d local ip %s", __func__, ipPort, otherLclIp.c_str() );
                }
            }
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "NetStatusAccum::%s: NO INTERNET ACCESS", __func__ );
    }
}

//============================================================================
bool NetStatusAccum::isHostResolved( EHostType hostType )
{
    VxGUID retHostOnlineId;
    if( m_Engine.getConnectionMgr().getDefaultHostOnlineId( hostType, retHostOnlineId ) )
    {
        return retHostOnlineId.isValid();
    }

    return false;
}

//============================================================================
bool NetStatusAccum::getResolvedHost( EHostType hostType, std::string& retHostUrl )
{
    VxGUID hostOnlineId;
    if( m_Engine.getConnectionMgr().getDefaultHostOnlineId( hostType, hostOnlineId ) && hostOnlineId.isValid() )
	{
        return m_Engine.getHostUrlListMgr().getResolvedHostUrl( hostType, hostOnlineId, retHostUrl );
	} 

    return false;
}

//============================================================================
void NetStatusAccum::setNetworkKey( std::string networkName )
{
    lockAccum();
    m_NetworkKey = networkName;
    unlockAccum();
}

//============================================================================
std::string NetStatusAccum::getNetworkKey( void )
{
    lockAccum();
    std::string netKey = m_NetworkKey;
    unlockAccum();
    return netKey;
}

//============================================================================
void NetStatusAccum::setNetworkHostUrl( std::string netHostUrl )
{
    lockAccum();
    m_NetHostUrl = netHostUrl;
    unlockAccum();

    std::string webHostName;
    std::string webFileName;
    uint16_t port = 0;
    VxSplitHostAndFile( netHostUrl.c_str(), webHostName, webFileName, port );
    if( !webHostName.empty() )
    {
        lockAccum();
        m_NetHostName = webHostName;
        if( port )
        {
            m_NetHostPort = port;
        }

        unlockAccum();
    }
}

//============================================================================
std::string NetStatusAccum::getNetworkHostUrl( void )
{
    lockAccum();
    std::string netHostUrl = m_NetHostUrl;
    unlockAccum();
    return netHostUrl;
}

//============================================================================
std::string NetStatusAccum::getNetworkHostName( void ) 
{
    lockAccum();
    std::string netHostName = m_NetHostName;
    unlockAccum();
    return netHostName;
}

//============================================================================
uint16_t NetStatusAccum::getNetworkHostPort( void )
{
    lockAccum();
    uint16_t netHostPort = m_NetHostPort;
    unlockAccum();
    return netHostPort;
}

//============================================================================
void NetStatusAccum::setConnectionTestHostUrl( std::string connectTestUrl )
{
    lockAccum();
    m_ConnectionTestUrl = connectTestUrl;
    unlockAccum();

    std::string webHostName;
    std::string webFileName;
    uint16_t port = 0;
    VxSplitHostAndFile( connectTestUrl.c_str(), webHostName, webFileName, port );
    if( !webHostName.empty() )
    {
        lockAccum();
        m_ConnectionTestHostName = webHostName;
        if( port )
        {
            m_ConnectionTestPort = port;
        }

        unlockAccum();
    }
}

//============================================================================
std::string NetStatusAccum::getConnectionTestHostUrl( void )
{
    lockAccum();
    std::string connectTestUrl = m_ConnectionTestUrl;
    unlockAccum();
    return connectTestUrl;
}


//============================================================================
std::string NetStatusAccum::getConnectionTestHostName( void ) 
{
    lockAccum();
    std::string connectTestHostName = m_ConnectionTestHostName;
    unlockAccum();
    return connectTestHostName;
}

//============================================================================
uint16_t NetStatusAccum::getConnectionTestHostPort( void )
{
    lockAccum();
    uint16_t connectTestPort = m_ConnectionTestPort;
    unlockAccum();
    return connectTestPort;
}

//============================================================================
bool NetStatusAccum::canAnnounceToNlcHost( bool iAmNetHost )
{
    if( !isRxPortOpen() )
    {
        return false;
    }

    bool isNlcHostUrl = hasDefaultNetworkUrl();
    bool isNlcNetworkKey = hasDefaultNetworkKey();
    if( iAmNetHost )
    {
        // if I am a network host can I still announce to the NLC network host?
        if( !isNlcHostUrl )
        {
            return false;
        }
    }

    // do not announce to NLC network host if using different network key or will get banned as hacker
    if( isNlcHostUrl && !isNlcNetworkKey )
    {
        return false;
    }
    
    return true;
}

//============================================================================
bool NetStatusAccum::hasDefaultNetworkKey( void )
{
    return getNetworkKey() == NET_DEFAULT_NETWORK_KEY;
}

//============================================================================
bool NetStatusAccum::hasDefaultNetworkUrl( void )
{
    return (getNetworkHostUrl() == NET_DEFAULT_NET_HOST_URL_IPV4 || getNetworkHostUrl() == NET_DEFAULT_NET_HOST_URL_IPV6);
}

//============================================================================
bool NetStatusAccum::isLocalAndExternIpsTheSame( void )
{
    std::string ipExternAddr = getExternalIpAddress();
    return !ipExternAddr.empty() && getLocalIpAddress() == ipExternAddr;
}
