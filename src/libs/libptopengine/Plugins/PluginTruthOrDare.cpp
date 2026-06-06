//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================


#include "PluginTruthOrDare.h"
#include "PluginMgr.h"

#include "TodGameSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

#include <NetLib/VxSktBase.h>

#include <PktLib/PktsTodGame.h>
#include <PktLib/PktChatReq.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/PktsSession.h>
#include <PktLib/PktsVideoFeed.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//#define DEBUG_AUTOPLUGIN_LOCK 1

//============================================================================
PluginTruthOrDare::PluginTruthOrDare( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_PluginSessionMgr( engine, *this, pluginMgr )
, m_VoiceFeedMgr( engine, *this, m_PluginSessionMgr )
, m_VideoFeedMgr( engine, *this, m_PluginSessionMgr )
{
}

//============================================================================
P2PSession* PluginTruthOrDare::createP2PSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    return new TodGameSession( sktBase, onlineId, getPluginType() );
}

//============================================================================
bool PluginTruthOrDare::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	LogMsg( LOG_VERBOSE, " PluginVideoPhone::fromGuiMakePluginOffer %s", m_Engine.describeUser( onlineId ).c_str() );

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
				LogMsg( LOG_VERBOSE, " PluginVideoPhone::fromGuiMakePluginOffer success" );
				return true;
			}
			else
			{
				LogMsg( LOG_VERBOSE, " PluginVideoPhone::fromGuiMakePluginOffer failed to send pkt" );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, " PluginVideoPhone::fromGuiMakePluginOffer failed to create session" );
		}
	}

	return false;
}

//============================================================================
bool PluginTruthOrDare::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	return m_PluginSessionMgr.fromGuiOfferReply( false, onlineId, offerInfo );
}

//============================================================================
bool PluginTruthOrDare::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
}

//============================================================================
bool PluginTruthOrDare::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	const bool isInSession = m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
	if( !isInSession )
	{
		LogMsg( LOG_WARNING,
				"PluginTruthOrDare::%s no active session yet for %s lclSessionId %s",
				__func__,
				m_Engine.describeUser( onlineId ).c_str(),
				lclSessionId.toHexString().c_str() );
	}

	// Offer/reply packet flow (PktPluginOfferReq/PktPluginOfferReply) is responsible
	// for creating session state; this call should not hard-fail the GUI start path.
	return true;
}

//============================================================================
void PluginTruthOrDare::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	m_PluginSessionMgr.fromGuiStopPluginSession( false, onlineId, lclSessionId );
}

//============================================================================
bool PluginTruthOrDare::fromGuiInstMsg( VxGUID& onlineId, const char* msg )
{
	LogMsg( LOG_VERBOSE, "PluginTruthOrDare::fromGuiInstMsg" );
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
		LogMsg( LOG_ERROR, "PluginTruthOrDare::fromGuiInstMsg session not found" );
		return false;
	}
}

//============================================================================
bool PluginTruthOrDare::fromGuiTodGameActionSend( VxGUID& onlineId, ETodGameAction todGameAction )
{
	PluginBase::AutoPluginLock pluginMutexLock( this );
	PluginSessionBase* poSession = m_PluginSessionMgr.findPluginSessionByOnlineId( onlineId, true );
	if( poSession )
	{
		PktTodGameAction pktGameAction;
		pktGameAction.setAction( todGameAction, 0 );
		return m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, poSession->getSkt(), &pktGameAction );
	}

	return false;
}

//============================================================================
void PluginTruthOrDare::callbackOpusPkt( PktVoiceReq* pktOpusAudio )
{
	m_VoiceFeedMgr.callbackOpusPkt( pktOpusAudio );
}

//============================================================================
void PluginTruthOrDare::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	m_VoiceFeedMgr.callbackAudioOutSpaceAvail( freeSpaceLenBytes );
}

//============================================================================
void PluginTruthOrDare::callbackVideoPktPic( VxGUID& onlineId, PktVideoFeedPic* pktVid, int pktsInSequence, int thisPktNum )
{
	m_VideoFeedMgr.callbackVideoPktPic( onlineId, pktVid, pktsInSequence, thisPktNum );
}

//============================================================================
void PluginTruthOrDare::callbackVideoPktPicChunk( VxGUID& onlineId, PktVideoFeedPicChunk* pktVid, int pktsInSequence, int thisPktNum )
{
	m_VideoFeedMgr.callbackVideoPktPicChunk( onlineId, pktVid, pktsInSequence, thisPktNum );
}

//============================================================================
void PluginTruthOrDare::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktChatReq * pkt = (PktChatReq *)pktHdr;
	VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	IToGui::getIToGui().toGuiInstMsg( srcOnlineId, m_ePluginType, (const char*)pkt->getDataPayload() );
}

//============================================================================
void PluginTruthOrDare::onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//PktTodGameStats * poPkt = (PktTodGameStats *)pktHdr;
	//for( int i = 0; i < eMaxTodGameStatId; ++i )
	//{
	//	IToGui::getIToGui().toGuiSetGameValueVar( m_ePluginType, netIdent->getMyOnlineId(), i, poPkt->getVar((ETodGameVarId)i) );
	//}

	//IToGui::getIToGui().toGuiSetGameActionVar( m_ePluginType, netIdent->getMyOnlineId(), eTodGameActionSendStats, 1 );
}

//============================================================================
void PluginTruthOrDare::onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktTodGameAction * poPkt = (PktTodGameAction *)pktHdr;
	VxGUID srcOnlineId = poPkt->getSrcOnlineId();
	IToGui::getIToGui().toGuiTodGameAction( m_ePluginType, srcOnlineId, poPkt->getActionVarId() );
}

//============================================================================
void PluginTruthOrDare::onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//PktTodGameValue * poPkt = (PktTodGameValue *)pktHdr;
	//IToGui::getIToGui().toGuiSetGameValueVar( m_ePluginType, netIdent->getMyOnlineId(), poPkt->getValueVarId(), poPkt->getValueVar() );
}

//============================================================================
void PluginTruthOrDare::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktPluginOfferReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedStatus( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPic( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPicChunk( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPicAck( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_PluginSessionMgr.onPktSessionStopReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginTruthOrDare::onSessionStart( PluginSessionBase* session, bool pluginIsLocked )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginTruthOrDare::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	PluginBase::onSessionStart( session, pluginIsLocked );; // mark user session time so contact list is sorted with latest used on top

	m_VoiceFeedMgr.enableAudioCapture( true, session->getSendToId() );
	m_VoiceFeedMgr.enableAudioReceive( true, session->getSendToId() );
	// in order to get my video packets to send out the ident has to be myself
	m_VideoFeedMgr.fromGuiStartPluginSession( pluginIsLocked, getMediaModule(), getEngine().getMyOnlineId());
}

//============================================================================
void PluginTruthOrDare::onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginTruthOrDare::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	m_VoiceFeedMgr.enableAudioCapture( false, session->getSendToId() );
	m_VoiceFeedMgr.enableAudioReceive( false, session->getSendToId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( pluginIsLocked, getMediaModule(), session->getSendToId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( pluginIsLocked, getMediaModule(), getEngine().getMyOnlineId() );
}

//============================================================================
void PluginTruthOrDare::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginTruthOrDare::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginTruthOrDare::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_VoiceFeedMgr.enableAudioCapture( false, netIdent->getMyOnlineId() );
	m_VoiceFeedMgr.enableAudioReceive( false, netIdent->getMyOnlineId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( false, getMediaModule(), netIdent->getMyOnlineId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( false, getMediaModule(), getEngine().getMyOnlineId() );
	m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
}

//============================================================================
void PluginTruthOrDare::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
}



