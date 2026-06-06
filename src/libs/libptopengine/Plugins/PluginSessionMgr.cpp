//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginSessionMgr.h"

#include <OfferBase/OfferBaseInfo.h>
#include <P2PEngine/P2PEngine.h>

#include "PluginBase.h"
#include "PluginMgr.h"
#include "PluginSessionBase.h"

#include <NetLib/VxSktBase.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/PktsSession.h>

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

//============================================================================ 
PluginSessionMgr::PluginSessionMgr( P2PEngine& engine, PluginBase& plugin, PluginMgr& pluginMgr )
: SessionMgrBase( engine, plugin, pluginMgr )
{
}

//============================================================================
PluginSessionBase* PluginSessionMgr::findPluginSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->getLclSessionId() == sessionId )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
PluginSessionBase* PluginSessionMgr::findPluginSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->getSendToId() == onlineId )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
void PluginSessionMgr::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);
		if( poOldSkt == session->getSkt() )
		{
			session->setSkt( poNewSkt );
		}
	}
}

//============================================================================
//! called when connection is lost
void PluginSessionMgr::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	bool sktIsDisconnected = !sktBase->isConnected();
	// deadlock occurs here if use continuous lock so get the session to erase but dont end session until lock is removed 
	bool bErased = true;
	while( bErased ) 
	{
		bErased = false;
		PluginSessionBase* sessionBase = nullptr;

		m_Plugin.lockPlugin();

		for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
		{
			sessionBase = (*iter);
			if( ( netIdent->getMyOnlineId() == sessionBase->getSendToId() )
				|| ( sktIsDisconnected && ( sessionBase->getSkt() == sktBase ) ) )
			{
				if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::onContactWentOffline erasing session for %s", 
					m_Engine.describeUser( sessionBase->getSendToId() ).c_str() );
				bErased = true;
				break;
			}
		}

		m_Plugin.unlockPlugin();
		if( bErased )
		{
			doEndAndEraseSession( sessionBase, eOfferResponseUserOffline, false );
		}
	}
}

//============================================================================
void PluginSessionMgr::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	if( !isOnline )
	{
		cancelSessionByOnlineId( connectId.getUserOnlineId() );
	}
}

//============================================================================
//! called when connection is lost
void PluginSessionMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	bool bErased = true;

	while( bErased ) 
	{
		bErased = false;
		PluginSessionBase* sessionBase = nullptr;

		m_Plugin.lockPlugin();

		for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
		{
			sessionBase = (*iter);
			if( sktBase == sessionBase->getSkt() )
			{
				if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::onConnectionLost erasing session for %s", m_Engine.describeUser( sessionBase->getSendToId() ).c_str() );
				bErased = true;
				break;
			}
		}

		m_Plugin.unlockPlugin();
		if( bErased )
		{
			doEndAndEraseSession( sessionBase, eOfferResponseUserOffline, false );
		}
	}
}

//============================================================================
void PluginSessionMgr::cancelSessionByOnlineId( VxGUID& onlineId )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	bool bErased = true;

	while( bErased ) 
	{
		bErased = false;
		PluginSessionBase* sessionBase = nullptr;

		m_Plugin.lockPlugin();

		for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
		{
			sessionBase = (*iter);
			if( sessionBase->getSendToId() == onlineId )
			{
				if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::cancelSessionByOnlineId erasing session for %s", 
					m_Engine.describeUser( sessionBase->getSendToId() ).c_str() );
				bErased = true;
				break;
			}
		}

		m_Plugin.unlockPlugin();
		if( bErased )
		{
			doEndAndEraseSession( sessionBase, eOfferResponseCancelSession, false );
		}
	}
}

//============================================================================
void PluginSessionMgr::doEndAndEraseSession( PluginSessionBase* sessionBase, EOfferResponse offerResponse, bool pluginIsLocked, bool sendSessionStop )
{
	if( sendSessionStop )
	{
		m_Plugin.sendSessionStop( sessionBase->getSkt(), sessionBase );
	}
	m_Engine.getToGui().toGuiPluginSessionEnded( sessionBase->getSendToId(), getPluginType(), sessionBase->getLclSessionId() );
	m_Plugin.onSessionEnded( sessionBase, pluginIsLocked, offerResponse );

	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		if( sessionBase == (*iter) )
		{
			m_aoSessions.erase(iter);
			delete sessionBase;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================
bool PluginSessionMgr::fromGuiIsPluginInSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID lclSessionId )
{
	bool isInSesion = false;

	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	if( lclSessionId.isValid() )
	{
		for( auto session : m_aoSessions )
		{
			if( session->getLclSessionId() == lclSessionId )
			{
				isInSesion = true;
				break;
			}
		}
	}

	if( !isInSesion )
	{
		for( auto session : m_aoSessions )
		{
			if( session->getSendToId() == onlineId )
			{
				isInSesion = true;
				break;
			}
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, " PluginSessionMgr::%s in session? %d", __func__, isInSesion );
	return isInSesion;
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool PluginSessionMgr::fromGuiMakePluginOffer( bool pluginIsLocked, VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	bool offerSentResult = false;

	VxGUID& lclSessionId = offerInfo.getOfferId();
	std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
	if( !sktBase || !sktBase->isConnected() )
	{
		return false;
	}

	PluginSessionBase* pluginSession{ nullptr };
	if( lclSessionId.isValid() && ( false == isPluginSingleSession() ) )
	{
		pluginSession = findOrCreateP2PSessionWithSessionId( lclSessionId, sktBase, onlineId, pluginIsLocked );
	}
	else
	{
		pluginSession = findOrCreateP2PSessionWithOnlineId( onlineId, sktBase, pluginIsLocked, lclSessionId );
	}

	if( pluginSession )
	{
		if( !pluginSession->getRmtSessionId().isValid() )
		{
			// might need to send a cancel before is answered and if we sent then remote user setup session with same session id
			// if we get a resonse the remote session id will get set to the response remote session id
			pluginSession->setRmtSessionId( lclSessionId );
		}

		PktPluginOfferReq pktReq;

		pktReq.setLclSessionId( pluginSession->getLclSessionId() );
		pktReq.setRmtSessionId( pluginSession->getRmtSessionId() );

		offerInfo.addToBlob( pktReq.getBlobEntry() );
		pktReq.calcPktLen();

		offerSentResult = m_Plugin.txPacket( pluginSession->getSendToId(), pluginSession->getSkt(), &pktReq );
	}

	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, " PluginSessionMgr::%s offer sent? %d to %s", __func__, 
		offerSentResult, m_Engine.describeUser( onlineId ).c_str() );
	return offerSentResult;
}

//============================================================================
//! handle reply to offer
bool PluginSessionMgr::fromGuiOfferReply( bool pluginIsLocked, VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	PluginSessionBase* baseSession = nullptr;
	VxGUID& lclSessionId = offerInfo.getOfferId();
	EOfferResponse offerResponse = offerInfo.getOfferResponse();
	if( lclSessionId.isValid() && ( false == isPluginSingleSession() ) )
	{
		baseSession = findPluginSessionBySessionId( lclSessionId, pluginIsLocked );
	}
	else
	{
		baseSession = findPluginSessionByOnlineId( onlineId, pluginIsLocked );
	}

	bool responseSent = false;
	if( baseSession )
	{
		baseSession->setOfferResponse( offerResponse );
		baseSession->setLclSessionId( lclSessionId );

		PktPluginOfferReply pktReply;
		pktReply.setOfferResponse( offerResponse );
		pktReply.setLclSessionId( baseSession->getLclSessionId() );
		pktReply.setRmtSessionId( baseSession->getRmtSessionId() );
		offerInfo.addToBlob( pktReply.getBlobEntry() );
		if( m_Plugin.txPacket( baseSession->getSendToId(), baseSession->getSkt(), &pktReply ) )
		{
			responseSent = true;
		}

		if( ( false == responseSent )
			|| ( eOfferResponseReject == offerResponse )
			|| ( eOfferResponseCancelSession == offerResponse )
			|| ( eOfferResponseEndSession == offerResponse ) )
		{
			removeSession( pluginIsLocked, onlineId, lclSessionId, offerResponse );
		}
		else if( eOfferResponseAccept == offerResponse )
		{
			m_Plugin.onSessionStart( baseSession, pluginIsLocked );
		}
	}
	else
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, " PluginSessionMgr::fromGuiOfferReply: OFFER NOT FOUND");
	}

	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, " PluginSessionMgr::%s response sent? %d to %s", __func__,
		responseSent, m_Engine.describeUser( onlineId ).c_str() );
	return responseSent;
}

//============================================================================
void PluginSessionMgr::fromGuiStopPluginSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID lclSessionId )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, " PluginSessionMgr::%s user %s", __func__,
		m_Engine.describeUser( onlineId ).c_str() );
	removeSession( pluginIsLocked, onlineId, lclSessionId, eOfferResponseEndSession );
}

//============================================================================
void PluginSessionMgr::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdentIn )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    VxNetIdent* srcNetIdent = m_Engine.getBigListMgr().findNetIdent( srcOnlineId );
    if( !srcNetIdent )
    {
        if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s: unknown src ident %s from %s %s", __func__,
               srcOnlineId.toOnlineIdString().c_str(),
               netIdentIn->getOnlineName(), sktBase->describeSktType().c_str() );
        // TODO should this be a hack offense?
        return;
    }

    if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s: offer from %s %s", __func__,
           srcNetIdent->getOnlineName(), sktBase->describeSktType().c_str() );
	
	PktPluginOfferReq* pktReq = (PktPluginOfferReq *)pktHdr;

	OfferBaseInfo offerInfo;
	if( !offerInfo.extractFromBlob( pktReq->getBlobEntry() ) )
	{
		LogMsg( LOG_ERROR, "PluginSessionMgr::%s: could not extract offer from %s", __func__, srcNetIdent->getOnlineName() );
		return;
	}

	// since we are recieving offer then we are client
	offerInfo.setIsRemoteInitiated( true );

	if( IsPluginSingleSession( offerInfo.getPluginType() ) && m_Plugin.getIsPluginInSession() )
	{
		offerInfo.setOfferResponse( eOfferResponseBusy );

		PktPluginOfferReply pktReply;

		pktReply.setLclSessionId( pktReq->getRmtSessionId() );
		pktReply.setRmtSessionId( pktReq->getLclSessionId() );
		pktReply.setPluginType( getPluginType() );
		offerInfo.addToBlob( pktReply.getBlobEntry() );
		pktReply.calcPktLen();
		if( !m_Plugin.txPacket( srcOnlineId, sktBase, &pktReply ) )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s: failed send to %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
			return;
		}

		// for call missed
		IToGui::getIToGui().toGuiRxedPluginOffer(srcOnlineId, offerInfo );
		return;
	}

	VxGUID lclSessionId = offerInfo.getOfferId();

	PluginSessionBase* pluginSession = nullptr;
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	if( lclSessionId.isValid() && (false == isPluginSingleSession()) )
	{
		pluginSession = findOrCreateP2PSessionWithSessionId( lclSessionId, sktBase, srcOnlineId, true);
	}
	else
	{
		pluginSession = findOrCreateP2PSessionWithOnlineId( srcOnlineId, sktBase, true, lclSessionId );
	}

	pluginSession->setSkt( sktBase );
	pluginSession->setSendToId( srcOnlineId );
	pluginSession->setLclSessionId( lclSessionId );
	pluginSession->setRmtSessionId( pktReq->getLclSessionId() );
	pluginSession->setOfferInfo( offerInfo, false );

	IToGui::getIToGui().toGuiRxedPluginOffer( srcOnlineId, offerInfo );
}

//============================================================================
void PluginSessionMgr::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	PktPluginOfferReply* pktReply = (PktPluginOfferReply*)pktHdr;

	OfferBaseInfo offerInfo;
	if( offerInfo.extractFromBlob( pktReply->getBlobEntry() ) )
	{
		// if we sent the offer then session is already created
		VxGUID lclSessionId = offerInfo.getOfferId();
		IToGui::getIToGui().toGuiRxedOfferReply( srcOnlineId, offerInfo );

		PluginSessionBase* sessionBase = findP2PSessionBySessionId( lclSessionId, true);
		if( sessionBase )
		{
			sessionBase->setOfferInfo( offerInfo, false );
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s lclSession %s rmtSession %s user %s", __func__,
				lclSessionId.toHexString().c_str(),
				pktReply->getLclSessionId().toHexString().c_str(),
				m_Engine.describeUser( srcOnlineId ).c_str() );
			sessionBase->setRmtSessionId( pktReply->getLclSessionId() );
			sessionBase->setOfferResponse( pktReply->getOfferResponse() );
			if( eOfferResponseAccept == sessionBase->getOfferResponse() )
			{
				m_Plugin.onSessionStart( sessionBase, true );
			}
			else
			{
                removeSession( true, srcOnlineId, sessionBase->getRmtSessionId(), sessionBase->getOfferResponse(), false );
			}
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginSessionMgr::%s: could not extract offer from %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
	}
}

//============================================================================
void PluginSessionMgr::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	PktSessionStopReq * pkt = (PktSessionStopReq *)pktHdr;

	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s lclSession %s rmtSession %s user %s", __func__,
		pkt->getLclSessionId().toHexString().c_str(),
        pkt->getRmtSessionId().toHexString().c_str(),
		m_Engine.describeUser( srcOnlineId ).c_str() );

	PluginSessionBase* sessionBase = findP2PSessionBySessionId( pkt->getRmtSessionId(), false );
	if( sessionBase )
	{
		removeSession( false, srcOnlineId, pkt->getRmtSessionId(), eOfferResponseEndSession );
	}
	else
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s failed findP2PSessionBySessionId my lclSession %s user %s", __func__,
			pkt->getLclSessionId().toHexString().c_str(),
            pkt->getRmtSessionId().toHexString().c_str(),
			m_Engine.describeUser( srcOnlineId ).c_str() );
	}
}

//============================================================================
P2PSession* PluginSessionMgr::findP2PSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->isP2PSession() && session->getLclSessionId() == sessionId )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return (P2PSession*)session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
P2PSession* PluginSessionMgr::findP2PSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked )
{
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->isP2PSession() && ( session->getSendToId() == onlineId ) )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();;
			}

			return (P2PSession*)session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}
//============================================================================
P2PSession* PluginSessionMgr::findOrCreateP2PSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked )
{
	if( !sktBase || !sktBase->isConnected() )
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s user %s not connected", __func__, m_Engine.describeUser( onlineId ).c_str() );
		return nullptr;
	}

	P2PSession* session = findP2PSessionBySessionId( sessionId, pluginIsLocked );
	if( !session )
	{
		session = m_Plugin.createP2PSession( sessionId, sktBase, onlineId );
		// initially set remote session id to same as local session id
		// this is so if canceled right away the remote id is set 
		// if offer response is recieve then the remote id will be set from that
		session->setRmtSessionId( sessionId );

		addSession( session->getLclSessionId(), session, pluginIsLocked );
	}
	else
	{
		session->setSkt( sktBase );
	}

	return session;
}

//============================================================================
P2PSession* PluginSessionMgr::findOrCreateP2PSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase, bool pluginIsLocked, VxGUID lclSessionId )
{
	if( !sktBase || !sktBase->isConnected() )
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s user %s not connected", __func__, m_Engine.describeUser( onlineId ).c_str() );
		return nullptr;
	}

	P2PSession* session = findP2PSessionByOnlineId( onlineId, pluginIsLocked );
	if( !session )
	{
        session = m_Plugin.createP2PSession( sktBase, onlineId );
		if( false == lclSessionId.isValid() )
		{
			lclSessionId.initializeWithNewVxGUID();
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s invalid lclSession .. setting to lclSession %s user %s", __func__, 
				lclSessionId.toHexString().c_str(),
				m_Engine.describeUser( onlineId ).c_str() );
			session->setLclSessionId( lclSessionId );
			addSession( onlineId, session, pluginIsLocked );
		}
		else
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s existing lclSession %s user %s", __func__,
				lclSessionId.toHexString().c_str(),
				m_Engine.describeUser( onlineId ).c_str() );
			session->setLclSessionId( lclSessionId );
			addSession( onlineId, session, pluginIsLocked );
		}
	}
	else
	{
		session->setSkt( sktBase );
	}

	return session;
}

//============================================================================
TxSession * PluginSessionMgr::findTxSessionBySessionId( bool pluginIsLocked, VxGUID& sessionId )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->isTxSession() )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return (TxSession *)session;
		}
	}

	if( false == pluginIsLocked )
	{				
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
TxSession * PluginSessionMgr::findTxSessionByOnlineId( bool pluginIsLocked, VxGUID& onlineId )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->isTxSession() && ( session->getSendToId() == onlineId ) )
		{
			if( false == pluginIsLocked )
			{			
				m_Plugin.unlockPlugin();
			}

			return (TxSession *)session;
		}
	}

	if( false == pluginIsLocked )
	{				
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
TxSession * PluginSessionMgr::findOrCreateTxSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked )
{
	TxSession * session = findTxSessionBySessionId( pluginIsLocked, sessionId );
	if( nullptr == session )
	{
        session = m_Plugin.createTxSession( sessionId, sktBase, onlineId );
		addSession( sessionId, session, pluginIsLocked );
	}
	else
	{
		session->setSkt( sktBase );
	}

	return session;
}

//============================================================================
TxSession * PluginSessionMgr::findOrCreateTxSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase, bool pluginIsLocked, VxGUID lclSessionId )
{
	TxSession * session = findTxSessionByOnlineId( pluginIsLocked, onlineId );
	if( !session )
	{
        session = m_Plugin.createTxSession( lclSessionId, sktBase, onlineId );
		addSession( lclSessionId, session, pluginIsLocked );
	}
	else
	{
		session->setSkt( sktBase );
	}

	return session;
}

//============================================================================
int PluginSessionMgr::getTxSessionCount( bool pluginIsLocked )
{
	int txSessionCnt = 0;
	if( !pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto session : m_aoSessions )
	{
		if( session->isTxSession() )
		{
			txSessionCnt++;
		}
	}

	if( !pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return txSessionCnt;
}

//============================================================================
RxSession * PluginSessionMgr::findRxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked  )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = ( *iter );
		if( session->isRxSession() && session->getLclSessionId() == sessionId )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return (RxSession *)session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
RxSession * PluginSessionMgr::findRxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked  )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);
		if( session->isRxSession() && ( session->getSendToId() == onlineId ) )
		{
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			return (RxSession *)session;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

	return nullptr;
}

//============================================================================
RxSession * PluginSessionMgr::findOrCreateRxSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked )
{
	RxSession * session = findRxSessionBySessionId( sessionId, pluginIsLocked );
	if( nullptr == session )
	{
        session = m_Plugin.createRxSession( sessionId, sktBase, onlineId );
		addSession( sessionId, session, pluginIsLocked );
	}

	return session;
}

//============================================================================
RxSession * PluginSessionMgr::findOrCreateRxSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase, bool pluginIsLocked, VxGUID lclSessionId )
{
	RxSession * session = findRxSessionByOnlineId( onlineId, pluginIsLocked );
	if( nullptr == session )
	{
        session = m_Plugin.createRxSession( sktBase, onlineId );
		if( false == lclSessionId.isValid() )
		{
			addSession( session->getLclSessionId(), session, pluginIsLocked );
		}
		else
		{
			session->setLclSessionId( lclSessionId );
			addSession( lclSessionId, session, pluginIsLocked );
		}
	}

	return session;
}


//============================================================================ 
void PluginSessionMgr::addSession( VxGUID& sessionId, PluginSessionBase* session, bool pluginIsLocked )
{
	if( false == session->getLclSessionId().isValid() )
	{
		if( sessionId.isValid() )
		{
			session->setLclSessionId( sessionId );
		}
		else
		{
			session->getLclSessionId().initializeWithNewVxGUID();
		}
	}

	if( sessionId != session->getLclSessionId() )
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_WARNING, "WARNING SESSION IDS DONT MATCH PluginSessionMgr::addSession %s session id %s connect info %s", 
				m_Engine.describeUser( session->getSendToId() ).c_str(), sessionId.toHexString().c_str(), session->getSkt()->describeSktType().c_str());
	}

	if( false == sessionId.isValid() )
	{
		sessionId = session->getLclSessionId();
	}

	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::addSession %s session id %s connect info %s", 
			m_Engine.describeUser( session->getSendToId() ).c_str(), sessionId.toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
	if( pluginIsLocked )
	{
		m_aoSessions.emplace_back( session );
	}
	else
	{
		PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
		m_aoSessions.emplace_back( session );
	}
}

//============================================================================ 
void PluginSessionMgr::endPluginSession( PluginSessionBase* session, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		if( session == (*iter) )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::endPluginSession %s session id %s connect info %s", 
					m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
			m_aoSessions.erase(iter);
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
void PluginSessionMgr::endPluginSession( VxGUID& sessionId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = ( *iter );
		if( sessionId == session->getLclSessionId() )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::endPluginSession %s session id %s connect info %s",
				m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
			m_aoSessions.erase( iter );
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
void PluginSessionMgr::removeTxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = ( *iter );
		if( session->isTxSession() && sessionId == session->getLclSessionId() )
		{
			m_aoSessions.erase(iter);	
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
void PluginSessionMgr::removeTxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);
		if( session->isTxSession() && ( session->getSendToId() == onlineId ) )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::removeTxSessionByOnlineId %s session id %s connect info %s", 
					m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );		
			m_aoSessions.erase(iter);	
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
void PluginSessionMgr::removeRxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = ( *iter );
		if( session->isRxSession() && ( session->getLclSessionId() == sessionId ) )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::removeRxSessionBySessionId %s session id %s connect info %s", 
					m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
			m_aoSessions.erase(iter);	
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
void PluginSessionMgr::removeRxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked )
{
	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);
		if( session->isRxSession() && ( session->getSendToId() == onlineId ) )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::removeRxSessionByOnlineId %s session id %s connect info %s", 
				m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
			m_aoSessions.erase( iter );
			delete session;
			break;
		}
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}
}

//============================================================================ 
// returns true if found and removed session
bool PluginSessionMgr::removeSessionBySessionId( bool pluginIsLocked, VxGUID& lclSessionId, EOfferResponse offerResponse )
{
	bool wasRemoved = false;
	if( lclSessionId.isValid() )
	{
		// doEndAndEraseSession erases from m_aoSessions so keep ending until all are gone
		bool endedSession = true;
		while( endedSession )
		{
			endedSession = false;
			if( false == pluginIsLocked )
			{
				m_Plugin.lockPlugin();
			}

			PluginSessionBase* session = nullptr;
			for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
			{
				session = ( *iter );
				if( session->getLclSessionId() == lclSessionId )
				{
					
					wasRemoved = true;
					endedSession = true;
					break;
				}
			}

			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			if( endedSession && session )
			{
				doEndAndEraseSession( session, offerResponse, pluginIsLocked );
			}
		}
	}
	
	if( !wasRemoved )
	{
		if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_ERROR, "PluginSessionMgr::%s failed find lclSession %s", __func__,
			lclSessionId.toHexString().c_str() );
	}

	return wasRemoved;
}

//============================================================================
bool PluginSessionMgr::removeSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID& sessionId, EOfferResponse offerResponse, bool fromGui )
{
	if( removeSessionBySessionId( pluginIsLocked, sessionId, offerResponse ) )
	{
		return true;
	}

	if( false == pluginIsLocked )
	{
		m_Plugin.lockPlugin();
	}

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);
		if( session->getSendToId() == onlineId )
		{
			if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s user %s session id %s connect info %s", __func__,
				m_Engine.describeUser( session->getSendToId() ).c_str(), session->getLclSessionId().toHexString().c_str(), session->getSkt()->describeSktType().c_str() );
			if( false == pluginIsLocked )
			{
				m_Plugin.unlockPlugin();
			}

			doEndAndEraseSession( session, offerResponse, pluginIsLocked, !fromGui );
			return true;
		}
	}

	if(  false == pluginIsLocked )
	{
		m_Plugin.unlockPlugin();
	}

    if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginSessionMgr::%s failed remov session id %s user %s",
        __func__, sessionId.toHexString().c_str(), m_Engine.describeUser( onlineId ).c_str() );
	return false;
}

//============================================================================ 
void PluginSessionMgr::removeAllSessions( bool testSessionsOnly )
{
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	for( auto iter = m_aoSessions.begin(); iter != m_aoSessions.end(); ++iter )
	{
		PluginSessionBase* session = (*iter);

		if( ( false == testSessionsOnly ) || ( ( true == testSessionsOnly ) && session->isInTest() ) )
		{
			iter = m_aoSessions.erase(iter);
			delete session;
		}
		else
		{
			++iter;
		}
	}
}
