//============================================================================
// Copyright (C) 2020 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseMultimedia.h"
#include "P2PSession.h"
#include "PluginMgr.h"

#include <CoreLib/VxDebug.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>

#include <PktLib/PktsVideoFeed.h>
#include <PktLib/PktsMultiSession.h>
#include <PktLib/PktsTodGame.h>

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>
#include <AssetMgr/AssetMgr.h>
#include <AssetMgr/AssetXferMgr.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
PluginBaseMultimedia::PluginBaseMultimedia( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_PluginSessionMgr( engine, *this, pluginMgr )
, m_VoiceFeedMgr( engine, *this, m_PluginSessionMgr )
, m_VideoFeedMgr( engine, *this, m_PluginSessionMgr )
, m_AssetXferMgr( engine, engine.getAssetMgr(), *this )
, m_HostType( PluginTypeToHostType( pluginType ) )
, m_HostedId( myIdent->getMyOnlineId(), m_HostType )
{
}

//============================================================================
void PluginBaseMultimedia::fromGuiUserLoggedOn( void )
{
	m_AssetXferMgr.fromGuiUserLoggedOn();
}

//============================================================================
bool PluginBaseMultimedia::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	bool result = false;
	P2PSession* poSession = nullptr;
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMakePluginOffer autoLock start\n" );
	PluginBase::AutoPluginLock pluginMutexLock( this );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiTodGameActionSend autoLock done\n" );

	poSession = (P2PSession*)m_PluginSessionMgr.findP2PSessionByOnlineId( onlineId, true );
	if( poSession )
	{
		LogMsg( LOG_ERROR, "PluginBaseMultimedia already in session");
		result = true;
	}
	else
	{
		offerInfo.getOfferId().assureIsValidGUID();
		result = m_PluginSessionMgr.fromGuiMakePluginOffer( true, onlineId, offerInfo );
	}

	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMakePluginOffer autoLock destroy\n" );
	return result;
}

//============================================================================
bool PluginBaseMultimedia::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiOfferReply start\n" );
	bool result = m_PluginSessionMgr.fromGuiOfferReply( false, onlineId, offerInfo );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiOfferReply done\n" );
	return result;
}

//============================================================================
void PluginBaseMultimedia::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktSessionStopReq start" );
	m_PluginSessionMgr.onPktSessionStopReq( sktBase, pktHdr, netIdent );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktSessionStopReq done" );
}

//============================================================================
bool PluginBaseMultimedia::fromGuiSendAsset( AssetBaseInfo& assetInfo )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiSendAsset start" );
	return m_AssetXferMgr.fromGuiSendAssetBase( assetInfo );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiSendAsset done" );
}

//============================================================================
bool PluginBaseMultimedia::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_PluginSessionMgr.fromGuiIsPluginInSession( false, onlineId, lclSessionId );
}

//============================================================================
//! called to start service or session with remote friend
bool PluginBaseMultimedia::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
    return true;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginBaseMultimedia::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiStopPluginSession start\n" );
	m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
	m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
	m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), getEngine().getMyOnlineId() );

	m_PluginSessionMgr.fromGuiStopPluginSession( false, onlineId, lclSessionId );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiStopPluginSession done\n" );
}

//============================================================================
bool PluginBaseMultimedia::fromGuiMultiSessionAction( VxGUID& onlineId, EMSessionAction mSessionAction, int pos0to100000, VxGUID lclSessionId )
{
	LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMultiSessionAction action %d \n", mSessionAction );
	bool sendSucces = false;

	if( eMSessionActionAccept == mSessionAction )
	{
		switch( pos0to100000 )
		{
		case eMSessionTypePhone:
			m_VoiceFeedMgr.enableAudioCapture( true, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( true, onlineId );
			break;

		case eMSessionTypeVidChat:
		case eMSessionTypeTruthOrDare:
			m_VoiceFeedMgr.enableAudioCapture( true, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( true, onlineId );
			m_VideoFeedMgr.fromGuiStartPluginSession( true, getMediaModule(), getEngine().getMyOnlineId() );
			break;
		}
	}
	else if(  eMSessionActionHangup == mSessionAction )
	{
		switch( pos0to100000 )
		{
		case eMSessionTypePhone:
			m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
			break;

		case eMSessionTypeVidChat:
		case eMSessionTypeTruthOrDare:
			m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
			m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), onlineId );
			m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), getEngine().getMyOnlineId() );
			break;
		}
	}

	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMultiSessionAction autoLock start" );
	#endif //DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( this );
	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMultiSessionAction autoLock done" );
	#endif //DEBUG_AUTOPLUGIN_LOCK
	PluginSessionBase* poSession = m_PluginSessionMgr.findPluginSessionByOnlineId( onlineId, true );
	if( 0 == poSession )
	{
		LogMsg( LOG_ERROR, "ERROR PluginBaseMultimedia::fromGuiMultiSessionAction missing plugin session" );
	}
	else
	{
		PktMultiSessionReq pktReq;
		pktReq.setMSessionAction( (uint32_t)mSessionAction );
		pktReq.setMSessionParam( (uint32_t)pos0to100000 );
		sendSucces = m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, poSession->getSkt(), &pktReq );
	}

	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiMultiSessionAction autoLock destroy" );
	#endif //DEBUG_AUTOPLUGIN_LOCK
	return sendSucces;
}

//============================================================================
bool PluginBaseMultimedia::fromGuiTodGameActionSend( VxGUID& onlineId, ETodGameAction todGameAction )
{
	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiSetGameActionVar autoLock start" );
	#endif //DEBUG_AUTOPLUGIN_LOCK
	bool sendSucces = false;
	PluginBase::AutoPluginLock pluginMutexLock( this );
	PluginSessionBase* poSession = m_PluginSessionMgr.findPluginSessionByOnlineId( onlineId, true );
	if( poSession )
	{
		PktTodGameAction pktGameAction;
		pktGameAction.setAction( todGameAction, 0 );
		sendSucces = m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, poSession->getSkt(), &pktGameAction );
	}

	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "PluginBaseMultimedia::fromGuiSetGameActionVar autoLock done" );
	#endif //DEBUG_AUTOPLUGIN_LOCK
	return sendSucces;
}

//============================================================================
void PluginBaseMultimedia::callbackOpusPkt( PktVoiceReq * pktOpusAudio )
{
	m_VoiceFeedMgr.callbackOpusPkt( pktOpusAudio );
}

//============================================================================
void PluginBaseMultimedia::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	m_VoiceFeedMgr.callbackAudioOutSpaceAvail( freeSpaceLenBytes );
}

//============================================================================
void PluginBaseMultimedia::callbackVideoPktPic( VxGUID& onlineId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum )
{
	m_VideoFeedMgr.callbackVideoPktPic( onlineId, pktVid, pktsInSequence, thisPktNum );
}

//============================================================================
void PluginBaseMultimedia::callbackVideoPktPicChunk( VxGUID& onlineId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum ) 
{
	m_VideoFeedMgr.callbackVideoPktPicChunk( onlineId, pktVid, pktsInSequence, thisPktNum );
}

//============================================================================
void PluginBaseMultimedia::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktPluginOfferReq start\n" );
	m_PluginSessionMgr.onPktPluginOfferReq( sktBase, pktHdr, netIdent );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktPluginOfferReq done\n" );
}

//============================================================================
void PluginBaseMultimedia::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktPluginOfferReply start\n" );
	m_PluginSessionMgr.onPktPluginOfferReply( sktBase, pktHdr, netIdent );
	//LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktPluginOfferReply done\n" );
}

//============================================================================
void PluginBaseMultimedia::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedStatus( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPic( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPicChunk( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VideoFeedMgr.onPktVideoFeedPicAck( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_VoiceFeedMgr.onPktVoiceReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseMultimedia::onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendReq start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetSendReq( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendReq done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendReply start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetSendReply( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendReply done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetChunkReq start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetChunkReq( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetChunkReq done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetChunkReply start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetChunkReply( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetChunkReply done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendCompleteReq start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetSendCompleteReq( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendCompleteReq done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendCompleteReply start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetSendCompleteReply( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetSendCompleteReply done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetXferErr start" );
#endif // DEBUG_SEND_ASSET
	m_AssetXferMgr.onPktAssetXferErr( sktBase, pktHdr, netIdent );
#ifdef DEBUG_SEND_ASSET
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktAssetXferErr done" );
#endif // DEBUG_SEND_ASSET
}

//============================================================================
void PluginBaseMultimedia::onPktMultiSessionReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktMultiSessionReq * pktReq			= (PktMultiSessionReq *)pktHdr;
	EMSessionAction eMSessionAction		= ( EMSessionAction )pktReq->getMSessionAction();
	EMSessionType eMSessionType			= ( EMSessionType )pktReq->getMSessionParam(); 
	VxGUID onlineId = pktReq->getSrcOnlineId();

	if( eMSessionActionAccept == eMSessionAction )
	{
		switch( eMSessionType )
		{
		case eMSessionTypePhone:
			m_VoiceFeedMgr.enableAudioCapture( true, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( true, onlineId );
			break;

		case eMSessionTypeVidChat:
		case eMSessionTypeTruthOrDare:
			m_VoiceFeedMgr.enableAudioCapture( true, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( true, onlineId );
			m_VideoFeedMgr.fromGuiStartPluginSession( true, getMediaModule(), getEngine().getMyOnlineId() );
			break;
		default:
			break;
		}
	}
	else if(  eMSessionActionHangup == eMSessionAction )
	{
		switch( eMSessionType )
		{
		case eMSessionTypePhone:
			m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
			break;

		case eMSessionTypeVidChat:
		case eMSessionTypeTruthOrDare:
			m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
			m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
			m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), onlineId );
			break;
		default:
			break;
		}
	}

	IToGui::getIToGui().toGuiMultiSessionAction( eMSessionAction, onlineId, pktReq->getMSessionParam() );

	PluginBase::AutoPluginLock pluginMutexLock( this );

	P2PSession* poSession = m_PluginSessionMgr.findOrCreateP2PSessionWithOnlineId( onlineId, sktBase, true );
	poSession->setSkt( sktBase );
}

//============================================================================
void PluginBaseMultimedia::onPktMultiSessionReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktMultiSessionReply start" );
	PktMultiSessionReply * pktReply = (PktMultiSessionReply *)pktHdr;

	IToGui::getIToGui().toGuiMultiSessionAction( ( EMSessionAction )pktReply->getMSessionAction(), netIdent->getMyOnlineId(), pktReply->getMSessionParam() );
	LogMsg( LOG_INFO, "PluginBaseMultimedia::onPktMultiSessionReply done" );
}

//============================================================================
void PluginBaseMultimedia::onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//PktTodGameStats * poPkt = (PktTodGameStats *)pktHdr;
	//for( int i = 0; i < eMaxTodGameStatId; ++i )
	//{
	//	IToGui::getIToGui().toGuiSetGameValueVar( m_ePluginType, netIdent->getMyOnlineId(), i, poPkt->getVar((ETodGameVarId)i) );
	//}

	//IToGui::getIToGui().toGuiSetGameActionVar( m_ePluginType, netIdent->getMyOnlineId(), eTodGameActionSendStats, 1 );
}

//============================================================================
void PluginBaseMultimedia::onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktTodGameAction * poPkt = (PktTodGameAction *)pktHdr;
	VxGUID srcOnlineId = poPkt->getSrcOnlineId();
	IToGui::getIToGui().toGuiTodGameAction( m_ePluginType, srcOnlineId, poPkt->getActionVarId() );
}

//============================================================================
void PluginBaseMultimedia::onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//PktTodGameValue * poPkt = (PktTodGameValue *)pktHdr;
	//IToGui::getIToGui().toGuiSetGameValueVar( m_ePluginType, netIdent->getMyOnlineId(), poPkt->getValueVarId(), poPkt->getValueVar() );
}

//============================================================================
void PluginBaseMultimedia::onSessionStart( PluginSessionBase* session, bool pluginIsLocked )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginBaseMultimedia::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	PluginBase::onSessionStart( session, pluginIsLocked ); // mark user session time so contact list is sorted with latest used on top

	//NOTE: for chat session the video and audio is started/stopped through MSession commands
	//m_VoiceFeedMgr.fromGuiStartPluginSession( session->getSendToId() );
	// in order to get my video packets to send out the ident has to by myself
	//m_VideoFeedMgr.fromGuiStartPluginSession( getEngine().getMyOnlineId() );
}

//============================================================================
void PluginBaseMultimedia::onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse )
{
	if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginBaseMultimedia::%s for %s", __func__,
		m_Engine.describeUser( session->getSendToId() ).c_str() );
	m_VoiceFeedMgr.enableAudioCapture( false, session->getSendToId() );
	m_VoiceFeedMgr.enableAudioReceive( false, session->getSendToId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), session->getSendToId() );
	m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), getEngine().getMyOnlineId() );
}

//============================================================================
void PluginBaseMultimedia::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_AssetXferMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginBaseMultimedia::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_AssetXferMgr.onConnectionLost( sktBase );
	m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginBaseMultimedia::onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_AssetXferMgr.onContactWentOnline( netIdent, sktBase );
}

//============================================================================
void PluginBaseMultimedia::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	if( !netIdent->isMyself() )
	{
		VxGUID onlineId = netIdent->getMyOnlineId();
        LogModule( eLogUsers, LOG_INFO, "PluginBaseMultimedia::%s %s start", __func__, netIdent->getOnlineName() );
		m_VoiceFeedMgr.enableAudioCapture( false, onlineId );
		m_VoiceFeedMgr.enableAudioReceive( false, onlineId );
		m_VideoFeedMgr.fromGuiStopPluginSession( true, getMediaModule(), netIdent->getMyOnlineId() );
		m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
        LogModule( eLogUsers, LOG_INFO, "PluginBaseMultimedia::%s %s done", __func__, netIdent->getOnlineName() );
	}
}

//============================================================================
void PluginBaseMultimedia::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
}

//============================================================================
void PluginBaseMultimedia::broadcastToClients( VxPktHdr* pktHdr, VxGUID& requesterOnlineId, std::shared_ptr<VxSktBase>& sktBaseRequester, bool includeRequester )
{
    if( pktHdr && pktHdr->isValidPktPrefix() )
    {
        bool sentToRequestor{ false };
        VxGUID requestorSktConnectionId;
        if( sktBaseRequester )
        {
            requestorSktConnectionId = sktBaseRequester->getSocketId();
        }

        std::set<ConnectId> connectIdSet;
        std::set<ConnectId> relayedIdSet;
        if( m_Engine.getConnectIdListMgr().getConnections( getHostedId(), connectIdSet, relayedIdSet ) )
        {
            for( auto& connectId : connectIdSet )
            {
                VxGUID memberOnlineId = const_cast<ConnectId&>(connectId).getUserOnlineId();
                VxGUID socketId = const_cast<ConnectId&>(connectId).getSocketId();
                GroupieId groupieId = const_cast<ConnectId&>(connectId).getGroupieId();

                m_Engine.getPeerMgr().lockSktBaseMgr();

                std::shared_ptr<VxSktBase> sktBase = m_Engine.getPeerMgr().findSktBase( socketId, true );
                if( sktBase && sktBase->isConnected() )
                {
                    VxGUID peerOnlineId = sktBase->getPeerOnlineId();
                    if( sktBase->getPeerOnlineId() != memberOnlineId )
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer %s id %s does NOT match user id %s for pkt %s",
                                sktBase->getPeerOnlineName().c_str(), peerOnlineId.toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str(),
                                pktHdr->describePktHdr().c_str() );
						#if defined(DEBUG_SKT_MGR_LOCK)
							LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s unlockSktBaseMgr", __func__ );
						#endif // defined(DEBUG_SKT_MGR_LOCK)
                        m_Engine.getPeerMgr().unlockSktBaseMgr();
                        continue;
                    }

                    if( peerOnlineId == requesterOnlineId )
                    {
                        if( !includeRequester )
                        {
                            LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients excluding requestor %s id %s for pkt %s",
                                    sktBase->getPeerOnlineName().c_str(), sktBase->getPeerOnlineId().toOnlineIdString().c_str(), 
                                    pktHdr->describePktHdr().c_str() );
							#if defined(DEBUG_SKT_MGR_LOCK)
								LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s unlockSktBaseMgr", __func__ );
							#endif // defined(DEBUG_SKT_MGR_LOCK)
                            m_Engine.getPeerMgr().unlockSktBaseMgr();
                            continue;
                        }

                        sentToRequestor = true;
                    }


                    LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseService::broadcastToClients pkt %s to %s peer %s", pktHdr->describePktHdr().c_str(),
                               m_Engine.describeGroupieId(groupieId).c_str(), sktBase->getPeerPktAnn().describeUser().c_str() );
					#if defined(DEBUG_SKT_MGR_LOCK)
						LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s unlockSktBaseMgr before txPkt", __func__ );
					#endif // defined(DEBUG_SKT_MGR_LOCK)
					m_Engine.getPeerMgr().unlockSktBaseMgr();
                    if(  txPacket( memberOnlineId, sktBase, pktHdr, getClientPluginType() ) )
                    {
                        // should we log fail to send ?
                    }

					continue;
                }

				#if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s unlockSktBaseMgr", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().unlockSktBaseMgr();
            }
        }

        if( !sentToRequestor && includeRequester && sktBaseRequester && requesterOnlineId.isValid() )
        {
            // allways send to requester even if not still joined
            txPacket( requesterOnlineId, sktBaseRequester, pktHdr, getClientPluginType() );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseService::broadcastToHostClients invalid pkt %s host %s", pktHdr->describePktHdr().c_str(), DescribeHostType( getHostType() ) );
    }
}

//============================================================================
void PluginBaseMultimedia::broadcastToClients( VxPktHdr* pktHdr, VxGUID& excludedOnlineId )
{
    if( pktHdr && pktHdr->isValidPktPrefix() )
    {
        std::set<ConnectId> connectIdSet;
        std::set<ConnectId> relayedIdSet;
        if( m_Engine.getConnectIdListMgr().getConnections( getHostedId(), connectIdSet, relayedIdSet ) )
        {
            for( auto& connectId : connectIdSet )
            {
                VxGUID memberOnlineId = const_cast<ConnectId&>(connectId).getUserOnlineId();
                VxGUID socketId = const_cast<ConnectId&>(connectId).getSocketId();
                GroupieId groupieId = const_cast<ConnectId&>(connectId).getGroupieId();
				#if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s lockSktBaseMgr", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().lockSktBaseMgr();
				#if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s lockSktBaseMgr locked", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                std::shared_ptr<VxSktBase> sktBase = m_Engine.getPeerMgr().findSktBase( socketId, true );
				#if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseMultimedia::%s unlockSktBaseMgr", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().unlockSktBaseMgr();

                if( sktBase && sktBase->isConnected() )
                {
                    if( sktBase->getPeerOnlineId() != memberOnlineId )
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer id %s does NOT match user id %s",
                                sktBase->getPeerOnlineId().toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str() );
                    }
                    else
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer id %s does match user id %s",
                                sktBase->getPeerOnlineId().toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str() );
                    }

                    bool isExcludeId = excludedOnlineId == sktBase->getPeerOnlineId();
                    if( isExcludeId )
                    {
                        continue;
                    }

                    LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseService::broadcastToClients pkt %s to %s peer %s", pktHdr->describePktHdr().c_str(),
                               m_Engine.describeGroupieId(groupieId).c_str(), sktBase->getPeerPktAnn().describeUser().c_str() );
                   
                    if( !txPacket( memberOnlineId, sktBase, pktHdr, getClientPluginType() ) )
                    {
                        // logging ?
                    }
                }
            }
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseService::broadcastToHostClients invalid pkt %s host %s", pktHdr->describePktHdr().c_str(),  DescribeHostType( getHostType() ) );
    }
}

//============================================================================
EConnectReason PluginBaseMultimedia::getHostAnnounceConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomAnnounce;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupAnnounce;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectAnnounce;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
EConnectReason PluginBaseMultimedia::getHostJoinConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomJoin;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupJoin;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectJoin;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
EConnectReason PluginBaseMultimedia::getHostSearchConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomSearch;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupSearch;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectSearch;
        break;
    default:
        break;
    }

    return connectReason;
}
