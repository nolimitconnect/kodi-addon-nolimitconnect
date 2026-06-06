//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxServerMgr.h"

#include "VxSktAccept.h"
#include "VxPortForward.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/AppErr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxThread.h>

#include <memory.h>

#if defined(TARGET_OS_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif // defined(TARGET_OS_WINDOWS)

//#define DEBUG_VXSERVER_MGR
#define DISABLE_WATCHDOG 1

int VxServerMgr::m_iAcceptMgrCnt = 0;				// number of accept managers that have been created

//============================================================================
VxServerMgr::VxServerMgr()
    : VxSktBaseMgr()
    , m_ListenLogicIpv4( *this, false )
    , m_ListenLogicIpv6( *this, true )
{
    m_iAcceptMgrCnt++;
    m_iMgrId = m_iAcceptMgrCnt;
    m_eSktMgrType = eSktMgrTypeTcpAccept;
}

//============================================================================
void VxServerMgr::sktMgrStartup( bool ipv6 )
{
}

//============================================================================
void VxServerMgr::sktMgrShutdown( void )
{
    VxPortForward::shutdownPortForward();
    m_ListenLogicIpv4.sktMgrShutdown();
    m_ListenLogicIpv6.sktMgrShutdown();
    VxSktBaseMgr::sktMgrShutdown();
}

//============================================================================
std::shared_ptr<VxSktBase> VxServerMgr::makeNewAcceptSkt( bool ipv6 )
{
    std::shared_ptr<VxSktBase> sharedSkt( new VxSktAccept() );
    sharedSkt->setIsIpv6Connection( ipv6 );
    sharedSkt->setThisSkt( sharedSkt ); // so skt can do callbacks without look up in manager
    return sharedSkt;
}

//============================================================================
bool VxServerMgr::startListening( bool ipv6, uint16_t port, bool usePortForwardIfEnabled )
{
    setListenPort( port );
    // no need to use upnp if we assume no firewall
    if( usePortForwardIfEnabled )
    {
        addPortForward( ipv6, port );
    }

    return ipv6 ? m_ListenLogicIpv6.startListening( port ) : m_ListenLogicIpv4.startListening( port );
}

//============================================================================
void VxServerMgr::stopListening( bool ipv6 )
{
    ipv6 ? m_ListenLogicIpv6.stopListening() : m_ListenLogicIpv4.stopListening();
}

//============================================================================
bool VxServerMgr::isListening( bool ipv6 )
{
    return ipv6 ? m_ListenLogicIpv6.getIsListening() : m_ListenLogicIpv4.getIsListening();
}

//============================================================================
void VxServerMgr::listenSktWasOpened( bool ipv6, SOCKET listenSkt )
{
    if( m_pfnSktMgrStatus )
    {
        m_pfnSktMgrStatus( "ListenOpen", listenSkt, m_pvSktMgrStatusCallbackUserData );
    }
}

//============================================================================
void VxServerMgr::listenSktWasClosed( bool ipv6, SOCKET listenSkt )
{
    if( m_pfnSktMgrStatus )
    {
        m_pfnSktMgrStatus( "ListenClose", listenSkt, m_pvSktMgrStatusCallbackUserData );
    }
}

//============================================================================
int32_t VxServerMgr::acceptConnection( bool ipv6, VxThread* poVxThread, SOCKET listenSkt, uint16_t port )
{
    if( VxIsAppShuttingDown() )
    {
        return -1;
    }

    int32_t rc = 0;
    if( INVALID_SOCKET == listenSkt )
    {
        LogModule( eLogConnect, LOG_ERROR, "VxServerMgr::%s INVALID LISTEN SOCKET thread 0x%x", __func__, VxGetCurrentThreadId() );
        return -2;
    }

    if( poVxThread->isAborted() )
    {
        LogModule( eLogConnect, LOG_ERROR, "VxServerMgr::%s aborted accept thread 0x%x", __func__, VxGetCurrentThreadId() );
        return -3;
    }

    static int dumpAcceptCnt = 0;
    dumpAcceptCnt++;
    if( dumpAcceptCnt > 20 )
    {
        dumpAcceptCnt = 0;
        LogModule( eLogConnect, LOG_INFO, "VxServerMgr::%s start acceptConnection listen skt %d rc %d thread 0x%x", __func__, listenSkt, VxGetLastError(), VxGetCurrentThreadId() );
    }

    // perform accept
    // setup address
    struct sockaddr_storage acceptAddrStorage;
    memset( &acceptAddrStorage, 0, sizeof( struct sockaddr_storage ) );
    struct sockaddr* acceptAddr = reinterpret_cast<sockaddr*>( &acceptAddrStorage );
    socklen_t acceptAddrLen = sizeof( struct sockaddr_storage );

    if( poVxThread->isAborted() )
    {
        LogModule( eLogConnect, LOG_ERROR, "VxServerMgr::%s aborted accept2 thread 0x%x", __func__, VxGetCurrentThreadId() );
        return -5;
    }

    // TODO can still hang the thread if calls accept at the exact right moment of close listen socket

    // NOTE: in android the return to blocking on listen doesn't work so we just set it once before start listening so accept does not get hung
    SOCKET acceptSkt = accept( listenSkt, acceptAddr, &acceptAddrLen );

    if( poVxThread->isAborted() )
    {
        LogModule( eLogConnect, LOG_ERROR, "VxServerMgr::%s aborted accept3 thread 0x%x", __func__, VxGetCurrentThreadId() );
        return -4;
    }

    static int acceptErrCnt = 0;
    static int dumpSktStatsCnt = 0;
    if( INVALID_SOCKET == acceptSkt )
    {
        rc = VxGetLastError();
#if defined(TARGET_OS_WINDOWS)
        if( rc == WSAEWOULDBLOCK )
        {
            rc = EAGAIN;
        }
#endif // defined(TARGET_OS_WINDOWS)

        if( rc )
        {
            acceptErrCnt++;
            if( acceptErrCnt > 300 )
            {
                acceptErrCnt = 0;
                dumpSktStatsCnt++;
                if( rc != EAGAIN )
                {
                    LogModule( eLogConnect, LOG_DEBUG, "VxServerMgr::%s: listen port %d skt %d error %d thread 0x%x", __func__, port, listenSkt, rc, VxGetCurrentThreadId() );
                }

                if( dumpSktStatsCnt > 10 )
                {
                    dumpSktStatsCnt = 0;
                    dumpSocketStats( "full dump", true );
                }
                else
                {
                    dumpSocketStats();
                }
            }
        }
        else
        {
            acceptErrCnt = 0;
        }

        if( 0 == rc )
        {
            // not sure how it happens but seems to get in a loop where the clear doesn't clear and there is no error
            // so sleep just in case so doesn't eat up all the CPU
            LogModule( eLogConnect, LOG_INFO, "VxServerMgr:%s: no rc acceptConnection skt %d rc %d thread 0x%x", __func__, listenSkt, VxGetLastError(), VxGetCurrentThreadId() );
            VxSleep( 500 );
            return -1;
        }
        else if( EAGAIN == rc )
        {
            // non blocking operation could not be done immediate error
            VxSleep( 200 );
            static int intRetry1Cnt = 0;
            intRetry1Cnt++;
            if( intRetry1Cnt > 20 )
            {
                intRetry1Cnt = 0;
                LogModule( eLogConnect, LOG_INFO, "VxServerMgr::%s non blocking operation  acceptConnection skt %d rc %d thread 0x%x", __func__, listenSkt, VxGetLastError(), VxGetCurrentThreadId() );
            }

            return 0;
        }
        else
        {
            LogModule( eLogConnect, LOG_INFO, "VxServerMgr::%s other error acceptConnection skt %d rc %d thread 0x%x", __func__, listenSkt, VxGetLastError(), VxGetCurrentThreadId() );
            VxSleep( 200 );
            return rc;
        }
    }
    else
    {
        acceptErrCnt = 0;
    }

    LogModule( eLogConnect, LOG_DEBUG, "VxServerMgr::%s: listen skt %d accepted skt %d thread 0x%x", __func__, listenSkt, acceptSkt, VxGetCurrentThreadId() );
    if( poVxThread->isAborted() || VxIsAppShuttingDown() )
    {
        return -1;
    }

    // valid accept socket
    if( m_aoSkts.size() >= m_u32MaxConnections )
    {
        LogMsg( LOG_ERROR, "VxServerMgr: reached max connections %d thread 0x%x", m_u32MaxConnections, VxGetCurrentThreadId() );
        dumpSocketStats( "VxServerMgr" );
        // we have reached max connections
        // just close it immediately
        VxCloseSktNow( acceptSkt );

        doSktDeleteCleanup();
        // sleep awhile
        VxSleep( 200 );
        return 0; // keep running until number of connections clear up
    }

    struct sockaddr_storage peerAddrStorage;
    memset( &peerAddrStorage, 0, sizeof( peerAddrStorage ) );
    struct sockaddr* peerAddr = reinterpret_cast<sockaddr*>( &peerAddrStorage );
    socklen_t peerAddrLen = sizeof( struct sockaddr_storage );

    std::string rmtIp;

    if( 0 == getpeername( acceptSkt, peerAddr, &peerAddrLen ) )
    {
        InetAddress::getIpFromAddr( peerAddr, rmtIp );
    }

    if( rmtIp.empty() )
    {
        LogMsg( LOG_ERROR, "Failed to get remote ip for accept skt handle %d", acceptSkt );
        VxCloseSktNow( acceptSkt );

        return -9;
    }
    else if( isHacker( rmtIp ) )
    {
        LogModule( eLogHackers, LOG_INFO, "Hacker from IP %d attempted connect again", rmtIp.c_str() );
        VxCloseSktNow( acceptSkt );

        return -10;
    }

    // add a skt to our list	
    std::shared_ptr<VxSktBase> sktBase = makeNewAcceptSkt( VxIsIPv6Address( rmtIp.c_str() ) );
    if( sktBase.get() == nullptr )
    {
        LogMsg( LOG_ERROR, "%s makeNewAcceptSkt returned null", __func__ );
        VxCloseSktNow( acceptSkt );
        return -11;
    }

#if defined(DEBUG_SKT_MGR_LOCK)
    LogMsg( LOG_DEBUG, "VxServerMgr::%s lockSktBaseMgr", __func__ );
#endif // defined(DEBUG_SKT_MGR_LOCK)
    lockSktBaseMgr(); // dont let other threads mess with array while we add
#if defined(DEBUG_SKT_MGR_LOCK)
    LogMsg( LOG_DEBUG, "VxServerMgr::%s lockSktBaseMgr locked", __func__ );
#endif // defined(DEBUG_SKT_MGR_LOCK)
    m_aoSkts.emplace_back( sktBase );
    // do tell skt to do accept stuff
    sktBase->m_Socket = acceptSkt;
    sktBase->setReceiveCallback( m_pfnOurReceive, this );
    sktBase->setTransmitCallback( m_pfnOurTransmit, this );
#if defined(DEBUG_SKT_MGR_LOCK)
    LogMsg( LOG_DEBUG, "VxServerMgr::%s unlockSktBaseMgr", __func__ );
#endif // defined(DEBUG_SKT_MGR_LOCK)
    unlockSktBaseMgr();

    LogModule( eLogConnect, LOG_INFO, "VxServerMgr::%s doing accept skt handle %d skt id %d thread 0x%x", __func__, sktBase->m_Socket, sktBase->getSktNumber(), VxGetCurrentThreadId() );

    int32_t rcAccept = dynamic_cast<VxSktAccept*>( sktBase.get() )->doAccept( this, *acceptAddr );
    if( rcAccept || poVxThread->isAborted() || INVALID_SOCKET == listenSkt )
    {
        sktBase->closeSkt( eSktCloseAcceptFailed );
        LogMsg( LOG_ERROR, "VxServerMgr::%s error %d doing accept skt handle %d skt id %d thread 0x%x", __func__, rc, sktBase->m_Socket, sktBase->getSktNumber(), VxGetCurrentThreadId() );
        moveToEraseList( sktBase );
        return -12;
    }
    else
    {
        acceptErrCnt = 0; // reset counter
        m_LastListenActivityMs = GetGmtTimeMs();
        LogModule( eLogConnect, LOG_INFO, "VxServerMgr::%s accept success skt %d skt id %d thread 0x%x", __func__, sktBase->m_Socket, sktBase->getSktNumber(), VxGetCurrentThreadId() );
    }

    doSktDeleteCleanup();
    return rc;
}

//============================================================================
void VxServerMgr::setIsReadyToAcceptConnections( bool ipv6, bool isReady )
{
    lockListenSettings();
    ipv6 ? m_IsReadyToAcceptConnectionsIpv6 = isReady : m_IsReadyToAcceptConnectionsIpv4 = isReady;
    unlockListenSettings();
}

//============================================================================
bool VxServerMgr::getIsReadyToAcceptConnections( bool ipv6 )
{
    lockListenSettings();
    bool isReadyToAccept = ipv6 ? m_IsReadyToAcceptConnectionsIpv6 : m_IsReadyToAcceptConnectionsIpv4;
    unlockListenSettings();

    return isReadyToAccept;
}

//============================================================================
void VxServerMgr::setListenPort( uint16_t port )
{
    if( !port )
    {
        LogMsg( LOG_ERROR, "VxServerMgr::%s invalid port %d", __func__, port );
        return;
    }

    lockListenSettings();
    m_ListenPort = port;
    unlockListenSettings();
}

//============================================================================
uint16_t VxServerMgr::getListenPort( void )
{
    uint16_t port;
    lockListenSettings();
    port = m_ListenPort;
    unlockListenSettings();
    return port;
}

//============================================================================
void VxServerMgr::setLocalIp( std::string ipAddr )
{
    if( ipAddr.empty() )
    {
        LogMsg( LOG_ERROR, "VxServerMgr::%s empty ip addr", __func__ );
        return;
    }

    EIpAddrType addrType = VxGetIpAddrType( ipAddr.c_str() );
    if( eIpAddrTypeUnknown == addrType )
    {
        LogMsg( LOG_ERROR, "VxServerMgr::%s invalid ip addr %s", __func__, ipAddr.c_str() );
        return;
    }

    bool ipChanged{ false };
    lockListenSettings();
    if( eIpAddrTypeIpv6 == addrType )
    {
        if( m_LocalIpAddrIpv6 != ipAddr )
        {
            m_LocalIpAddrIpv6 = ipAddr;
            ipChanged = true;
        }
    }
    else if( m_LocalIpAddrIpv4 != ipAddr )
    {
        m_LocalIpAddrIpv4 = ipAddr;
        ipChanged = true;
    }

    unlockListenSettings();
    if( ipChanged )
    {
        //checkPortForward( eIpAddrTypeIpv6 == addrType );
    }
}

//============================================================================
std::string VxServerMgr::getLocalIp( bool ipv6 )
{
    std::string ipAddr;
    lockListenSettings();
    ipAddr = ipv6 ? m_LocalIpAddrIpv6 : m_LocalIpAddrIpv4;
    unlockListenSettings();
    return ipAddr;
}

//============================================================================
void VxServerMgr::setUpnpEnable( bool enable )
{
    if( enable != VxPortForward::getEnablePortForward() )
    {
        VxPortForward::setEnablePortForward( enable );
    }
}

//============================================================================
bool VxServerMgr::getUpnpEnable( void )
{
    return VxPortForward::getEnablePortForward();
}

//============================================================================
void VxServerMgr::setUseIpv6( bool enable )
{
    if( enable != VxPortForward::getUseIpv6() )
    {
        VxPortForward::setUseIpv6( enable );
    }
}

//============================================================================
bool VxServerMgr::getUseIpv6( void )
{
    return VxPortForward::getUseIpv6();
}

//============================================================================
bool VxServerMgr::addPortForward( bool ipv6, uint16_t port )
{
    m_PortForwardResult = false;
    if( !getUpnpEnable() )
    {
        return false;
    }

    if( !port )
    {
        LogModule( eLogPortForward, LOG_ERROR, "VxServerMgr::%s invalid port", __func__ );
        return false;
    }

    std::string lclIp = GetPtoPEngine().getNetStatusAccum().getLocalIpAddress( ipv6 );
    if( lclIp.empty() )
    {
        LogModule( eLogPortForward, LOG_ERROR, "VxServerMgr::%s no local address for %s", __func__, ipv6 ? "ipv6" : "ipv4" );
        return false;
    }

    if( ipv6 && !VxPortForward::getUseIpv6() )
    {
        LogModule( eLogPortForward, LOG_VERBOSE, "VxServerMgr::%s skip ipv6 port forward (not primary configuration)", __func__ );
        return false;
    }

    m_PortForwardResult = VxPortForward::addPortForward( ipv6, lclIp, port );
    if( !m_PortForwardResult )
    {
        LogMsg( LOG_ERROR, "VxServerMgr::%s Add ipv6 port %d for ip %s FAILED", __func__, port, lclIp.c_str() );
        return false;
    }

    return true;
}
