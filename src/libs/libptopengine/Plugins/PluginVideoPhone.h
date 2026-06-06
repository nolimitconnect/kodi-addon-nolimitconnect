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

#include "PluginBase.h"
#include "PluginSessionMgr.h"
#include "VoiceFeedMgr.h"
#include "VideoFeedMgr.h"

class PluginVideoPhone : public PluginBase
{
public:
	PluginVideoPhone( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginVideoPhone() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModuleVideoPhone; }

	bool                        fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;
	bool                        fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

	bool                        fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	bool                        fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	void                        fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

	bool                        fromGuiInstMsg( VxGUID& onlineId, const char* pMsg ) override;

protected:
	void                        onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void                        onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void                        onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        callbackOpusPkt( PktVoiceReq * pktOpusAudio ) override;
    void                        callbackAudioOutSpaceAvail( int freeSpaceLenBytes ) override;

    void                        callbackVideoPktPic( VxGUID& onlineId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum ) override;
    void                        callbackVideoPktPicChunk( VxGUID& onlineId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum ) override;

    void                        onSessionStart( PluginSessionBase* session, bool pluginIsLocked ) override;
    void                        onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse ) override;

    void                        replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override;
    void                        onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;
    void                        onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override;

	void					    onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override;

	//=== vars ===//
protected:
	PluginSessionMgr			m_PluginSessionMgr;
	VoiceFeedMgr				m_VoiceFeedMgr;
	VideoFeedMgr				m_VideoFeedMgr;
};



