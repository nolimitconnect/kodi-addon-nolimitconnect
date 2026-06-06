//============================================================================
// Copyright (C) 2013 Brett R. Jones 
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

#include <Network/NetworkMgr.h>
#include <BigListLib/BigListInfo.h>

#include <CoreLib/VxDebug.h>

#include <memory.h>

//============================================================================
//! called after m_PktAnn has changed
void P2PEngine::doPktAnnHasChanged( bool connectionListIsLocked )
{
	//LogMsg( LOG_ERROR, "P2PEngine::doPktAnnHasChanged start\n" );
    setPktAnnLastModTime( GetTimeStampMs() );
	m_ConnectionList.setIsRelayRequired( m_PktAnn.requiresRelay() );
	// announce to all our new announce
	if( false == isNetworkOnline() )
	{
		// don't announce to users until online
		return;
	}

	PktAnnounce oPktAnn;
	BigListInfo * poBigInfo;

	if( false == connectionListIsLocked )
	{
        //LogMsg( LOG_ERROR, "P2PEngine::doPktAnnHasChanged m_ConnectListMutex attempt lock" );
		m_ConnectionList.connectListLock();
        //LogMsg( LOG_ERROR, "P2PEngine::doPktAnnHasChanged m_ConnectListMutex lock success" );
	}

	// make copy of pkt announce 
	lockAnnouncePktAccess();
	memcpy( &oPktAnn, &m_PktAnn, sizeof(PktAnnounce) );
	unlockAnnouncePktAccess();
	oPktAnn.setIsPktAnnReplyRequested( false );

	std::map<VxGUID, RcConnectInfo *, cmp_vxguid>::iterator iter;
	for( iter = m_ConnectionList.m_ConnectList.begin(); iter != m_ConnectionList.m_ConnectList.end(); ++iter )
	{
		RcConnectInfo * poConnectInfo = iter->second;
		if( poConnectInfo )
		{
			poBigInfo = poConnectInfo->m_BigListInfo;

			// set permissions
			oPktAnn.setMyFriendshipToHim(poBigInfo->getMyFriendshipToHim());
			oPktAnn.setHisFriendshipToMe(poBigInfo->getHisFriendshipToMe());
			txSystemPkt( poBigInfo->getMyOnlineId(), poConnectInfo->m_SktBase, &oPktAnn );
		}
	}

	if( false == connectionListIsLocked )
	{
        //LogMsg( LOG_INFO, "P2PEngine::doPktAnnHasChanged: m_ConnectListMutex.unlock()" );
		m_ConnectionList.connectListUnlock();
	}

	//LogMsg( LOG_INFO, "P2PEngine::doPktAnnHasChanged done\n" );
}

//============================================================================
//! pkt ann has changed and needs to be re announced
void P2PEngine::doPktAnnConnectionInfoChanged( bool connectionListIsLocked )
{
	doPktAnnHasChanged( connectionListIsLocked );
	IToGui::getIToGui().toGuiUpdateMyIdent( &m_PktAnn );
}
