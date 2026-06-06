//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginPushToTalk.h"
#include "PluginMgr.h"

#include "P2PSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsPushToTalk.h>
#include <PktLib/PktChatReq.h>

#include <memory.h>
#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

//============================================================================
PluginPushToTalk::PluginPushToTalk( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_PluginSessionMgr( engine, *this, pluginMgr )
, m_PushToTalkFeedMgr( engine, *this, m_PluginSessionMgr )
{
	m_ePluginType = ePluginTypePushToTalk;
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool PluginPushToTalk::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	P2PSession* poSession = nullptr;
	VxGUID& lclSessionId = offerInfo.getOfferId();
	PluginBase::AutoPluginLock pluginMutexLock( this );
	if( lclSessionId.isValid() )
	{
		poSession = (P2PSession*)m_PluginSessionMgr.findP2PSessionBySessionId( lclSessionId, true  );
	}
	else
	{
		poSession = (P2PSession*)m_PluginSessionMgr.findP2PSessionByOnlineId( onlineId, true );
	}

	if( poSession )
	{
		LogMsg( LOG_ERROR, "PluginPushToTalk already in session");
		// assume some error in logic
		m_PluginSessionMgr.removeSessionBySessionId( true, lclSessionId );
	}

	return m_PluginSessionMgr.fromGuiMakePluginOffer( true, onlineId, offerInfo );
}

//============================================================================
bool PluginPushToTalk::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	return m_PluginSessionMgr.fromGuiOfferReply( false, onlineId, offerInfo );
}

//============================================================================
bool PluginPushToTalk::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
}

//============================================================================
//! called to start service or session with remote friend
bool PluginPushToTalk::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID )
{
	//m_PushToTalkFeedMgr.fromGuiStartPluginSession( false, onlineId );
    return true;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginPushToTalk::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID )
{
	//m_PushToTalkFeedMgr.fromGuiStopPluginSession( false, onlineId );
	//m_PluginSessionMgr.fromGuiStopPluginSession( false, onlineId );
}

//============================================================================
bool PluginPushToTalk::fromGuiInstMsg( VxGUID& onlineId, 
										const char*	pMsg )
{
	PluginBase::AutoPluginLock pluginMutexLock( this );
	P2PSession* poSession = m_PluginSessionMgr.findP2PSessionByOnlineId( onlineId, true );
	if( poSession )
	{
		PktChatReq oPkt;
		oPkt.addMsg( pMsg );
		return m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, poSession->getSkt(), &oPkt );
	}

	return false;
}

//============================================================================
bool PluginPushToTalk::fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk )
{
	std::shared_ptr<VxSktBase> sktBase( nullptr );
	bool isMyself = onlineId == m_PluginMgr.getEngine().getMyOnlineId();
	if( isMyself )
	{
		sktBase = m_Engine.getSktLoopback();
	}
	else
	{

        sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
	}

	if( !sktBase || !sktBase->isConnected() )
	{
		return false;
	}

	if( enableTalk )
	{
		if( sktBase )
		{
			return m_PushToTalkFeedMgr.fromGuiPushToTalk( onlineId, enableTalk, sktBase );
		}
		else
		{
			std::shared_ptr<VxSktBase> sktBaseNull( nullptr );
			return m_PushToTalkFeedMgr.fromGuiPushToTalk( onlineId, false, sktBaseNull );
		}
	}
	else
	{
		return m_PushToTalkFeedMgr.fromGuiPushToTalk( onlineId, false, sktBase );
	}

	return false;
}

//============================================================================
void PluginPushToTalk::callbackOpusPkt( PktVoiceReq * pktOpusAudio )
{
	m_PushToTalkFeedMgr.callbackOpusPkt( pktOpusAudio );
}

//============================================================================
void PluginPushToTalk::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	m_PushToTalkFeedMgr.callbackAudioOutSpaceAvail( freeSpaceLenBytes );
}

//============================================================================
void PluginPushToTalk::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktSessionStopReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktPushToTalkReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktPushToTalkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktPushToTalkStart( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktPushToTalkStop( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktVoiceReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PushToTalkFeedMgr.onPktVoiceReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginPushToTalk::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
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
void PluginPushToTalk::onSessionStart( PluginSessionBase* session, bool pluginIsLocked )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginPushToTalk::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	PluginBase::onSessionStart( session, pluginIsLocked ); // mark user session time so contact list is sorted with latest used on top
	//m_PushToTalkFeedMgr.fromGuiStartPluginSession( pluginIsLocked, session->getSendToId() );
}

//============================================================================
void PluginPushToTalk::onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginPushToTalk::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	VxGUID onlineId = session->getSendToId();
	m_PushToTalkFeedMgr.onSessionEnded( onlineId );
}

//============================================================================
void PluginPushToTalk::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginPushToTalk::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginPushToTalk::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_PushToTalkFeedMgr.onContactWentOffline( netIdent, sktBase );
	m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
}

//============================================================================
void PluginPushToTalk::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PushToTalkFeedMgr.onContactOnlineStatusChange( connectId, isOnline );
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
}

