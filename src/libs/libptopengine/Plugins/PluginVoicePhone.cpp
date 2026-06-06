//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginVoicePhone.h"
#include "PluginMgr.h"

#include "P2PSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>

#include <PktLib/PktVoiceReq.h>
#include <PktLib/PktVoiceReply.h>
#include <PktLib/PktChatReq.h>
#include <PktLib/PktsPluginOffer.h>

#include <memory.h>
#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

//============================================================================
PluginVoicePhone::PluginVoicePhone( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_PluginSessionMgr( engine, *this, pluginMgr )
, m_VoiceFeedMgr( engine, *this, m_PluginSessionMgr )
{
	m_ePluginType = ePluginTypeVoicePhone;
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool PluginVoicePhone::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	LogMsg( LOG_VERBOSE, " PluginVoicePhone::fromGuiMakePluginOffer %s", m_Engine.describeUser( onlineId ).c_str() );

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
		P2PSession* p2pSession = m_PluginSessionMgr.findOrCreateP2PSessionWithSessionId( lclSessionId, sktBase, onlineId, true );
		if( p2pSession )
		{
			if( true == m_PluginMgr.pluginApiTxPacket( m_ePluginType,
				onlineId,
				sktBase,
				&pktReq ) )
			{
				LogMsg( LOG_VERBOSE, " PluginCamClient::fromGuiMakePluginOffer success" );
				return true;
			}
			else
			{
				LogMsg( LOG_VERBOSE, " PluginCamClient::fromGuiMakePluginOffer failed to send pkt" );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, " PluginCamClient::fromGuiMakePluginOffer failed to create session" );
		}
	}

	return false;
}

//============================================================================
//! handle reply to offer
bool PluginVoicePhone::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	return m_PluginSessionMgr.fromGuiOfferReply( false, onlineId, offerInfo );
}

//============================================================================
bool PluginVoicePhone::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
}

//============================================================================
//! called to start service or session with remote friend
bool PluginVoicePhone::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	const bool isInSession = m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
	if( !isInSession )
	{
		LogMsg( LOG_WARNING,
				"PluginVoicePhone::%s no active session yet for %s lclSessionId %s",
				__func__,
				m_Engine.describeUser( onlineId ).c_str(),
				lclSessionId.toHexString().c_str() );
	}

	return true;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginVoicePhone::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID )
{
	m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
	m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
	m_PluginSessionMgr.fromGuiStopPluginSession( false, onlineId );
}

//============================================================================
bool PluginVoicePhone::fromGuiInstMsg( VxGUID& onlineId, const char* msg )
{
	LogMsg( LOG_VERBOSE, "PluginVoicePhone::fromGuiInstMsg" );
	PluginBase::AutoPluginLock pluginMutexLock( this );
	P2PSession* poSession = m_PluginSessionMgr.findP2PSessionByOnlineId( onlineId, true );
	if( poSession )
	{
		PktChatReq pkt;
		pkt.addMsg( msg );
		return m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, poSession->getSkt(), &pkt );
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginVideoPhone::fromGuiInstMsg session not found" );
		return false;
	}

	return false;
}

//============================================================================
void PluginVoicePhone::callbackOpusPkt( PktVoiceReq * pktOpusAudio )
{
	m_VoiceFeedMgr.callbackOpusPkt( pktOpusAudio );
}

//============================================================================
void PluginVoicePhone::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	m_VoiceFeedMgr.callbackAudioOutSpaceAvail( freeSpaceLenBytes );
}

//============================================================================
void PluginVoicePhone::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginVoicePhone::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginVoicePhone::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktSessionStopReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginVoicePhone::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginVoicePhone::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginVoicePhone::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktChatReq* poPkt = (PktChatReq *)pktHdr;
	VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	PluginBase::AutoPluginLock pluginMutexLock( this );
	P2PSession* poSession = (P2PSession*)m_PluginSessionMgr.findP2PSessionByOnlineId( srcOnlineId, true );
	if( poSession )
	{
		IToGui::getIToGui().toGuiInstMsg( srcOnlineId, m_ePluginType, (const char*)poPkt->getDataPayload() );
	}
}

//============================================================================
void PluginVoicePhone::onSessionStart( PluginSessionBase* session, bool pluginIsLocked )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginVoicePhone::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	PluginBase::onSessionStart( session, pluginIsLocked ); // mark user session time so contact list is sorted with latest used on top
	m_VoiceFeedMgr.enableAudioCapture( true, session->getSendToId() );
	m_VoiceFeedMgr.enableAudioReceive( true, session->getSendToId() );
}

//============================================================================
void PluginVoicePhone::onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginVoicePhone::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	m_VoiceFeedMgr.enableAudioCapture( false, session->getSendToId() );
	m_VoiceFeedMgr.enableAudioReceive( false, session->getSendToId() );
}

//============================================================================
void PluginVoicePhone::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginVoicePhone::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginVoicePhone::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_VoiceFeedMgr.enableAudioCapture( false, netIdent->getMyOnlineId() );
	m_VoiceFeedMgr.enableAudioReceive( false, netIdent->getMyOnlineId() );
	m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
}

//============================================================================
void PluginVoicePhone::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
}



