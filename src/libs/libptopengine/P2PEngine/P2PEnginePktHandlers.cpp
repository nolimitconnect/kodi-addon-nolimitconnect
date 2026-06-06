//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include <BigListLib/BigListInfo.h>

#include <Plugins/PluginMgr.h>
#include <Plugins/PluginFileShareServer.h>
#include <Plugins/PluginFileShareClient.h>
#include <Plugins/PluginFriendRequest.h>
#include <Plugins/PluginRandomConnectHost.h>
#include <Plugins/PluginRandomConnectClient.h>

#include <Network/StayConnected.h>

#include <CoreLib/VxDebug.h>

#include <NetLib/VxSktCrypto.h>
#include <NetLib/VxSktBase.h>

#include <PktLib/PktAdminAvail.h>
#include <PktLib/PktTcpPunch.h>
#include <PktLib/PktsPing.h>
#include <PktLib/PktsMembership.h>

#include <memory.h>

//============================================================================
void P2PEngine::handlePkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	// relay packets will have a destination id other than ourself
	
	if( pktHdr->getDestOnlineId() != getMyOnlineId() )
	{
		getRelayMgr().handleRelayPkt( sktBase, pktHdr );
	}
	else
	{
		PktHandlerBase::handlePkt( sktBase, pktHdr );
	}
}

//============================================================================
void P2PEngine::onPktUnhandled( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	LogMsg( LOG_ERROR, "onPktUnhandled pkt type %d", pktHdr->getPktType() );
}

//============================================================================
void P2PEngine::onPktInvalid( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	LogMsg( LOG_ERROR, "onPktInvalid pkt type %d", pktHdr->getPktType() );
}

//============================================================================
void P2PEngine::onPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if( !sktBase || !sktBase->isConnected() )
	{
		LogMsg( LOG_ERROR, "%s null or invalid param  ", __func__ );
		return;
	}

	PktAnnounce* pktAnn = (PktAnnounce *)pktHdr;
	VxGUID contactOnlineId = pktAnn->getMyOnlineId();
    // VxGUID winDevOnlineId("!62F5902FCDE74AAF4928FCE61975ECE4!");
    // if( contactOnlineId == winDevOnlineId )
    // {
    //     LogMsg( LOG_VERBOSE, "GuiUserMgr::%s %s his friendship to me %s", __func__,
    //            pktAnn->getOnlineName(), DescribeFriendState( pktAnn->getHisFriendshipToMe() ) );
    // }

	if( contactOnlineId == getMyOnlineId() )
	{
		// it is ourself
        LogMsg( LOG_ERROR, "onPktAnnounce Cannot send a packet to ourself  " );

        std::string rmAddr = sktBase->getRemoteIpAddress() ? sktBase->getRemoteIpAddress() : "";
        std::string ourAddr = getNetStatusAccum().getExternalIpAddress();
        if( rmAddr.empty() || sktBase->getRemoteIpAddress() != ourAddr )
        {
            // remote attack.. serious
            hackerOffense( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, pktAnn, sktBase, "rxed same as our online id from another " );
            sktBase->closeSkt( eSktClosePktOnlineIdMeFromAnotherIp );
        }
        else
        {
            hackerOffense( eHackerLevelSuspicious, eHackerReasonPktOnlineIdMeFromMyIp, pktAnn, sktBase, "rxed same as our online from our address " );
            sktBase->closeSkt( eSktClosePktOnlineIdMeFromMyIp );
        }

		return;
	}

	pktAnn->clearIsJoined();
	bool isFirstAnnounce = false;
	if( false == sktBase->m_TxCrypto.isKeyValid() )
	{
		// setup tx crypto
		isFirstAnnounce = true;
		std::string networkName;
		m_EngineSettings.getNetworkKey( networkName );
		GenerateTxConnectionKey( sktBase, &pktAnn->m_DirectConnectId, networkName.c_str() );
	}
	else if( !sktBase->getIsPeerPktAnnSet() )
	{
		isFirstAnnounce = true;
	}

	pktAnn->reversePermissions();
	pktAnn->setTimeLastTcpContactMs( GetGmtTimeMs() );

	// if we connected out with a reason then that is the most trusted
	EHostType hostType = ConnectReasonToJoinHostType( sktBase->getConnectReason() );
	bool isJoinConnection = hostType != eHostTypeUnknown;
	if( !isJoinConnection )
	{
		// see if some other connect reason
		hostType = ConnectReasonToHostType( sktBase->getConnectReason() );
		if( hostType == eHostTypePeerUser ) // defaulted to peer because reason is unknown (might be other plugin reasons)
		{
			EHostType announcedHostType = pktAnn->getHostType();
			if( announcedHostType != eHostTypeUnknown && announcedHostType != hostType )
			{
				// it is a bit of a security risk to rely on anything told to us by others but here we are
				LogMsg( LOG_WARN, "P2PEngine::%s pktAnn hostType %s", __func__, DescribeHostType( announcedHostType ) );
				hostType = announcedHostType;
			}
		}
	}

	GroupieId groupieId( pktAnn->getMyOnlineId(), isFirstAnnounce ? pktAnn->getMyOnlineId() : sktBase->getPeerOnlineId(), hostType );
	ConnectId connectId( sktBase->getSocketId(), groupieId );
	connectId.setIsRelayed( groupieId.getHostOnlineId() != groupieId.getUserOnlineId() );

	if( isFirstAnnounce )
	{
		if( pktAnn->getIsPktAnnTempConnection() )
		{
			sktBase->setIsTempConnection( true );
			pktAnn->setIsPktAnnTempConnection( false );
			if( LogEnabled( eLogConnect ) ) LogModule( eLogConnect, LOG_WARN, "P2PEngine::%s temp connection %s connect reason %s", __func__, 
				pktAnn->getOnlineName(), DescribeConnectReason( sktBase->getConnectReason() ) );
		}
		else
		{
			if( LogEnabled( eLogConnect ) ) LogModule( eLogConnect, LOG_WARN, "P2PEngine::%s Not temp %s connect reason %s", __func__, 
				pktAnn->getOnlineName(), DescribeConnectReason( sktBase->getConnectReason() ) );
		}
	}
	else
	{
		if( LogEnabled( eLogUsers ) )LogModule( eLogUsers, LOG_VERBOSE, "P2PEngine::%s %s %s", __func__, pktAnn->describeUser().c_str(), describeGroupieId( groupieId ).c_str() );
	}

	if( IsHostARelayForUsers( hostType ) && connectId.isRelayed() )
	{
		pktAnn->setIsJoined( hostType, true );
	}

	BigListInfo * bigListInfo = 0;
	EPktAnnUpdateType pktAnnUpdateType = m_BigListMgr.updatePktAnn( pktAnn, &bigListInfo, hostType, false, !isJoinConnection );
	if( !bigListInfo->isValidNetIdent() )
	{
		LogMsg( LOG_ERROR, "PktAnnounce updatePktAnn INVALID" );
		return;
	}

	if( ePktAnnUpdateTypeIgnored == pktAnnUpdateType )
	{
		LogModule( eLogConnect, LOG_VERBOSE, "Ignoring %s ip %s id %s",
			pktAnn->getOnlineName(),
            sktBase->getRemoteIp().c_str(),
			contactOnlineId.toOnlineIdString().c_str() );
        // if is the first announce and ignored we can close the connection
        // if not first announce then was relayed through host and do not close the connection
        if( isFirstAnnounce )
		{
            m_ConnectionMgr.closeConnection( eSktCloseUserIgnored, contactOnlineId, sktBase );
            getConnectList().onConnectionLost( sktBase );
		}

		return;
	}

	bool pktAnnReplyRequested = pktAnn->getIsPktAnnReplyRequested();
	bool reverseConnectionRequested = pktAnn->getIsPktAnnRevConnectRequested();
	if( pktAnn->getTTL() > 0 )
	{
		pktAnn->setTTL( pktAnn->getTTL() - 1 );
		pktAnn->setIsPktAnnReplyRequested( false );
		pktAnn->setIsPktAnnStunRequested( false );
	}

	if( pktAnnReplyRequested )
	{
		if(LogEnabled(eLogConnect))LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::onPktAnnounce %s from %s %s at %s pktAnn reply requested",
				   sktBase->describeSktType().c_str(), pktAnn->getOnlineName(), pktAnn->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
        VxGUID srcOnlineId = pktAnn->getSrcOnlineId();
        if( !m_ConnectionMgr.sendMyPktAnnounce( srcOnlineId,
				sktBase,
				false,
				false,
                false,
                false ) )
		{
			LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::onPktAnnounce %s from %s at %s send pktAnn reply failed",
					   sktBase->describeSktType().c_str(), pktAnn->getOnlineName(), pktAnn->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
			sktBase->closeSkt( eSktClosePktAnnSendFail );
            getConnectList().onConnectionLost( sktBase );
			return;
		}
	}

	bool updateOk{ false };

	if( isFirstAnnounce )
	{
		int64_t timeNow( GetGmtTimeMs() );
		pktAnn->setLastSessionTimeMs( timeNow );
		bigListInfo->setLastSessionTimeMs( timeNow );
		getBigListMgr().dbUpdateSessionTime( pktAnn->getMyOnlineId(), timeNow );
		updateOk = onFirstPktAnnounce( sktBase, pktAnn, pktAnnUpdateType, bigListInfo, connectId );
	}
	else
	{
		if( sktBase->getIsPeerPktAnnSet() )
		{
			if( !connectId.isRelayed() && connectId.getUserOnlineId() == sktBase->getPeerOnlineId() )
			{
				// user may have changed permissions or something
				updateOk = onConnectionPktAnnounceUpdated( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
			}

			// detect if was announced by host instead of user
			if( IsHostARelayForUsers( hostType ) )
			{
				updateOk = onHostedUserPktAnnounce( sktBase, pktAnn, pktAnnUpdateType, bigListInfo, connectId );
			}
			else if( connectId.isRelayed() )
			{
				LogMsg( LOG_WARN, "P2PEngine::%s unexpected relay %s %s through relay %s %s ip %s", __func__,
				pktAnn->getOnlineName(),
                    pktAnn->getMyOnlineId().toOnlineIdString().c_str(),
                    sktBase->getPeerOnlineName().c_str(),
		 		    sktBase->getPeerOnlineId().toOnlineIdString().c_str(),
		 		    sktBase->getRemoteIp().c_str() );
				updateOk = onRelayedUserPktAnnounce( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
			}
	

			//LogModule( eLogOnline, LOG_VERBOSE, "P2PEngine::onPktAnnounce %s %s through relay %s %s ip %s",
			//		   pktAnn->getOnlineName(),
   //                    pktAnn->getMyOnlineId().toOnlineIdString().c_str(),
   //                    sktBase->getPeerOnlineName().c_str(),
			//		   sktBase->getPeerOnlineId().toOnlineIdString().c_str(),
			//		   sktBase->getRemoteIp().c_str() );
		}
		else
		{
			updateOk = onUnexpectedPktAnnounce( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
			//LogMsg( LOG_ERROR, "P2PEngine::onPktAnnounce not first PktAnd and peer PktAnn not set ip %s",
			//		sktBase->getRemoteIp().c_str() );
			// what should we do here. Hacker attempt or bad programming?
		}
	}

	if( !updateOk )
	{
		 LogMsg( LOG_ERROR, "P2PEngine::onPktAnnounce %s from %s %s at %s failed to update", 
					sktBase->describeSktType().c_str(), pktAnn->getOnlineName(), pktAnn->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
		 sktBase->closeSkt( eSktClosePktAnnUpdateFailed ); // should we close? TODO investigate failed PktAnn update failed
		 return; 
	}

	if( !sktBase->isTempConnection() )
	{
        if(LogEnabled(eLogConnect))LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::%s %s of %s %s by %s %s at %s skt id %s", __func__,
                sktBase->describeSktType().c_str(), pktAnn->getOnlineName(), pktAnn->getMyOnlineId().toOnlineIdString().c_str(),
				sktBase->getPeerOnlineName().c_str(), sktBase->getPeerOnlineId().toOnlineIdString().c_str(),
				sktBase->getRemoteIp().c_str(), sktBase->getSocketIdText().c_str() );
	}
    else
    {
		if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::%s ignoring temp connection %s to %s", __func__,
               sktBase->getSocketIdText().c_str(), sktBase->getRemoteIp().c_str() );
    }
}

//============================================================================
void P2PEngine::onPktAnnList( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAnnList" );
}
	
//============================================================================
void P2PEngine::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPluginOfferReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPluginOfferReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktChatReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktChatReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktChatReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVoiceReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVoiceReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVideoFeedReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVideoFeedStatus" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVideoFeedPic" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVideoFeedPicChunk" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktVideoFeedPicAck" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileGetReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileGetReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileSendReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileSendReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileListReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileListReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileInfoReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileChunkReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileChunkReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileSendCompleteReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileSendCompleteReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileGetCompleteReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileGetCompleteReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if( LogEnabled( eLogPkt ) ) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileXferCancel" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileShareErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFileShareErr" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetGetReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetGetReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetSendReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetSendReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetChunkReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetChunkReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetGetCompleteReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetGetCompleteReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetSendCompleteReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetSendCompleteReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktAssetXferErr" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktMultiSessionReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMultiSessionReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktMultiSessionReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMultiSessionReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktSessionStartReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktSessionStartReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktSessionStartReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktSessionStartReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktSessionStopReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktSessionStopReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktSessionStopReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktMyPicSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMyPicSendReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktMyPicSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMyPicSendReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerPicChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerPicChunkTx" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerPicChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerPicChunkAck" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerGetChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerGetChunkTx" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerGetChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerGetChunkAck" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerPutChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerPutChunkTx" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktWebServerPutChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktWebServerPutChunkAck" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktTodGameStats" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktTodGameAction" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktTodGameValue" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTcpPunch( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktTcpPunch" );

	PktTcpPunch * pktPunch = ( PktTcpPunch * )pktHdr;
	std::shared_ptr<VxSktBase> poNewSkt;
    bool newConnection{false};
    if( 0 == connectToContact( pktPunch->m_ConnectInfo, poNewSkt, newConnection, eConnectReasonPktTcpPunch ) )
	{
		LogMsg( LOG_INFO, "P2PEngine:: TcpPunch SUCCESS" );
		if( nullptr != poNewSkt )
		{
			LogMsg( LOG_INFO, "sendMyPktAnnounce 7" ); 
            m_ConnectionMgr.sendMyPktAnnounce(	pktPunch->m_ConnectInfo.getMyOnlineId(),
												poNewSkt,
												true,
												false,
                                                false,
                                                false );
		}
	}
	else
	{
		LogMsg( LOG_INFO, "P2PEngine:: TcpPunch FAIL" );
	}
}

//============================================================================
void P2PEngine::onPktPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPingReq" );

	PktPingReq * pktPingReq = (PktPingReq *)pktHdr;
	PktPingReply pktPingReply;
	pktPingReply.setSrcOnlineId( m_PktAnn.getMyOnlineId() );
	pktPingReply.setTimestamp( pktPingReq->getTimestamp() );
	sktBase->txPacket( pktPingReq->getSrcOnlineId(), &pktPingReply );
}

//============================================================================
void P2PEngine::onPktPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPingReply" );

	PktPingReply * pktPingReply = (PktPingReply *)pktHdr;
	int64_t timeDiffTmp = GetGmtTimeMs() - pktPingReply->getTimestamp();
	uint16_t timeDiff = timeDiffTmp > 30000 ? 30000 : (uint16_t)timeDiffTmp;

	std::string onlineName;
	BigListInfo * bigListInfo = m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( 0 != bigListInfo )
	{
		onlineName = bigListInfo->getOnlineName();
		bigListInfo->setPingTimeMs( timeDiff );
	}
	else
	{
		pktHdr->getSrcOnlineId().toHexString( onlineName );
	}
	
	LogMsg( LOG_DEBUG, "Ping %s Time ms %d", onlineName.c_str(), timeDiff );
}

//============================================================================
void P2PEngine::onPktImAliveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktImAliveReq skt id %s peer %s",
				sktBase->getSocketIdText().c_str(),
				sktBase->describePeerUser().c_str() );

	// do not respond to temp connections so if something went wrong it will eventually die
	if( !sktBase->isTempConnection() )
	{
        sktBase->setLastImAliveTimeTxMs( GetGmtTimeMs() );
		PktImAliveReply pktImAliveReply;
		pktImAliveReply.setSrcOnlineId( m_PktAnn.getMyOnlineId() );
		pktImAliveReply.setDestOnlineId( pktHdr->getSrcOnlineId() );

		sktBase->txPacketWithDestId( &pktImAliveReply );
	}
}

//============================================================================
void P2PEngine::onPktImAliveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktImAliveReply skt id %s peer %s",
				sktBase->getSocketIdText().c_str(),
				sktBase->describePeerUser().c_str() );

	sktBase->setLastImAliveTimeRxMs( GetGmtTimeMs() );
}

//============================================================================
void P2PEngine::onPktBlobSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobSendReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobSendReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobChunkReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobChunkReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobSendCompleteReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobSendCompleteReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktBlobXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktBlobXferErr" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostJoinReq" );
    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostJoinReply" );
    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostLeaveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostLeaveReq" );
	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostLeaveReply" );
	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUnJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostUnJoinReq" );
	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostUnJoinReply" );
	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostSearchReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostSearchReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostOfferReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktHostOfferReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFriendOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFriendOfferReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFriendOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktFriendOfferReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbGetReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbGetReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbSendReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbSendReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbChunkReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbChunkReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbGetCompleteReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbGetCompleteReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbSendCompleteReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbSendCompleteReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktThumbXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktThumbXferErr" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

// offers
//============================================================================
void P2PEngine::onPktOfferSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferSendReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferSendReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferChunkReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferChunkReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferSendCompleteReq" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferSendCompleteReply" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktOfferXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktOfferXferErr" );

    m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPushToTalkReq" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPushToTalkReply" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPushToTalkStart" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktPushToTalkStop" );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}


//============================================================================
void P2PEngine::onPktMembershipReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMembershipReq" );

	VxNetIdent* netIdent = pktHdr->getSrcOnlineId() == getMyOnlineId() ? getMyNetIdent() : m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( netIdent && !netIdent->isIgnored() && sktBase && sktBase->isConnected() )
	{
		PktAnnounce pktAnn;
		copyMyPktAnnounce( pktAnn );
		EFriendState myFriendshipToHim = netIdent->getMyFriendshipToHim();
		PktMembershipReply pktReply;
		pktReply.setCanPushToTalk( pktAnn.getPluginPermission( ePluginTypePushToTalk ) != eFriendStateIgnore && myFriendshipToHim >= pktAnn.getPluginPermission( ePluginTypePushToTalk ) );
		pktReply.setHostMembership( eHostTypeNetwork, getMembershipState( pktAnn, netIdent, ePluginTypeHostNetwork, myFriendshipToHim ) );
		pktReply.setHostMembership( eHostTypeConnectTest, getMembershipState( pktAnn, netIdent, ePluginTypeHostConnectTest, myFriendshipToHim ) );
		pktReply.setHostMembership( eHostTypeGroup, getMembershipState( pktAnn, netIdent, ePluginTypeHostGroup, myFriendshipToHim ) );
		pktReply.setHostMembership( eHostTypeChatRoom, getMembershipState( pktAnn, netIdent, ePluginTypeHostChatRoom, myFriendshipToHim ) );
        pktReply.setHostMembership( eHostTypeRandomConnect, getMembershipState( pktAnn, netIdent, ePluginTypeHostRandomConnect, myFriendshipToHim ) );

        sktBase->txPacket( netIdent->getMyOnlineId(), &pktReply );
	}
}

//============================================================================
void P2PEngine::onPktMembershipReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "P2PEngine::onPktMembershipReply" );

	//PktMembershipReply* pktReply = ( PktMembershipReply* )pktHdr;
	//if( pktReply && pktReply->isValidPktPrefix() )
	//{
	//	VxNetIdent* netIdent = pktHdr->getSrcOnlineId() == getMyOnlineId() ? getMyNetIdent() : m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	//	if( netIdent && !netIdent->isIgnored() && sktBase && sktBase->isConnected() )
	//	{
	//		MemberActive membership;
	//		membership.setCanPushToTalk( pktReply->getCanPushToTalk() );
	//		membership.setMembershipState( eHostTypeNetwork, pktReply->getHostMembership( eHostTypeNetwork ) );
	//		membership.setMembershipState( eHostTypeConnectTest, pktReply->getHostMembership( eHostTypeConnectTest ) );
	//		membership.setMembershipState( eHostTypeGroup, pktReply->getHostMembership( eHostTypeGroup ) );
	//		membership.setMembershipState( eHostTypeChatRoom, pktReply->getHostMembership( eHostTypeChatRoom ) );
	//		membership.setMembershipState( eHostTypeRandomConnect, pktReply->getHostMembership( eHostTypeRandomConnect ) );
	//	}
	//}
}

//============================================================================
EMembershipState P2PEngine::getMembershipState( PktAnnounce& myPktAnn, VxNetIdent* netIdent, enum EPluginType pluginType, enum EFriendState myFriendshipToHim )
{
	EMembershipState membershipState{ eMembershipStateNone };
	if( myFriendshipToHim == eFriendStateIgnore )
	{
		membershipState = eMembershipStateJoinDenied;
	}
	else if( myPktAnn.getPluginPermission( pluginType ) != eFriendStateIgnore )
	{
		if( ePluginTypeHostGroup == pluginType || ePluginTypeHostChatRoom == pluginType )
		{
			// look up joined state from group manager
			PluginBase* pluginBase = m_PluginMgr.findPlugin( ePluginTypeHostGroup );
			if( pluginBase )
			{
				membershipState = pluginBase->getMembershipState( netIdent );
			}
		}
		else
		{
			if( myFriendshipToHim >= myPktAnn.getPluginPermission( pluginType ) )
			{
				membershipState = eMembershipStateJoined;
			}
			else
			{
				membershipState = eMembershipStateCanBeRequested;
			}
		}
	}
	else
	{
		membershipState = eMembershipStateJoinDenied;
	}

	return membershipState;
}

//============================================================================
void P2PEngine::onPktHostInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE,  "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE,  "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );;

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktGroupieMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktFileInfoMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTestConnTestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTestConnTestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTestConnPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktTestConnPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktQueryHostUrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktQueryHostUrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	m_PluginMgr.handleNonSystemPackets( sktBase, pktHdr );
}

//============================================================================
void P2PEngine::onPktStreamCtrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	// steam control requests can only be sent to File Share server
	PluginBase* pluginBase = m_PluginMgr.getPlugin( ePluginTypeFileShareServer );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginFileShareServer*)pluginBase)->onPktStreamCtrlReq( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktStreamCtrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	// steam control replies can only be sent to File Share client
	PluginBase* pluginBase = m_PluginMgr.getPlugin( ePluginTypeFileShareClient );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginFileShareClient*)pluginBase)->onPktStreamCtrlReply( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktRandConnectReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	// steam control requests can only be sent to File Share server
	PluginBase* pluginBase = m_PluginMgr.getPlugin( ePluginTypeHostRandomConnect );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginRandomConnectHost*)pluginBase)->onPktRandConnectReq( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktRandConnectReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	// steam control replies can only be sent to File Share client
	PluginBase* pluginBase = m_PluginMgr.getPlugin(  ePluginTypeClientRandomConnect );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginRandomConnectClient*)pluginBase)->onPktRandConnectReply( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktFriendRequestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	PluginBase* pluginBase = m_PluginMgr.getPlugin( ePluginTypeFriendRequest );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginFriendRequest*)pluginBase)->onPktFriendRequestReq( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktFriendRequestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );

	PluginBase* pluginBase = m_PluginMgr.getPlugin(  ePluginTypeFriendRequest );
	if( pluginBase )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getDestOnlineId() );
		((PluginFriendRequest*)pluginBase)->onPktFriendRequestReply( sktBase, pktHdr, netIdent );
	}
}

//============================================================================
void P2PEngine::onPktAdminAvail( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	if( LogEnabled( eLogPkt ) ) LogModule( eLogPkt, LOG_VERBOSE, "%s", __func__ );
	PktAdminAvail* pktAdminAvail = (PktAdminAvail*)pktHdr;
	GroupieId adminGroupieId = pktAdminAvail->getAdminGroupieId();
	bool adminAvail = pktAdminAvail->getAdminAvailable();
	if( adminGroupieId.isValid() && IsHostARelayForUsers( adminGroupieId.getHostType() ) )
	{
		getToGui().toGuiAdminAvail( adminGroupieId, adminAvail );
	}
}
