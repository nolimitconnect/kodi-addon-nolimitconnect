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
#include "VoiceFeedMgr.h"
#include "VideoFeedMgr.h"

class PluginCamServer : public PluginBase
{
public:
	PluginCamServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginCamServer() = default;

    EMediaModule				getMediaModule( void ) override { return eMediaModuleCamServer; }

    bool                        fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    void                        fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    bool                        fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

	//! user wants to send offer to friend.. return false if cannot connect
	bool                        fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

    void                        fromGuiUpdatePluginPermission( EPluginType pluginType, EFriendState pluginPermission ) override;

	bool						stopCamSession(	VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

	void						sendVidPkt( VxPktHdr* vidPkt, bool requiresAck );

	void						stopAllSessions( EPluginType pluginType );

protected:
    EPluginAccess               canAcceptNewSession( VxNetIdent* netIdent ) override;

    void                        replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override;
    void                        onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override;
    void                        onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;

    void					    onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override;

    void                        onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktSessionStartReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktSessionStartReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktSessionStopReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        callbackOpusPkt( PktVoiceReq * pktOpusAudio ) override;
    void                        callbackAudioOutSpaceAvail( int freeSpaceLenBytes ) override;

    void                        callbackVideoJpg( VxGUID& feedId, std::shared_ptr<CamJpgVideo>& jpgVideo ) override;
    void                        callbackVideoPktPic( VxGUID& feedId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum ) override;
    void                        callbackVideoPktPicChunk( VxGUID& feedId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum ) override;

	// override this by plugin to create inherited RxSession
    RxSession *					createRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId ) override;
	// override this by plugin to create inherited TxSession
    TxSession *					createTxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId ) override;

	bool						requestCamSession(	RxSession *			rxSession,
													bool				bWaitForSuccess = false );
    void						setIsPluginInSession( bool isInSession ) override;

    void                        onNetworkConnectionReady( bool requiresRelay ) override;

	void						stopAllSessions( void );

	void						enableCamServerService( bool enable );

    void                        updateTxSessionCount( void );

	//=== vars ===//
	PluginSessionMgr			m_PluginSessionMgr;					
	VoiceFeedMgr				m_VoiceFeedMgr;
	VideoFeedMgr				m_VideoFeedMgr;

	bool						m_IsCamServiceEnabled{ false };
    bool						m_RequestedVidPkts{ false };
    VxGUID                      m_MediaSessionId;
};



