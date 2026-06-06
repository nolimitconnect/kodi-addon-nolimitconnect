#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginSessionMgr.h"
#include "PluginBase.h"
#include "PushToTalkFeedMgr.h"

#include <PktLib/VxCommon.h>

class PluginPushToTalk : public PluginBase
{
public:
	PluginPushToTalk( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginPushToTalk() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModulePushToTalk; }

	bool                        fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;
	bool                        fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

	bool                        fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	bool                        fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	void                        fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

	bool                        fromGuiInstMsg( VxGUID& onlineId, const char* pMsg ) override;
	bool                        fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk ) override;

	void                        replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override;
protected:
	void                        onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	
	void                        onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;
	void                        onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override;

	void					onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override;

	void                        onSessionStart( PluginSessionBase* poSession, bool pluginIsLocked ) override;
	void                        onSessionEnded( PluginSessionBase* poSession, bool pluginIsLocked, EOfferResponse offerResponse ) override;

protected:
	void                        callbackOpusPkt( PktVoiceReq * pktOpusAudio ) override;
	void                        callbackAudioOutSpaceAvail( int freeSpaceLenBytes ) override;

	PluginSessionMgr			m_PluginSessionMgr;
	PushToTalkFeedMgr			m_PushToTalkFeedMgr;
};



