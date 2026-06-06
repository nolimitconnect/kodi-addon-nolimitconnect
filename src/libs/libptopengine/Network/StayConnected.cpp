//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "StayConnected.h"
#include <Network/NetworkMgr.h>

#include <P2PEngine/P2PEngine.h>

#include <BigListLib/BigListMgr.h>
#include <BigListLib/BigListInfo.h>

#include <GuiInterface/IDefs.h>

#include <NetLib/VxSktConnect.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktCrypto.h>

#include <PktLib/PktsRelay.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>

#include <memory.h>

namespace
{
	const unsigned int	MIN_TIME_BETWEEN_CONNECT_ATTEMPTS_MS			= (10 * 60 * 1000); // 10 minutes in milliseconds
	const unsigned int	TIME_BETWEEN_ATTEMPTS_TO_STAY_CONNECTED_MS		= 2000; 
	//============================================================================
    void * StayConnectedThreadFunction( void * pvParam )
	{
		VxThread* poThread = (VxThread*)pvParam;
		poThread->setIsThreadRunning( true );
		StayConnected * poMgr = (StayConnected *)poThread->getThreadUserParam();

		poMgr->doStayConnectedThread();

		poThread->threadAboutToExit();
        return nullptr;
	}

} // namespace

//============================================================================
StayConnected::StayConnected( P2PEngine& engine )
: m_Engine( engine )
, m_BigListMgr( engine.getBigListMgr() )
, m_PktAnn( engine.getMyPktAnnounce() )
{
}

//============================================================================
StayConnected::~StayConnected()
{
	stayConnectedShutdown();
	m_StayConnectedThread.killThread();
}

//============================================================================
void StayConnected::stayConnectedStartup( void )
{
	m_StayConnectedThread.abortThreadRun( true );
	while( m_StayConnectedThread.isThreadRunning() )
	{
		LogMsg( LOG_INFO, "StayConnected::startup waiting for stay connected thread to die" );
		VxSleep( 200 );
	}

	m_StayConnectedThread.startThread( (VX_THREAD_FUNCTION_T)StayConnectedThreadFunction, this, "StayConnectedThread" ); 
}

//============================================================================
void StayConnected::stayConnectedShutdown( void )
{
	m_StayConnectedThread.abortThreadRun( true );
}

//============================================================================
void StayConnected::doStayConnectedThread( void )
{
	int iConnectToIdx							= 0;
	VxMutex * poListMutex						= &m_BigListMgr.m_FriendListMutex;
	std::vector< BigListInfo * >& friendList	= m_BigListMgr.m_FriendList;
	int iSize;
	BigListInfo * poInfo;

	std::shared_ptr<VxSktBase> sktBase( nullptr );
	while( ( false == m_StayConnectedThread.isAborted() )
			&& ( false == VxIsAppShuttingDown() ) )
	{
		VxSleep( TIME_BETWEEN_ATTEMPTS_TO_STAY_CONNECTED_MS );
		if( !m_Engine.getNetStatusAccum().isNetworkOnline() )
		{
			// don't ping friends until we are fully online and correct connect info in announcement
			continue;
		}

		if( 0 != friendList.size() )
		{
			//LogMsg( LOG_VERBOSE, "doStayConnected attempt lock" );
			poListMutex->lock();
			//LogMsg( LOG_VERBOSE, "doStayConnected attempt lock success" );
			iSize = (int)friendList.size();
			if( iSize )
			{
				iConnectToIdx++;
				if( iConnectToIdx >= iSize )
				{
					iConnectToIdx = 0;
				}

				poInfo = friendList[iConnectToIdx];
				if( !poInfo->requiresRelay() && 
					( poInfo->isFriend() || poInfo->isAdministrator() ) &&
					!m_Engine.getConnectIdListMgr().isUserOnline( poInfo->getMyOnlineId() ) )
				{
					if( MIN_TIME_BETWEEN_CONNECT_ATTEMPTS_MS < ( GetGmtTimeMs() - poInfo->getTimeLastConnectAttemptMs() ) )
					{
						bool isNewConnection = false;
						if( m_Engine.connectToContact( poInfo->getConnectInfo(), sktBase, isNewConnection, eConnectReasonStayConnected ) )
						{
							if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_DEBUG, "%s connected to user %s at %s", __func__,
									   poInfo->getOnlineName(), poInfo->getOnlineIpAddress().toString().c_str() );
							poInfo->contactWasAttempted( true );
						}
						else
						{
							if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_DEBUG, "%s failed connected to user %s at %s", __func__,
									   poInfo->getOnlineName(), poInfo->getOnlineIpAddress().toString().c_str() );
							poInfo->contactWasAttempted( false );
						}
					}
				}
			}

			poListMutex->unlock();
		}

		if( m_StayConnectedThread.isAborted() )
		{
			return;
		}
	}
}
