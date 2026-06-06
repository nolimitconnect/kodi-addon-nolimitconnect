//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include <AppMonitor/AppMonitor.h>
#include <Membership/MemberConfirmMgr.h>
#include <NetworkMonitor/NetworkMonitor.h>

#include <Network/NetworkMgr.h>
#include <NetServices/NetServicesMgr.h>
#include <Plugins/PluginMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxTime.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>

namespace
{
    //============================================================================
    void * P2PEngineTimerThreadFunc( void * pvContext )
    {
        VxThread* poThread = ( VxThread* )pvContext;
        poThread->setIsThreadRunning( true );
        P2PEngine * p2pEngine = ( P2PEngine * )poThread->getThreadUserParam();
        if( p2pEngine && false == poThread->isAborted() )
        {
            p2pEngine->executeTimerThreadFunctions();
        }

        poThread->threadAboutToExit();
        return nullptr;
    }
}

//============================================================================
void P2PEngine::enableTimerThread( bool enable )
{
    if( enable && !VxIsAppShuttingDown() )
    {
        static int threadStartCnt = 0;
        threadStartCnt++;
        std::string timerThreadName;
        StdStringFormat( timerThreadName, "TimerThread_%d", threadStartCnt );
        m_TimerThread.killThread();
        m_TimerThread.startThread( ( VX_THREAD_FUNCTION_T )P2PEngineTimerThreadFunc, this, timerThreadName.c_str() );

        LogModule( eLogThread, LOG_VERBOSE, "timer thread %d started", threadStartCnt );
    }
    else
    {
        m_TimerThreadSemaphore.signal();
        m_TimerThread.killThread();
    }
}

//============================================================================
void P2PEngine::executeTimerThreadFunctions( void )
{
    while( !m_TimerThread.isAborted() && !VxIsAppShuttingDown() )
    {
        m_TimerThreadSemaphore.wait();
        if( !m_TimerThread.isAborted() && !VxIsAppShuttingDown() )
        {
            onOncePerSecond();
        }
        else
        {
            break;
        }
    }
}

//============================================================================
void P2PEngine::fromGuiOncePerSecond( void )
{
    m_TimerThreadSemaphore.signal();
}

//============================================================================
void P2PEngine::onOncePerSecond( void )
{
    if( VxIsAppShuttingDown() )
    {
        return;
    }

    m_NetworkMonitor.onOncePerSecond();
    m_PluginMgr.onOncePerSecond();
    GetMemberConfirmMgr().onOncePerSecond();

    static int thirtySecCntInSeconds = 31;
    thirtySecCntInSeconds--;
    if( 0 >= thirtySecCntInSeconds )
    {
        thirtySecCntInSeconds = 30;
        onOncePer30Seconds();
    }

    static int minuteCntInSeconds = 62;
    minuteCntInSeconds--;
    if( 0 >= minuteCntInSeconds )
    {
        minuteCntInSeconds = 60;
        onOncePerMinute();
    }

    static int minute5CntSeconds = 60 * 5 + 3;
    minute5CntSeconds--;
    if( 0 >= minute5CntSeconds )
    {
        minute5CntSeconds = 60 * 5;
        onOncePer5Minutes();
    }

    static int minute10CntSeconds = 60 * 10 + 4;
    minute10CntSeconds--;
    if( 0 >= minute10CntSeconds )
    {
        minute10CntSeconds = 60 * 10;
        onOncePer10Minutes();
    }

    static int minute15CntSeconds = 60 * 15 + 5;
    minute15CntSeconds--;
    if( 0 >= minute15CntSeconds )
    {
        minute15CntSeconds = 60 * 15;
        onOncePer15Minutes();
    }

    static int minute30CntSeconds = 60 * 30 + 6;
    minute30CntSeconds--;
    if( 0 >= minute30CntSeconds )
    {
        minute30CntSeconds = 60 * 30;
        onOncePer30Minutes();
    }

    static int hourCntInSeconds = 60 * 60 + 7;
    hourCntInSeconds--;
    if( 0 >= hourCntInSeconds )
    {
        hourCntInSeconds = 3600;
        onOncePerHour();
    }
}

//============================================================================
void P2PEngine::onOncePer30Seconds( void )
{
    //LogMsg( LOG_VERBOSE, "P2PEngine::onOncePer30Seconds" );
    m_RcScan.onOncePer30Seconds();
    m_PeerMgr.onOncePer30Seconds( getMyOnlineId() );
}

//============================================================================
void P2PEngine::onOncePerMinute( void )
{
    m_RcScan.onOncePerMinute();

    static bool firstMinute = true;
    if( firstMinute || VxGetFastHostAnnounce() )
    {
        firstMinute = false;
        // this is so announcement of hosts start in a minute instead of waiting 
        // the full 15 minute before the first announce
        m_PluginMgr.onThreadOncePer15Minutes();
    }
}

//============================================================================
void P2PEngine::onOncePer5Minutes( void )
{

}

//============================================================================
void P2PEngine::onOncePer10Minutes( void )
{
    AppMonitor::dumpAppStats();
    if( LogEnabled( eLogThread ) )
    {
        VxThread::dumpRunningThreads();
    }
}

//============================================================================
void P2PEngine::onOncePer15Minutes( void )
{
    // if VxGetFastHostAnnounce() is set then will announce host every minute instead of every 15 minutes
    if( !VxGetFastHostAnnounce() )
    {
        m_PluginMgr.onThreadOncePer15Minutes();
    }  
}

//============================================================================
void P2PEngine::onOncePer30Minutes( void )
{
    int sktThreadCnt = VxSktBase::getRunningRxSktThreadCnt();
    int activeSktCnt = getPeerMgr().getActiveSktCnt();
    int toDeleteSktCnt = getPeerMgr().getToDeleteSktCnt();

    if( sktThreadCnt || activeSktCnt || toDeleteSktCnt )
    {
        LogMsg( LOG_VERBOSE, "Running Skt Rx Thread Count %d active %d to delete %d",
                sktThreadCnt, activeSktCnt, toDeleteSktCnt );
    }
}

//============================================================================
void P2PEngine::onOncePerHour( void )
{
// Attempting to renew port forward lease sometimes causes drop of all connections on hide.em
// If router does not honor infinate lease there does not seem to be a good solution
//#if defined(ENABLE_RENEW_PORT_FORWARD)
//    if( !m_PktAnn.requiresRelay() && !getNetStatusAccum().isLocalAndExternIpsTheSame() )
//    {
//        if( getEngineSettings().getUseUpnp() )
//        {
//            // even if you give upnp a lease of 0 (forever) some routers do not honor this
//            // periodically update upnp
//            m_NetServicesMgr.addNetActionToQueue( eNetActionRenewPortForward );
//        }
//    }
//#endif // !defined(ENABLE_RENEW_PORT_FORWARD)
}
