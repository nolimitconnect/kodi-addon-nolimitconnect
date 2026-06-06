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

class PluginBaseMultimedia : public PluginBase
{
public:
	PluginBaseMultimedia( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginBaseMultimedia() = default;

    EHostType                   getHostType( void ) override        { return m_HostType; }
    HostedId&                   getHostedId( void )                 { return m_HostedId; }

    void                        fromGuiUserLoggedOn( void ) override;

	bool                        fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;
    bool                        fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

    bool                        fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    bool                        fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    void                        fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

	bool                        fromGuiSendAsset( AssetBaseInfo& assetInfo ) override;
    bool                        fromGuiMultiSessionAction( VxGUID& onlineId, EMSessionAction mSessionAction, int pos0to100000, VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

    bool                        fromGuiTodGameActionSend( VxGUID& onlineId, ETodGameAction todGameAction ) override;

    void                        broadcastToClients( VxPktHdr* pktHdr, VxGUID& requesterOnlineId, std::shared_ptr<VxSktBase>& sktBaseRequester, bool includeRequester = true ) override;
    void                        broadcastToClients( VxPktHdr* pktHdr, VxGUID& excludedOnlineId ) override;

protected:

	void                        onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktMultiSessionReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktMultiSessionReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void                        onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void                        callbackOpusPkt( PktVoiceReq * pktOpusAudio ) override;
    void                        callbackAudioOutSpaceAvail( int freeSpaceLenBytes ) override;

	//virtual void				callbackVideoJpg( VxGUID& vidFeedId, uint8_t * jpgData, uint32_t jpgDataLen, int motion0to100000 );
    void                        callbackVideoPktPic( VxGUID& onlineId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum ) override;
    void                        callbackVideoPktPicChunk( VxGUID& onlineId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum ) override;

    void                        onSessionStart( PluginSessionBase* session, bool pluginIsLocked ) override;
    void                        onSessionEnded( PluginSessionBase* session, bool pluginIsLocked, EOfferResponse offerResponse ) override;

protected:
    void                        onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;
    void                        onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;

    void					    onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override;

    void                        replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override;
    void                        onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override;

    EConnectReason              getHostAnnounceConnectReason( void );
    EConnectReason              getHostJoinConnectReason( void );
    EConnectReason              getHostSearchConnectReason( void );

	//=== vars ===
	PluginSessionMgr			m_PluginSessionMgr;
	VoiceFeedMgr				m_VoiceFeedMgr;
	VideoFeedMgr				m_VideoFeedMgr;

    AssetXferMgr                m_AssetXferMgr;
    EHostType                   m_HostType{ eHostTypeUnknown };
    HostedId                    m_HostedId;
};



