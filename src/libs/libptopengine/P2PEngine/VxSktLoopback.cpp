//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktLoopback.h"
#include "P2PEngine.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxTime.h>

namespace
{
    //============================================================================
    void* VxSktLoopbackThreadFunc( void* pvContext )
    {
        VxThread* poThread = ( VxThread* )pvContext;
        poThread->setIsThreadRunning( true );
        VxSktLoopback* sktLoopback = ( VxSktLoopback* )poThread->getThreadUserParam();
        if( sktLoopback && false == poThread->isAborted() )
        {
            sktLoopback->executeSktLoopbackRxThreadFunc();
        }

        poThread->threadAboutToExit();
        sktLoopback->checkForMorePacketsAfterThreadExit();
        return nullptr;
    }
} // namespace

//============================================================================
VxSktLoopback::VxSktLoopback( P2PEngine& engine )
    : VxSktBase()
    , m_Engine( engine )
{
    // LogMsg( LOG_VERBOSE, "VxSktLoopback::VxSktLoopback" );
	m_eSktType = eSktTypeLoopback;
    m_ConnectionId = m_LoopbackSocketId;
    const char* loopBackIp = "127.0.0.1";
    m_LclIp.setIp( loopBackIp );
    m_strLclIp = loopBackIp;
    m_RmtIp.setIp( loopBackIp );
    m_strRmtIp = loopBackIp;
}

//============================================================================
int32_t VxSktLoopback::txPacketWithDestId( VxPktHdr* pktHdrIn, bool sktMgrLocked )
{
    if( !VxIsAppShuttingDown() && pktHdrIn )
    {
        vx_assert( pktHdrIn->getDestOnlineId() == m_Engine.getMyOnlineId() );

        VxPktHdr* pktHdr = pktHdrIn->makeCopy();

        lockPktList();
        m_PktList.emplace_back( pktHdr );
        unlockPktList();

        enableSktLoopbackThread( true );
    }

    return 0;
}

//============================================================================
void VxSktLoopback::enableSktLoopbackThread( bool enable )
{
    if( enable && !VxIsAppShuttingDown() && m_PktList.size() <= 1 && !m_SktLoopbackThread.isThreadRunning() )
    {
        // if the multiple threads run for a bit it should not matter
        std::string threadName{ "SktLoopbackThread" };
        m_SktLoopbackThread.startThread( ( VX_THREAD_FUNCTION_T )VxSktLoopbackThreadFunc, this, threadName.c_str() );
    }
    else if( !enable )
    {
        m_SktLoopbackThread.killThread();
    }
}

//============================================================================
void VxSktLoopback::executeSktLoopbackRxThreadFunc( void )
{
    int64_t lastTimePktProcessed = GetHighResolutionTimeMs();
    while( !m_SktLoopbackThread.isAborted() && !VxIsAppShuttingDown() && (!m_PktList.empty() || GetHighResolutionTimeMs() - lastTimePktProcessed  < 10000))
    {
        if( !m_PktList.empty() )
        {
            VxPktHdr* pktHdr{ nullptr };
            lockPktList();
            auto iter = m_PktList.begin();
            if( iter != m_PktList.end() )
            {
                pktHdr = *iter;
                m_PktList.erase( iter );
            }

            unlockPktList();
            if( pktHdr )
            {
                lastTimePktProcessed = GetHighResolutionTimeMs();
                m_Engine.handlePkt( getThisSkt(), pktHdr);
                delete pktHdr;
            }
        }
        else
        {
            VxSleep( 400 );
        }
    }
}

//============================================================================
void VxSktLoopback::checkForMorePacketsAfterThreadExit( void )
{
    if( !m_PktList.empty() )
    {
        enableSktLoopbackThread( true );
    }
}