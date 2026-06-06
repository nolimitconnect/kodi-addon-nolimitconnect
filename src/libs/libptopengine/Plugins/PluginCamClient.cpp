//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginCamClient.h"
#include "PluginMgr.h"
#include "TxSession.h"
#include "RxSession.h"

#include <P2PEngine/P2PEngine.h>
#include <MediaProcessor/MediaProcessor.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxDebug.h>

#include <NetLib/VxSktBase.h>

#include <PktLib/PktsPluginOffer.h>
#include <PktLib/PktsSession.h>
#include <PktLib/PktsVideoFeed.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

// #define DEBUG_PLUGIN_LOCK

//============================================================================
PluginCamClient::PluginCamClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_PluginSessionMgr( engine, *this, pluginMgr )
, m_VoiceFeedMgr( engine, *this, m_PluginSessionMgr )
, m_VideoFeedMgr( engine, *this, m_PluginSessionMgr )
{
	m_ePluginType = ePluginTypeCamClient;
}

//============================================================================
void PluginCamClient::setIsPluginInSession( bool isInSession )
{
	setIsServerInSession( isInSession );
	IToGui::getIToGui().toGuiPluginStatus( m_ePluginType, isInSession ? 1 : 0, isInSession ? m_PluginSessionMgr.getSessionCount() : -1 );
}

//============================================================================
// override this by plugin to create inherited RxSession
RxSession * PluginCamClient::createRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	return new RxSession( sktBase, onlineId, getPluginType() );
}

//============================================================================
// override this by plugin to create inherited TxSession
TxSession * PluginCamClient::createTxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	return new TxSession( sktBase, onlineId, getPluginType() );
}

//============================================================================
void PluginCamClient::callbackVideoJpg( VxGUID& feedId, std::shared_ptr<CamJpgVideo>& jpgVideo )
{
	m_PluginMgr.pluginApiPlayJpgVideo( m_ePluginType, m_MyIdent, jpgVideo );
}

//============================================================================
void PluginCamClient::sendVidPkt( VxPktHdr* vidPkt, bool requiresAck )
{
	if( m_PluginSessionMgr.getSessionCount() )
	{
		auto sessionList = m_PluginSessionMgr.getSessions();
		PluginBase::AutoPluginLock pluginMutexLock( this );
		for( auto iter = sessionList.begin(); iter != sessionList.end(); ++iter )
		{
			PluginSessionBase* sessionBase = *iter;
			if( sessionBase->isTxSession() )
			{
				TxSession * poSession = (TxSession *)sessionBase;
				if( poSession 
					&& ( !requiresAck  || (10 > poSession->getOutstandingAckCnt() ) ) )
				{
					m_PluginMgr.pluginApiTxPacket(	m_ePluginType, 
													poSession->getSendToId(), 
													poSession->getSkt(), 
													vidPkt ); 
					if( requiresAck )
					{
						poSession->setOutstandingAckCnt( poSession->getOutstandingAckCnt() + 1 );
					}
				}
			}
		}

#if defined(DEBUG_PLUGIN_LOCK)
		LogMsg( LOG_VERBOSE, "PluginCamClient::sendVidPkt unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	}
}

//============================================================================
void PluginCamClient::callbackOpusPkt( PktVoiceReq * pktOpusAudio )
{
	m_VoiceFeedMgr.callbackOpusPkt( pktOpusAudio );
}

//============================================================================
void PluginCamClient::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	m_VoiceFeedMgr.callbackAudioOutSpaceAvail( freeSpaceLenBytes );
}

//============================================================================
void PluginCamClient::callbackVideoPktPic( VxGUID& feedId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum )
{
	sendVidPkt( pktVid, true );
}

//============================================================================
void PluginCamClient::callbackVideoPktPicChunk( VxGUID& feedId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum )
{
	sendVidPkt( pktVid, false );
}

//============================================================================
//! called to start service or session with remote friend
bool PluginCamClient::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	LogMsg( LOG_VERBOSE, "PluginCamClient::%s %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
    bool result{false};
	AutoPluginLock pluginMutexLock( this );
	RxSession* rxSession = m_PluginSessionMgr.findRxSessionByOnlineId( onlineId, true );
	if( !rxSession )
	{
		std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
		if( sktBase && sktBase->isConnected() )
		{
			rxSession = m_PluginSessionMgr.findOrCreateRxSessionWithOnlineId( onlineId, sktBase, true, lclSessionId );
		}
	}

	if( rxSession )
	{
		if( lclSessionId.isValid() )
		{
			rxSession->setLclSessionId( lclSessionId );
		}
		else
		{
			lclSessionId = rxSession->getLclSessionId();
		}

        bool reqSent = requestCamSession( rxSession, false );
        if( reqSent )
        {
            setIsPluginInSession( true );
            result = true;
        }
        else
        {
            LogMsg( LOG_VERBOSE, "PluginCamClient::%s request send failed to %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
        }
	}
	else
	{
        LogMsg( LOG_VERBOSE, "PluginCamClient::%s could not connect to %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	}

    return result;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginCamClient::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	LogMsg( LOG_VERBOSE, "PluginCamClient::fromGuiStopPluginSession" );
	m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
	PluginBase::AutoPluginLock pluginMutexLock( this );
	bool isMyself = onlineId == m_MyIdent->getMyOnlineId();
	if( isMyself )
	{
		m_Engine.setHasSharedWebCam(false);
		// don't want video capture anymore
		m_VideoFeedMgr.fromGuiStopPluginSession( true, eMediaModuleCamClient, onlineId, false );
		if( true == fromGuiIsPluginInSession() )
		{
			setIsPluginInSession( false );

			PktVideoFeedStatus oPkt;
			oPkt.setFeedStatus( eFeedStatusOffline );

			// Send offline status to all TxSessions before removal.
			// Use a copy of the sessions list to avoid iterator invalidation.
			{
				auto sessionListCopy = m_PluginSessionMgr.getSessions();
				for( auto session : sessionListCopy )
				{
					if( session->isTxSession() )
					{
						TxSession* txSession = (TxSession*)session;
						if( txSession && txSession->getSkt() )
						{
							oPkt.setLclSessionId( txSession->getLclSessionId() );
							oPkt.setRmtSessionId( txSession->getRmtSessionId() );
							m_PluginMgr.pluginApiTxPacket( m_ePluginType, txSession->getSendToId(), txSession->getSkt(), &oPkt );
						}
					}
				}
			}

			// Now safely remove all TxSessions for this plugin using the SessionMgr.
			// This ensures proper synchronization and prevents use-after-free.
			auto sessionList = m_PluginSessionMgr.getSessions();
			while( !sessionList.empty() )
			{
				PluginSessionBase* session = sessionList[0];
				if( session->isTxSession() )
				{
					m_PluginSessionMgr.removeTxSessionByOnlineId( session->getSendToId(), true );
					// List will be updated by the remove call; refresh our reference
					sessionList = m_PluginSessionMgr.getSessions();
				}
				else if( session->isRxSession() )
				{
					m_PluginSessionMgr.removeRxSessionByOnlineId( session->getSendToId(), true );
					// List will be updated by the remove call; refresh our reference
					sessionList = m_PluginSessionMgr.getSessions();
				}
				else
				{
					// Skip unknown session types
					break;
				}
			}
		}
	}
	else
	{
		RxSession* rxSession = (RxSession*)m_PluginSessionMgr.findRxSessionByOnlineId( onlineId, true );
		if( rxSession )
		{
			PktSessionStopReq oPkt;
			oPkt.setLclSessionId( rxSession->getLclSessionId() );
			oPkt.setRmtSessionId( rxSession->getRmtSessionId() );
			m_PluginMgr.pluginApiTxPacket( m_ePluginType, rxSession->getSendToId(), rxSession->getSkt(), &oPkt );
			lclSessionId = rxSession->getLclSessionId();
		}

		m_PluginSessionMgr.removeSession( true, onlineId, lclSessionId, eOfferResponseEndSession, true );
	}
}

//============================================================================
bool PluginCamClient::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	// for cam server we really want to know if server is running
	return getIsServerInSession();
}

//============================================================================
EPluginAccess PluginCamClient::canAcceptNewSession( VxNetIdent* netIdent )
{
	EFriendState eHisPermissionToMe = netIdent->getHisFriendshipToMe();
	EFriendState eMyPermissionToHim = netIdent->getMyFriendshipToHim();

	if( (eFriendStateIgnore == eHisPermissionToMe) ||
		(eFriendStateIgnore == eMyPermissionToHim) )
	{
		return ePluginAccessIgnored;
	}
	EFriendState ePermissionLevel = this->m_MyIdent->getPluginPermission(m_ePluginType);
	if( eFriendStateIgnore == ePermissionLevel )
	{
		return ePluginAccessDisabled;
	}
	if( ePermissionLevel > eMyPermissionToHim )
	{
		return ePluginAccessLocked;
	}

	return ePluginAccessOk;
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool PluginCamClient::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )	
{
	LogMsg( LOG_VERBOSE, " PluginCamClient::fromGuiMakePluginOffer %s", m_Engine.describeUser( onlineId ).c_str() );

	std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
	if( sktBase && sktBase->isConnected() )
	{
		VxGUID& lclSessionId = offerInfo.getOfferId();

		PluginBase::AutoPluginLock pluginMutexLock( this );

		PktPluginOfferReq pktReq;
		pktReq.setLclSessionId( lclSessionId );
		pktReq.setRmtSessionId( lclSessionId );
		pktReq.setPluginType( getPluginType() );
		offerInfo.addToBlob( pktReq.getBlobEntry() );
		pktReq.calcPktLen();

        // force session to be created so have session to lookup on reply
		RxSession * rxSession = (RxSession *)m_PluginSessionMgr.findOrCreateRxSessionWithSessionId( lclSessionId, sktBase, onlineId, true );
        if( 0 != rxSession )
        {
            if( true == m_PluginMgr.pluginApiTxPacket(	m_ePluginType,
                                                        onlineId,
                                                        sktBase,
                                                        &pktReq ) )
            {
                m_VoiceFeedMgr.enableAudioReceive( getPluginType(), rxSession->getSendToId() );
                LogMsg( LOG_VERBOSE, " PluginCamClient::fromGuiMakePluginOffer success");
                return true;
            }
            else
            {
                LogMsg( LOG_VERBOSE, " PluginCamClient::fromGuiMakePluginOffer failed to send pkt");
            }
        }
        else
        {
            LogMsg( LOG_ERROR, " PluginCamClient::fromGuiMakePluginOffer failed to create session");
        }
    }

	return false;
}

//============================================================================
bool PluginCamClient::requestCamSession( RxSession* rxSession, bool	bWaitForSuccess )
{
	PktSessionStartReq oPkt;
	oPkt.setLclSessionId( rxSession->getLclSessionId() );
    bool result = m_PluginMgr.pluginApiTxPacket(	m_ePluginType,
													rxSession->getSendToId(), 
													rxSession->getSkt(), 
													&oPkt );
    if( result )
    {
        m_VoiceFeedMgr.enableAudioReceive( getPluginType(), rxSession->getSendToId() );
    }

    if( ( true == result ) && bWaitForSuccess )
	{
        result = false;
		bool bResponseReceived = rxSession->waitForResponse( 9000 );
		if( bResponseReceived )
		{
			if( rxSession->getIsSessionStarted() )
			{
                result = true;
			}
		}
	}
	
    return result;
}

//============================================================================
bool PluginCamClient::stopCamSession( VxNetIdent* netIdent,	std::shared_ptr<VxSktBase>& sktBase )
{
	LogMsg( LOG_ERROR, "PluginCamClient::stopCamSession");
	PktSessionStopReq oPkt;
	bool bSuccess = m_PluginMgr.pluginApiTxPacket(	m_ePluginType,
												netIdent->getMyOnlineId(),
												sktBase,
												&oPkt );
	VxGUID sessionId;
	m_PluginSessionMgr.removeSession( false,
												netIdent->getMyOnlineId(),
												sessionId,
												eOfferResponseEndSession,
												true );

	return bSuccess;
}

//============================================================================
//! packet with remote users offer
void PluginCamClient::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if( !isAccessAllowed( netIdent, true, "onPktPluginOfferReq" ) )
	{
		return;
	}

	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktPluginOfferReq" );
	PktPluginOfferReq* pktOfferReq = ( PktPluginOfferReq * )pktHdr;
	OfferBaseInfo offerInfo;
	if( offerInfo.extractFromBlob( pktOfferReq->getBlobEntry() ) )
	{
		PktPluginOfferReply pktReply;
		pktReply.setLclSessionId( pktOfferReq->getLclSessionId() );
		pktReply.setRmtSessionId( pktOfferReq->getRmtSessionId() );
		pktReply.setPluginType( getPluginType() );
		offerInfo.addToBlob( pktReply.getBlobEntry() );
		pktReply.calcPktLen();

		PluginBase::AutoPluginLock pluginMutexLock( this );
		if( getIsServerInSession() && (ePluginAccessOk == canAcceptNewSession( netIdent )) )
		{
			TxSession* txSession = (TxSession*)m_PluginSessionMgr.findOrCreateTxSessionWithOnlineId( netIdent->getMyOnlineId(), sktBase, true );
			pktReply.setLclSessionId( txSession->getLclSessionId() );
			pktReply.setOfferResponse( eOfferResponseAccept );
		}
		else
		{
			LogMsg( LOG_VERBOSE, "PluginCamClient::onPktPluginOfferReq REJECTED in session %d canAcceptNewSession %d",
				getIsServerInSession(), canAcceptNewSession( netIdent ) );
			pktReply.setOfferResponse( eOfferResponseReject );
		}

		m_PluginMgr.pluginApiTxPacket( m_ePluginType,
			netIdent->getMyOnlineId(),
			sktBase,
			&pktReply );
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginCamClient::onPktPluginOfferReq failed to extract offerInfo " );
	}
}

//============================================================================
//! packet with remote users reply to offer
void PluginCamClient::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktPluginOfferReply* pktReply = (PktPluginOfferReply*)pktHdr;
	EOfferResponse offerResponse = pktReply->getOfferResponse();
	PluginBase::handleToGuiOfferResponse( netIdent, pktReply );

	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktPluginOfferReply %d", offerResponse );
	PluginBase::AutoPluginLock pluginMutexLock( this );
	if( offerResponse == eOfferResponseAccept )
	{
		RxSession * rxSession = (RxSession *)m_PluginSessionMgr.findOrCreateRxSessionWithOnlineId( netIdent->getMyOnlineId(), sktBase, true );
		rxSession->setOfferResponse( offerResponse );
	}
}

//============================================================================
void PluginCamClient::onPktSessionStartReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if( !isAccessAllowed( netIdent, true, "onPktSessionStartReq" ) )
	{
		return;
	}

	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReq" );
	PktSessionStartReply oPkt;
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReq lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	PluginBase::AutoPluginLock pluginMutexLock( this );
	if( getIsServerInSession() && ( ePluginAccessOk == canAcceptNewSession( netIdent ) ) ) 
	{
		oPkt.setOfferResponse(eOfferResponseAccept);
        TxSession * txSession = (TxSession *)m_PluginSessionMgr.findOrCreateTxSessionWithOnlineId( netIdent->getMyOnlineId(), sktBase, true );
        if( 0 == txSession )
        {
            LogMsg( LOG_ERROR, "PluginCamClient::onPktSessionStartReq failed to create or find session" );
            oPkt.setOfferResponse( eOfferResponseReject );
        }
	}
	else
	{
		oPkt.setOfferResponse( eOfferResponseReject );
	}

	m_PluginMgr.pluginApiTxPacket(	m_ePluginType, 
									netIdent->getMyOnlineId(), 
									sktBase, 
									&oPkt ); 
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReq unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
}

//============================================================================
void PluginCamClient::onPktSessionStartReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktSessionStartReply * poPkt = (PktSessionStartReply *)pktHdr;
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReply lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	PluginBase::AutoPluginLock pluginMutexLock( this );
	RxSession * poSession = (RxSession *)m_PluginSessionMgr.findRxSessionByOnlineId( netIdent->getMyOnlineId(), true );
	if( poSession )
	{
		LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReply %d", poPkt->getOfferResponse() );
		if( eOfferResponseAccept == poPkt->getOfferResponse() )
		{
			poSession->setIsSessionStarted( true );
		}
		else
		{
			poSession->setIsSessionStarted( false );
		}

		poSession->signalResponseRecieved();
	}

#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktSessionStartReply unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
}

//============================================================================
void PluginCamClient::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.removeTxSessionByOnlineId( netIdent->getMyOnlineId(), false );
	if( getIsServerInSession() )
	{
		IToGui::getIToGui().toGuiPluginStatus( m_ePluginType, 1, m_PluginSessionMgr.getTxSessionCount() );
	}
}

//============================================================================
void PluginCamClient::onPktSessionStopReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_ERROR, "PluginCamClient::onPktSessionStopReply" );
}

//============================================================================
void PluginCamClient::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void PluginCamClient::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktVideoFeedStatus * pktVideoStatus = ( PktVideoFeedStatus * )pktHdr;
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedStatus lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)

	RxSession* rxSession = (RxSession*)m_PluginSessionMgr.findRxSessionByOnlineId( netIdent->getMyOnlineId(), true );
	if( rxSession )
	{
		LogMsg( LOG_INFO, "PluginCamServer::onPktVideoFeedStatus %d", pktVideoStatus->getFeedStatus() );
		if( eFeedStatusOnline != pktVideoStatus->getFeedStatus() )
		{
			//IToGui::getIToGui().toGuiRxedOfferReply( netIdent,
			//	m_ePluginType,
			//	0,				// plugin defined data
			//	(eFeedStatusBusy == pktVideoStatus->getFeedStatus()) ? eOfferResponseBusy : eOfferResponseEndSession,
			//	0,
			//	0,
			//	pktVideoStatus->getRmtSessionId(),
			//	pktVideoStatus->getLclSessionId() );

			VxGUID sessionId = rxSession->getLclSessionId();
			m_VoiceFeedMgr.enableAudioReceive( false, netIdent->getMyOnlineId() );
			m_PluginSessionMgr.removeSession( true, netIdent->getMyOnlineId(), sessionId, eOfferResponseEndSession, true );
		}
	}

#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedStatus unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
}

//============================================================================
void PluginCamClient::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//LogMsg( LOG_VERBOSE, "PluginCamClient::onPktCastPic\n" );
    // might be relayed. use source id instead of connection netIdent;
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    if( !srcOnlineId.isValid() )
    {
        LogMsg( LOG_VERBOSE, "PluginCamClient::%s invalid source id", __func__ );
        return;
    }

	PktVideoFeedPicAck oPkt;
    m_PluginMgr.pluginApiTxPacket( m_ePluginType, srcOnlineId, sktBase, &oPkt );

	PktVideoFeedPic * poPktCastPic = ( PktVideoFeedPic * )pktHdr;
	if( poPktCastPic->getTotalDataLen() == poPktCastPic->getThisDataLen() )
	{
        m_Engine.getMediaProcessor().processFriendVideoFeed( srcOnlineId, poPktCastPic->getDataPayload(), poPktCastPic->getTotalDataLen(), poPktCastPic->getMotionDetect() );
#if defined(DEBUG_PLUGIN_LOCK)
		LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPic lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
		PluginBase::AutoPluginLock pluginMutexLock( this );
        RxSession * poSession = (RxSession *)m_PluginSessionMgr.findRxSessionByOnlineId( srcOnlineId, true );
		if( poSession )
		{
			if( poSession->getVideoFeedPkt() )
			{
				delete poSession->getVideoFeedPkt();
				poSession->setVideoFeedPkt( nullptr );
			}
		}

#if defined(DEBUG_PLUGIN_LOCK)
		LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPic unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	}
	else
	{
#if defined(DEBUG_PLUGIN_LOCK)
		LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPic chunk lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
		// picture was too big for one packet
		PluginBase::AutoPluginLock pluginMutexLock( this );

        RxSession * poSession = (RxSession *)m_PluginSessionMgr.findRxSessionByOnlineId( srcOnlineId, true );
		if( poSession )
		{
			if( poSession->getVideoFeedPkt() )
			{
				delete poSession->getVideoFeedPkt();
				poSession->setVideoFeedPkt( NULL );
			}
		}
		else
		{
            poSession = (RxSession *)m_PluginSessionMgr.findOrCreateRxSessionWithOnlineId( srcOnlineId, sktBase, true );
            LogMsg( LOG_VERBOSE, "PluginCamClient::%s creating rx session because could not be found", __func__ );
		}

		PktVideoFeedPic * poPic = ( PktVideoFeedPic * ) new char[ sizeof( PktVideoFeedPic ) + 16 + poPktCastPic->getTotalDataLen() ];
		poPic->setThisDataLen( poPktCastPic->getThisDataLen() );
		memcpy( poPic, poPktCastPic, poPktCastPic->getPktLength() );
		poSession->setVideoFeedPkt( poPic );

		// wait for rest of picture to arrive
#if defined(DEBUG_PLUGIN_LOCK)
		LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPic chunk unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	}
}

//============================================================================
void PluginCamClient::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    // might be relayed. use source id instead of connection netIdent;
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    if( !srcOnlineId.isValid() )
    {
        LogMsg( LOG_VERBOSE, "PluginCamClient::%s invalid source id", __func__ );
        return;
    }

	PktVideoFeedPicChunk * poPktPicChunk = ( PktVideoFeedPicChunk * )pktHdr;
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPicChunk lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
	PluginBase::AutoPluginLock pluginMutexLock( this );

    RxSession * poSession = (RxSession *)m_PluginSessionMgr.findRxSessionByOnlineId( srcOnlineId, true );
	if( !poSession )
	{
		// this may occur normally with dropped frames due to slow acknowledgment
		//LogMsg( LOG_ERROR, "PluginCamClient::onPktVideoFeedPicChunk: could not find RxSession\n");
		return;
	}

	PktVideoFeedPic * poPktCastPic = poSession->getVideoFeedPkt();
	if( poPktCastPic && 
		(poPktCastPic->getTotalDataLen() >=  (poPktCastPic->getThisDataLen() + poPktPicChunk->getThisDataLen() ) ) )
	{
		memcpy( &poPktCastPic->getDataPayload()[ poPktCastPic->getThisDataLen() ], 
				poPktPicChunk->getDataPayload(), 
				poPktPicChunk->getThisDataLen() );
		poPktCastPic->setThisDataLen( poPktCastPic->getThisDataLen() + poPktPicChunk->getThisDataLen() );
		if( poPktCastPic->getThisDataLen() >= poPktCastPic->getTotalDataLen() )
		{
			// all of picture arrived
			PktVideoFeedPicAck oPkt;
			m_PluginMgr.pluginApiTxPacket(	m_ePluginType, 
                srcOnlineId,
				sktBase, 
                &oPkt );

            m_Engine.getMediaProcessor().processFriendVideoFeed( srcOnlineId, poPktCastPic->getDataPayload(), poPktCastPic->getTotalDataLen(), poPktCastPic->getMotionDetect() );

            //std::shared_ptr<uint8_t> jpgData( new uint8_t[poPktCastPic->getTotalDataLen()] );
            //memcpy( jpgData.get(), poPktCastPic->getDataPayload(), poPktCastPic->getTotalDataLen() );
            //std::shared_ptr<CamJpgVideo> jpgVideo( new CamJpgVideo( jpgData, poPktCastPic->getTotalDataLen(), poPktCastPic->getMotionDetect() ) );

            //m_PluginMgr.pluginApiPlayJpgVideo( m_ePluginType, netIdent, jpgVideo );
			
			delete poSession->getVideoFeedPkt();
			poSession->setVideoFeedPkt( NULL );
		}
	}

#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPicChunk unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
}

//============================================================================
void PluginCamClient::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPicAck lock" );
#endif //defined(DEBUG_PLUGIN_LOCK)

	PluginBase::AutoPluginLock pluginMutexLock( this );

	TxSession * poSession = (TxSession *)m_PluginSessionMgr.findTxSessionByOnlineId( true, netIdent->getMyOnlineId() );
	if( NULL == poSession )
	{
		LogMsg( LOG_ERROR, "PluginCamClient::onPktVideoFeedPicAck: could not find TxSession");
		return;
	}

	poSession->decrementOutstandingAckCnt();

#if defined(DEBUG_PLUGIN_LOCK)
	LogMsg( LOG_VERBOSE, "PluginCamClient::onPktVideoFeedPicAck unlock" );
#endif //defined(DEBUG_PLUGIN_LOCK)
}

//============================================================================
void PluginCamClient::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginCamClient::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginCamClient::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginCamClient::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onConnectionLost( sktBase );
	if( getIsServerInSession() )
	{
		IToGui::getIToGui().toGuiPluginStatus( m_ePluginType, 1, m_PluginSessionMgr.getTxSessionCount() );
	}
}

//============================================================================
void PluginCamClient::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
	if( getIsServerInSession() )
	{
		IToGui::getIToGui().toGuiPluginStatus( m_ePluginType, 1, m_PluginSessionMgr.getTxSessionCount() );
	}
}

//============================================================================
void PluginCamClient::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
	if( !isOnline && getIsServerInSession() )
	{
		IToGui::getIToGui().toGuiPluginStatus( m_ePluginType, 1, m_PluginSessionMgr.getTxSessionCount() );
	}
}

//============================================================================
void PluginCamClient::onNetworkConnectionReady( bool requiresRelay )
{
	if( eFriendStateIgnore != m_MyIdent->getPluginPermission( getPluginType() ) )
	{
		// automatically start web cam on startup if enabled
		enableCamServerService( true );
	}
}

//============================================================================
void PluginCamClient::fromGuiUpdatePluginPermission( EPluginType pluginType, EFriendState pluginPermission )
{
	if( pluginType == getPluginType() )
	{
		enableCamServerService( eFriendStateIgnore != pluginPermission );
	}
}

//============================================================================
void PluginCamClient::stopAllSessions( void )
{
	if( true == fromGuiIsPluginInSession() )
	{
		setIsPluginInSession( false );

		// tell everyone we are no longer online
		PktVideoFeedStatus oPkt;
		oPkt.setFeedStatus( eFeedStatusOffline );

		auto sessionList = m_PluginSessionMgr.getSessions();
		for( auto iter = sessionList.begin(); iter != sessionList.end(); )
		{
			PluginSessionBase* sessionBase = *iter;
			if( sessionBase->isTxSession() )
			{
				TxSession* poSession = (TxSession*)sessionBase;
				if( poSession->getSkt() )
				{
					oPkt.setLclSessionId( poSession->getLclSessionId() );
					oPkt.setRmtSessionId( poSession->getRmtSessionId() );
					m_PluginMgr.pluginApiTxPacket( m_ePluginType, poSession->getSendToId(), poSession->getSkt(), &oPkt );
					iter = sessionList.erase( iter );
					delete poSession;
				}
				else
				{
					++iter;
				}
			}
			else
			{
				++iter;
			}
		}
	}

	m_VideoFeedMgr.stopAllSessions( getMediaModule(), getPluginType() );
	m_VoiceFeedMgr.stopAllSessions();
}

//============================================================================
void PluginCamClient::enableCamServerService( bool enable )
{
	if( m_IsCamServiceEnabled == enable )
	{
		return;
	}

	m_IsCamServiceEnabled = enable;

	if( m_IsCamServiceEnabled )
	{
		m_Engine.setHasSharedWebCam( true );
		// request video capture
		m_VideoFeedMgr.fromGuiStartPluginSession( false, getMediaModule(), m_Engine.getMyOnlineId(), false);
		setIsPluginInSession( true );
	}
	else
	{
		// stop video capture
		m_VideoFeedMgr.fromGuiStopPluginSession( false, getMediaModule(), m_Engine.getMyOnlineId(), false );
		m_Engine.setHasSharedWebCam( false );
		stopAllSessions();
		setIsPluginInSession( false );
	}
}
