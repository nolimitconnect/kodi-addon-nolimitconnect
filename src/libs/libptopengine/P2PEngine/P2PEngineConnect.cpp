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

#include <GuiInterface/IToGui.h>

#include <BigListLib/BigListInfo.h>
#include <ConnectIdListMgr/ConnectIdListMgr.h>

#include <Network/ConnectRequest.h> 
#include <Network/StayConnected.h>
#include <Network/NetworkMgr.h>
#include <NetworkMonitor/NetworkMonitor.h>
#include <NetServices/NetServicesMgr.h>

#include "P2PConnectList.h"
#include <Plugins/PluginMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsRelay.h>

#include <string.h>

//============================================================================
bool P2PEngine::connectToContact(	VxConnectInfo&		connectInfo, 
									std::shared_ptr<VxSktBase>&	ppoRetSkt,
									bool&				retIsNewConnection,
									EConnectReason		connectReason )
{
	bool result = m_ConnectionMgr.connectToContact( connectInfo, ppoRetSkt, getMyOnlineId(), retIsNewConnection );
	if(  true == result )
	{
		if( retIsNewConnection )
		{
			ppoRetSkt->setIsTempConnection( IsConnectReasonTemporary( connectReason ) );
			// handle success connect
			BigListInfo * bigListInfo = 0;
			int retryCnt = 0;
			while( ( nullptr == bigListInfo )
					&& ( retryCnt < 3 ) )
			{
				// wait for announce packet that was sent when connected to be rxed so we get big list info
				retryCnt++;
				bigListInfo = getBigListMgr().findBigListInfo( connectInfo.getMyOnlineId() );
				if( 0 == bigListInfo )
				{
					VxSleep( 1000 );
				}
			}

			if( bigListInfo )
			{
                LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::connectToContact: success %s", bigListInfo->getOnlineName() );
				m_ConnectionMgr.handleConnectSuccess( bigListInfo, ppoRetSkt, retIsNewConnection, connectReason );

                // nearby elevate to guest permission
                updateOnFirstConnect( ppoRetSkt, bigListInfo, eConnectReasonNearbyLan == connectReason
                                    || eConnectReasonSameExternalIp == connectReason );
			}
			else
			{
                LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::connectToContact: No BigList for connected" );
			}
		}
	}

	return result;
}

//============================================================================
bool P2PEngine::txSystemPkt(	const VxGUID&		destOnlineId,
								std::shared_ptr<VxSktBase>&			sktBase, 
								VxPktHdr*			poPkt )
{
	bool bSendSuccess = false;
	poPkt->setSrcOnlineId( m_PktAnn.getMyOnlineId() );

	if( 0 == (poPkt->getPktLength() & 0xf ) )
	{
		if( sktBase->isConnected() && sktBase->isTxEncryptionKeySet() )
		{
			sktBase->m_u8TxSeqNum++;
			poPkt->setPktSeqNum( sktBase->m_u8TxSeqNum );
			int32_t rc = sktBase->txPacket( destOnlineId, poPkt );
			if( 0 == rc )
			{
				bSendSuccess = true;
			}
#ifdef DEBUG_PKTS
			else
			{
				LogMsg( LOG_ERROR, "P2PEngine::txSystemPkt: skt %d error %d", sktBase->m_SktNumber, sktBase->m_rcLastError );
			}
#endif // DEBUG_PKTS
		}
#ifdef DEBUG_PKTS
		else
		{
			if( false == sktBase->isConnected() )
			{
				LogMsg( LOG_ERROR, "P2PEngine::txSystemPkt: error skt %d not connected", sktBase->m_SktNumber );
			}
			else
			{
				LogMsg( LOG_ERROR, "P2PEngine::txSystemPkt: error skt %d has no encryption key", sktBase->m_SktNumber );
			}
		}
#endif // DEBUG_PKTS
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::txSystemPkt: Invalid system Packet length %d type %d", poPkt->getPktLength(), poPkt->getPktType() );
		vx_assert( false );
	}

	return bSendSuccess;
}

//============================================================================
void P2PEngine::broadcastSystemPkt( VxPktHdr* pkt, bool onlyIncludeMyContacts )
{
	m_ConnectionList.broadcastSystemPkt( pkt, onlyIncludeMyContacts );
}

//============================================================================
void P2PEngine::broadcastSystemPkt( VxPktHdr* pkt, VxGUIDList& retIdsSentPktTo )
{
	m_ConnectionList.broadcastSystemPkt( pkt, retIdsSentPktTo );
}

//============================================================================
bool P2PEngine::txPluginPkt( enum EPluginType			pluginType,
								VxNetIdent*				netIdent, 
								std::shared_ptr<VxSktBase>&	sktBase, 
								VxPktHdr*				poPkt )
{
	bool bSendSuccess = false;
	if( 0 == (poPkt->getPktLength() & 0xf ) )
	{
        if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "skt %d txPluginPkt %d", sktBase->m_SktNumber, poPkt->getPktType() );
		if( sktBase->isConnected() && sktBase->isTxEncryptionKeySet() )
		{
			poPkt->setSrcOnlineId( m_PktAnn.getMyOnlineId() );
			poPkt->setPluginNum( (uint8_t)pluginType );
			sktBase->m_u8TxSeqNum++;
			poPkt->setPktSeqNum( sktBase->m_u8TxSeqNum );
			int32_t rc = sktBase->txPacket( netIdent->getMyOnlineId(), poPkt );
			if( 0 == rc )
			{
				bSendSuccess = true;
			}
			else
			{
                LogMsg( LOG_ERROR, "P2PEngine::txPluginPkt: error %d", rc );
			}
		}
		else
		{
            LogMsg( LOG_ERROR, "skt %d P2PEngine::txPluginPkt: ERROR disconnected or no TxEncryption key", sktBase->m_SktNumber );
		}
	}
	else
	{
        LogMsg( LOG_ERROR, "P2PEngine::txPluginPkt: Invalid Packet length %d type %d from plugin %s",
			poPkt->getPktLength(),
			poPkt->getPktType(),
			DescribePluginType( pluginType ));
	}
	return bSendSuccess;
}

//============================================================================
void P2PEngine::attemptConnectionToRelayService( BigListInfo * poInfo )
{
	std::shared_ptr<VxSktBase> sktBase( nullptr );
    bool newConnection{false};
    connectToContact( poInfo->getConnectInfo(), sktBase, newConnection, eConnectReasonRelayService );
}

//============================================================================
bool P2PEngine::isContactConnected( VxGUID& oOnlineId )
{
	return (nullptr != m_ConnectionList.findConnection( oOnlineId, false )) ? true : false;
}

//============================================================================
std::string P2PEngine::describeContact( BigListInfo * bigListInfo )
{
	VxConnectInfo& connectInfo = bigListInfo->getConnectInfo();

	std::string hexId;
	connectInfo.getMyOnlineId().toOnlineIdString( hexId );

	std::string strDesc;
	if( connectInfo.requiresRelay() )
	{
		StdStringFormat( strDesc, " %s Connected ? %d Online ID %s requires relay ", 
			bigListInfo->getOnlineName(),
			getConnectIdListMgr().isUserOnline( bigListInfo->getMyOnlineId() ),
			hexId.c_str() );
	}
	else
	{
		StdStringFormat( strDesc, " %s Connected ? %d Online ID %s at ip %s ", 
			bigListInfo->getOnlineName(),
			getConnectIdListMgr().isUserOnline( bigListInfo->getMyOnlineId() ),
			hexId.c_str(), 
			connectInfo.getOnlineIpAddress().toString().c_str() );
	}

	return strDesc;
}

//============================================================================
std::string P2PEngine::describeContact( VxConnectInfo& connectInfo )
{
	BigListInfo * poBigListInfo = getBigListMgr().findBigListInfo( connectInfo.getMyOnlineId() );
	if( poBigListInfo )
	{
		return describeContact( poBigListInfo );
	}

	std::string hexId;
	connectInfo.getMyOnlineId().toOnlineIdString( hexId );

	std::string strDesc;
	if( connectInfo.requiresRelay() )
	{
		StdStringFormat( strDesc, " name %s ID %s requires relay ", 
			(0 == strlen( connectInfo.getOnlineName() )) ? "UNKNOWN" : connectInfo.getOnlineName(),
			hexId.c_str() );
	}
	else
	{
		StdStringFormat( strDesc, " name %s ID %s at ip %s ", 
			(0 == strlen( connectInfo.getOnlineName() )) ? "UNKNOWN" : connectInfo.getOnlineName(),
			hexId.c_str(), 
			connectInfo.getOnlineIpAddress().toString().c_str() );
	}
	
	return strDesc;
}

//============================================================================
std::string P2PEngine::describeContact( ConnectRequest& connectRequest )
{
	return describeContact( connectRequest.getConnectInfo() );
}

//============================================================================
bool P2PEngine::updateOnFirstConnect( std::shared_ptr<VxSktBase>& sktBase, BigListInfo* poInfo, bool nearbyLanConnected )
{
	if( !sktBase || !poInfo )
	{
		LogMsg( LOG_ERROR, "P2PEngine::updateOnFirstConnect: Invalid Param" );
		return false;
	}

	int64_t timestamp = sktBase->getLastActiveTimeMs();
	if( poInfo->isIgnored() )
	{
		getIgnoreListMgr().updateIdent( poInfo->getMyOnlineId(), timestamp );
		return false;
	}

	poInfo->setLastSessionTimeMs( timestamp );
	poInfo->setConnectSuccessCnt( poInfo->getConnectSuccessCnt() + 1 );
	poInfo->setConnectErrCnt( 0 );

    EFriendState eMyFriendshipToHim = poInfo->getMyFriendshipToHim();
    EFriendState eHisFriendshipToMe = poInfo->getHisFriendshipToMe();

    if( eMyFriendshipToHim != eFriendStateAnonymous || eHisFriendshipToMe != eFriendStateAnonymous )
    {
        LogModule( eLogConnect, LOG_DEBUG, "P2PEngine::updateOnFirstConnect myFriendship %s hisFriendship %s name %s id %s",
				DescribeFriendState( eMyFriendshipToHim ), DescribeFriendState( eHisFriendshipToMe ),
				poInfo->getOnlineName(), poInfo->getMyOnlineId().toOnlineIdString().c_str()  );
    }

	if( poInfo->isFriend() || poInfo->isAdministrator() )
	{
		getFriendListMgr().updateIdent( poInfo->getMyOnlineId(), timestamp );
	}

	if( !sktBase->isTempConnection() )
	{
        GroupieId groupieId( poInfo->getMyOnlineId(), poInfo->getMyOnlineId(), ConnectReasonToHostType( sktBase->getConnectReason() ) );
		// make sure user identity is updated first before updating connection info
        //if( getConnectIdListMgr().onUserOnline( groupieId, sktBase, poInfo->getVxNetIdent() ) )
		//{
            // must use client instead of host
			getThumbMgr().queryThumbIfNeeded( sktBase, poInfo->getVxNetIdent(), eHostTypePeerUser );
		//}

		getConnectIdListMgr().addConnection( sktBase, groupieId, false );
	}

    return true;
}

//============================================================================
bool P2PEngine::isMyAccessAllowedFromHim( VxGUID& onlineId, EPluginType pluginType )
{
	bool isAllowed{ false };
    bool isOnline = getConnectIdListMgr().isUserOnline( onlineId );
	if( isOnline )
	{
		VxNetIdent* netIdent = getBigListMgr().findNetIdent( onlineId );
		if( netIdent )
		{
			isAllowed = netIdent->isMyAccessAllowedFromHim( pluginType );
		}
	}

	if( !isAllowed )
	{
		LogMsg( LOG_WARN, "P2PEngine::%s not allowed %s by him", __func__, DescribePluginType( pluginType ) );
	}

	return isAllowed;
}

//============================================================================
bool P2PEngine::isHisAccessAllowedFromMe( VxGUID& onlineId, EPluginType pluginType )
{
	bool isAllowed{ false };
    bool isOnline = getConnectIdListMgr().isUserOnline( onlineId );
	if( isOnline )
	{
		VxNetIdent* netIdent = getBigListMgr().findNetIdent( onlineId );
		if( netIdent )
		{
			isAllowed = netIdent->isHisAccessAllowedFromMe( pluginType );
		}
	}

	if( !isAllowed )
	{
		LogMsg( LOG_WARN, "P2PEngine::%s not allowed %s from me", __func__, DescribePluginType( pluginType ) );
	}

	return isAllowed;
}

//============================================================================
void P2PEngine::disconnectFromHostIfNotNeeded( HostedId& adminId )
{
	if( adminId.getHostOnlineId() == getMyOnlineId() )
	{
		LogMsg( LOG_ERROR, "HostBaseMgr::%s host is myself", __func__ );
		return;
	}

	VxNetIdent* hostIdent = getBigListMgr().findNetIdent( adminId.getHostOnlineId() );
	if( !hostIdent )
	{
		LogMsg( LOG_ERROR, "HostBaseMgr::%s host ident not foune", __func__ );
		return;
	}

	hostIdent->setIsJoined( adminId.getHostType(), false );
	if( hostIdent->isJoinedAny() )
	{
		LogMsg( LOG_ERROR, "HostBaseMgr::%s still joined to other hosts", __func__ );
		return;
	}

	if( hostIdent->getHisFriendshipToMe() > eFriendStateGuest )
	{
		LogMsg( LOG_ERROR, "HostBaseMgr::%s disconnect ignored because his friendship", __func__ );
		return;
	}

	if( hostIdent->getMyFriendshipToHim() > eFriendStateGuest )
	{
		LogMsg( LOG_ERROR, "HostBaseMgr::%s disconnect ignored because my friendship to him", __func__ );
		return;
	}

	getConnectIdListMgr().disconnectFromHost( adminId );
}
