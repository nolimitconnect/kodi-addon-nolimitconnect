//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"
#include "P2PConnectList.h"

#include <GuiInterface/IToGui.h>

#include <BigListLib/BigListInfo.h>

#include <HostServerJoinMgr/HostServerJoinMgr.h>
#include <Membership/MemberConfirmMgr.h>
#include <Network/NetworkMgr.h>
#include <Plugins/PluginMgr.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <PktLib/PktsRelay.h>
#include <PktLib/PktsPing.h>

#include <NetLib/VxSktBase.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>

//============================================================================
void P2PEngine::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::replaceConnection: old skt num %d tmp %d new skt num %d tmp %d handle %d",
				poOldSkt->m_SktNumber,
				poOldSkt->isTempConnection(),
				poNewSkt->m_SktNumber,
				poNewSkt->isTempConnection(),
				poNewSkt->m_Socket );

	m_RcScan.replaceConnection( netIdent, poOldSkt, poNewSkt );
	m_PluginMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
//! socket about to close
void P2PEngine::onConnectionClosing( std::shared_ptr<VxSktBase>& sktBase )								
{
	// everything happens in onConnectionLost.. maybe this function should be removed
}

//============================================================================
//! socket became disconnected
void P2PEngine::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
    if( sktBase->isTempConnection() )
    {
        return; // ignore
    }

	bool possibleMemberConnection = getConnectIdListMgr().onConnectionLost( sktBase );

	if( possibleMemberConnection )
	{
		getHostJoinMgr().onConnectionLost( sktBase, sktBase->getSocketId(), sktBase->getPeerOnlineId() );
		getUserJoinMgr().onConnectionLost( sktBase, sktBase->getSocketId(), sktBase->getPeerOnlineId() );
	#if ENABLE_COMPONENT_NEARBY
		getNetworkMgr().getNearbyMgr().onConnectionLost( sktBase, sktBase->getSocketId(), sktBase->getPeerOnlineId() );
	#endif // ENABLE_COMPONENT_NEARBY
	}

    getConnectIdListMgr().onConnectionLost( sktBase );
	m_RcScan.onConnectionLost( sktBase );
	m_ConnectionList.onConnectionLost( sktBase );
    if( sktBase->getIsPeerPktAnnSet() )
    {
        getConnectionMgr().onSktDisconnected( sktBase );
    }

	m_PluginMgr.onConnectionLost( sktBase );
	GetMemberConfirmMgr().onConnectionLost( sktBase );
}

//============================================================================
void P2PEngine::onContactConnected( RcConnectInfo * poInfo, bool connectionListIsLocked, bool newContact )
{
	if( ( false == VxIsAppShuttingDown() ) 
		&& poInfo->getSkt()->isConnected() ) 
	{
		std::string strName = poInfo->m_BigListInfo->getOnlineName();
		if( !poInfo->getSkt()->isTempConnection() && shouldNotifyGui( poInfo->m_BigListInfo ) )
		{
			//LogMsg( LOG_INFO, "toGuiContactOnline id %s name %s\n", 
			//	poInfo->m_BigListInfo->getMyOnlineId().describeVxGUID().c_str(),
			//	strName.c_str() );
			IToGui::getIToGui().toGuiContactOnline( poInfo->m_BigListInfo->getVxNetIdent() );
		}
		//else
		//{
		//	LogMsg( LOG_INFO, "NO NOTIFY Gui of new contact id %s name %s\n", 
		//		poInfo->m_BigListInfo->getMyOnlineId().describeVxGUID().c_str(),
		//		strName.c_str() );
		//}


		m_PluginMgr.onContactWentOnline( (VxNetIdent*)poInfo->m_BigListInfo, poInfo->getSkt() );
		m_RcScan.onContactWentOnline( (VxNetIdent*)poInfo->m_BigListInfo, poInfo->getSkt() );
	}
}

//============================================================================
void P2PEngine::onContactDisconnected( RcConnectInfo * poInfo, bool connectionListLocked )
{
	if( !VxIsAppShuttingDown() )
	{
		getNetStatusAccum().onConnectionLost( poInfo->getSkt()->getSocketId() );

		LogModule( eLogConnect, LOG_VERBOSE, "onContactDisconnected %s telling plugin mgr", poInfo->m_BigListInfo->getOnlineName() );
		m_RcScan.onContactWentOffline( poInfo->m_BigListInfo->getVxNetIdent(), poInfo->getSkt() );

		m_PluginMgr.onContactWentOffline( poInfo->m_BigListInfo->getVxNetIdent(), poInfo->getSkt() );
	}
}

//============================================================================
void P2PEngine::sktMgrStatusCallback( std::string& sktAction, SOCKET sktHandle )
{
	if( !VxIsAppShuttingDown() )
	{
		LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::sktMgrStatusCallback param %s value %d", sktAction.c_str(), sktHandle );
	}

	if( sktAction == "ListenClose" )
	{
		getEngineParams().setLastListenSocket( 0 );
	}
	else if( sktAction == "ListenOpen" )
	{
		getEngineParams().setLastListenSocket( sktHandle );
	}
}
