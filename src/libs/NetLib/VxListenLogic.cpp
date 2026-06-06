//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxListenLogic.h"

#include "VxServerMgr.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/AppErr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxThread.h>

#include <stdio.h>
#include <memory.h>
#include <time.h>

#if defined(TARGET_OS_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif // defined(TARGET_OS_WINDOWS)

//============================================================================
namespace
{
    const int MAX_LISTEN_BACKLOG = 4;
    const int INACTIVE_LISTEN_RESTART_MS = 15 * 60 * 1000; // if no listen activity for 15 minutes reopen listen socket

    void* VxListenLogicVxThreadFunc( void* pvContext )
    {
        if( pvContext )
        {
            VxThread* vxThread = (VxThread*)pvContext;
            vxThread->setIsThreadRunning( true );
            VxListenLogic* listenLogic = (VxListenLogic*)vxThread->getThreadUserParam();
            if( listenLogic )
            {
                LogModule( eLogConnect, LOG_INFO, "#### VxListenLogic: ipv6 %d Listen port %d thread started thread 0x%x",
                    listenLogic->getIsIpv6(), listenLogic->getListenPort(), VxGetCurrentThreadId() );
                listenLogic->listenForConnectionsToAccept();
                // quitting
                LogModule( eLogConnect, LOG_INFO, "#### VxListenLogic: ipv6 %d Listen port %d thread 0x%x tid %d quiting",
                    listenLogic->getIsIpv6(), listenLogic->getListenPort(), VxGetCurrentThreadId(), vxThread->getThreadTid() );
            }

            //! VxThread calls this just before exit
            vxThread->threadAboutToExit();
        }

        return nullptr;
    }
}

//============================================================================
VxListenLogic::VxListenLogic( VxServerMgr& serverMgr, bool ipv6 )
    : m_ServerMgr( serverMgr )
    , m_IsIpv6( ipv6 )
{
}

//============================================================================
bool VxListenLogic::startListeningThread( void )
{
    int32_t rc = 0;
    std::string threadName;
    if( m_IsIpv6 )
    {
        StdStringFormat( threadName, "VxListenLogic%dIPv6", m_ServerMgr.getMgrId() );
    }
    else
    {
        StdStringFormat( threadName, "VxListenLogic%dIPv4", m_ServerMgr.getMgrId() );
    }

    rc = m_ListenThread.startThread( (VX_THREAD_FUNCTION_T)VxListenLogicVxThreadFunc, this, threadName.c_str() );

    return rc == 0;
}

//============================================================================
void VxListenLogic::stopListeningThread( void )
{
    setIsReadyToAcceptConnections( false );
    // set thread to abort
    m_ListenThread.abortThreadRun( true );
    closeListenSocket();

    if( m_ListenThread.isThreadRunning() )
    {
        VxSleep( 300 ); // give accept time to abort before closing the listen socket
        // give thread a while to exit
        m_ListenThread.killThread();
    }

    if( m_ListenThread.isThreadRunning() )
    {
        LogMsg( LOG_FATAL, "%s could not stop listen thread", __func__ );
    }
}

//============================================================================
void VxListenLogic::sktMgrShutdown( void )
{
    stopListeningThread();
}

//============================================================================
bool VxListenLogic::startListening( uint16_t port )
{
    stopListening();
    m_ListenPort = port;
    if( m_ListenPort && startListeningThread() )
    {
        return true;
    }

    return false;
}

//============================================================================
void VxListenLogic::stopListening( void )
{
    m_ListenThread.abortThreadRun( true );

    stopListeningThread();
    closeListenSocket();
}

//============================================================================
bool VxListenLogic::getIsListening( void )
{
    if( !m_ListenPort )
    {
        return false;
    }

    return m_ListenThread.isThreadRunning();
}

//============================================================================
// called from thread
void VxListenLogic::listenForConnectionsToAccept( void )
{
    LogMsg( LOG_INFO, "VxListenLogic::%s port %d ipv6 %d started", __func__, m_ListenPort, m_IsIpv6 );

start_over:
    closeListenSocket();

    if( shouldListenAbort() )
    {
        return;
    }

    SOCKET listenSock = INVALID_SOCKET;
    if( !createNewListenSocket( listenSock ) )
    {
        LogMsg( LOG_ERROR, "VxServerMgr::%s failed create ipv6 %d listen socket port %d", __func__, m_IsIpv6, getListenPort() );
        return;
    }

    setListenSkt( listenSock );

    if( shouldListenAbort() )
    {
        return;
    }

    m_LastListenActivityMs = GetGmtTimeMs();

    // for some unknown reason select code that works on mac/windows/linux does not work on android
    // on android when use select the select seems to work but in the accept it gets error 22 (invalid param) .. so do this crap
    while( !shouldListenAbort() )
    {
        int32_t rc = 0;

        int listenResult = listen( getListenSkt(), MAX_LISTEN_BACKLOG );
        if( 0 > listenResult )
        {
            rc = VxGetLastError();
#if defined(TARGET_OS_WINDOWS)
            if( rc == WSAEWOULDBLOCK )
            {
                rc = EAGAIN;
            }
#endif // defined(TARGET_OS_WINDOWS)
        }

        if( shouldListenAbort() )
        {
            LogModule( eLogConnect, LOG_DEBUG, "%s: aborting1", __func__ );
            break;
        }

        if( rc )
        {
            if( rc == EAGAIN )
            {
                static int listenErrCnt = 0;
                listenErrCnt++;
                if( listenErrCnt > 50 )
                {
                    listenErrCnt = 0;
                    LogModule( eLogConnect, LOG_DEBUG, "%s: try again: listen ipv6 %d port %d skt %d error %d thread 0x%x", __func__, m_IsIpv6, m_ListenPort, getListenSkt(), rc, VxGetCurrentThreadId() );
                }

                VxSleep( 200 );
                if( shouldListenAbort() )
                {
                    LogModule( eLogConnect, LOG_DEBUG, "%s: aborting2", __func__ );
                    break;
                }

                continue;
            }
            else
            {
                LogMsg( LOG_DEBUG, "listen: ERROR %s", VxDescribeSktError( rc ) );

                VxSleep( 500 );
                if( shouldListenAbort() )
                {
                    LogModule( eLogConnect, LOG_DEBUG, "%s: aborting3", __func__ );
                    break;
                }

                // probably we lost internet connection for some reason.. create a new socket and try again
                goto start_over;
            }
        }

        if( shouldListenAbort() || INVALID_SOCKET == getListenSkt() )
        {
            LogModule( eLogConnect, LOG_DEBUG, "%s: aborting5", __func__ );
            break;
        }

        int32_t acceptResult = m_ServerMgr.acceptConnection( m_IsIpv6, &m_ListenThread, getListenSkt(), m_ListenPort );
        if( acceptResult )
        {
            LogMsg( LOG_DEBUG, "ListenLogic::%s acceptConnection ERROR %d %s", __func__, acceptResult, VxDescribeSktError( acceptResult ) );
        }
    }

    m_ServerMgr.setIsReadyToAcceptConnections( m_IsIpv6, false );

    closeListenSocket();

    LogModule( eLogConnect, LOG_INFO, "Listen Thread is exiting thread 0x%x", VxGetCurrentThreadId() );
}

//============================================================================
bool VxListenLogic::createNewListenSocket( SOCKET& retListenSock )
{
    // some vpns get confused if the same socket number is reused after closed so make a dummy socket so socket number is incremented
    SOCKET dummySock = socket( m_IsIpv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0 );               // creates IP based TCP socket
    if( dummySock <= 0 )
    {
        LogMsg( LOG_ERROR, "VxListenLogic::%s failed", __func__ );
        return false;
    }

    SOCKET listenSock = socket( m_IsIpv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0 );               // creates IP based TCP socket
    if( listenSock <= 0 )
    {
        LogMsg( LOG_ERROR, "VxListenLogic::%s failed", __func__ );
        VxCloseSkt( dummySock );
        return false;
    }

    VxCloseSkt( dummySock );

    NetStatusAccum& netStatusAccum = GetPtoPEngine().getNetStatusAccum();

    std::string bindLclIp; // seems that VPNs work better without the bind to a specific address

    if( ( netStatusAccum.getIsConnectTestHost() || netStatusAccum.getIsNetworkHost() ) &&
        eFirewallTestAssumeNoFirewall == netStatusAccum.getFirewallTestType() )
    {
        // if we are a connection service or network host service and assumed no firewall
        // then need to bind to a specific address or we will recieve ipv4 connections on ipv6 listen
        // socket which gets the wrong remote ip for connection tests
        bindLclIp = m_IsIpv6 ? netStatusAccum.getLocalIpv6() : netStatusAccum.getLocalIpv4();
    }

    struct sockaddr_storage sockAddrStorage;
    struct sockaddr* sockAddr = reinterpret_cast<sockaddr*>( &sockAddrStorage );

    socklen_t sockAddrLen = VxSktAddrInit( m_IsIpv6, sockAddrStorage, bindLclIp, m_ListenPort );

    // Bind Socket
    int bindStatus = bind( listenSock, sockAddr, sockAddrLen );
    int retryCnt = 0;
    while( bindStatus < 0 )
    {
        retryCnt++;
        if( retryCnt >= 2 )
        {
            LogMsg( LOG_ERROR, "VxServerMgr::%s bind socket %d failed event after %d tries", __func__, listenSock, retryCnt );
            break;
        }

        VxSleep( 500 );
        bindStatus = bind( listenSock, (struct sockaddr*)&sockAddr, sockAddrLen );
    }

    VxSetSktAllowReuseAddress( listenSock );

    // android set listen skt back to blocking doesn't work so just set to non blocking always ( part of accept hang fix ) 
    //VxSetSktBlocking( listenSock, false );

    m_ServerMgr.listenSktWasOpened( m_IsIpv6, listenSock );

    retListenSock = listenSock;

    return true;
}

//============================================================================
bool VxListenLogic::shouldListenAbort( void )
{
    if( VxIsAppShuttingDown() )
    {
        return true;
    }

    return  m_ListenThread.isAborted();
}

//============================================================================
void VxListenLogic::closeListenSocket( void )
{
    SOCKET sktToClose = getListenSkt();
    if( INVALID_SOCKET != sktToClose )
    {
        setIsReadyToAcceptConnections( false );
        setListenSkt( INVALID_SOCKET );
#if !defined(TARGET_OS_LINUX)
        VxSetSktBlocking( sktToClose, false ); // so should release accept but seems to hang sometimes
#endif // defined(TARGET_OS_LINUX)

        LogModule( eLogConnect, LOG_INFO, "VxListenLogic:listenForConnectionsToAccept closing listen skt %d", sktToClose );

        // set the socket to reuse or even though closed the system may not allow another listen on that port to be done
        // until the system has completely cleaned it up
        char setTrue = 1;
        setsockopt( sktToClose, SOL_SOCKET, SO_REUSEADDR, &setTrue, sizeof( char ) );

        ::VxCloseSkt( sktToClose );

        m_ServerMgr.listenSktWasClosed( m_IsIpv6, sktToClose );
    }
}

//============================================================================
uint16_t VxListenLogic::getListenPort( void )
{
    lockListenSettings();
    uint16_t port = m_ListenPort;
    unlockListenSettings();

    return port;
}

//============================================================================
void VxListenLogic::setListenSkt( SOCKET skt )
{
    lockListenSettings();
    m_ListenSocket = skt;
    unlockListenSettings();
}

//============================================================================
SOCKET VxListenLogic::getListenSkt( bool setExistingSktToInvalid )
{
    lockListenSettings();
    SOCKET skt = m_ListenSocket;
    if( setExistingSktToInvalid )
    {
        m_ListenSocket = INVALID_SOCKET;
    }

    unlockListenSettings();

    return skt;
}

//============================================================================
void VxListenLogic::setIsReadyToAcceptConnections( bool isReady )
{
    lockListenSettings();
    m_IsReadyToAcceptConnections = isReady;
    unlockListenSettings();

    m_ServerMgr.setIsReadyToAcceptConnections( m_IsIpv6, isReady );
}

//============================================================================
bool VxListenLogic::getIsReadyToAcceptConnections( void )
{
    lockListenSettings();
    bool isReadyToAccept = m_IsReadyToAcceptConnections;
    unlockListenSettings();

    return isReadyToAccept;
}
