#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
 
#include "PktTypes.h"

#include <memory>

class VxSktBase;
class VxPktHdr;

class PktHandlerBase
{
public:
	PktHandlerBase();
	virtual ~PktHandlerBase() = default;

	virtual void				handlePkt				    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	//=== packet handlers ===//
	virtual void				onPktUnhandled				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktInvalid				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktAnnounce				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAnnList				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktHostUnJoinReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUnJoinReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktScanReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktScanReply			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktPluginOfferReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktPluginOfferReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktChatReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktChatReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktVoiceReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktVoiceReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktVideoFeedReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktVideoFeedStatus		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktVideoFeedPic			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktVideoFeedPicChunk		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktVideoFeedPicAck		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileGetReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFindFileReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFindFileReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileListReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileListReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileInfoReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileChunkReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileGetCompleteReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileXferCancel			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileShareErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktAssetGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktAssetGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetChunkReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktAssetGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktAssetGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktAssetXferErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktMultiSessionReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktMultiSessionReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktSessionStartReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktSessionStartReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktSessionStopReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktSessionStopReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktMyPicSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktMyPicSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerPicChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerPicChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerGetChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerGetChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerPutChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktWebServerPutChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktTodGameStats			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktTodGameAction			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktTodGameValue			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktTcpPunch				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktPingReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktPingReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktImAliveReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktImAliveReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktBlobSendReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobSendReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobChunkReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobChunkReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobSendCompleteReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobSendCompleteReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktBlobXferErr            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktHostJoinReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktHostJoinReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostLeaveReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostLeaveReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktHostSearchReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktHostSearchReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktHostOfferReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktHostOfferReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktFriendOfferReq         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktFriendOfferReply       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktThumbGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktThumbXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

    virtual void				onPktOfferSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
    virtual void				onPktOfferXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktPushToTalkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktPushToTalkReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktPushToTalkStart		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktPushToTalkStop         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktMembershipReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktMembershipReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktHostInfoReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInfoReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktHostInviteAnnReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInviteAnnReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInviteSearchReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInviteSearchReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInviteMoreReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostInviteMoreReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktGroupieInfoReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieInfoReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktGroupieAnnReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieAnnReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieSearchReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieSearchReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieMoreReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktGroupieMoreReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileInfoInfoReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoInfoReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktFileInfoAnnReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoAnnReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoSearchReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoSearchReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoMoreReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFileInfoMoreReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktRelayUserDisconnect	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktHostUserInfoReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserInfoReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserStatusReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserStatusReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktHostUserListReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserListReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserListMoreReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktHostUserListMoreReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktTestConnTestReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktTestConnTestReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktTestConnPingReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktTestConnPingReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktQueryHostUrlReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktQueryHostUrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktStreamCtrlReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktStreamCtrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktRandConnectReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktRandConnectReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	
	virtual void				onPktFriendRequestReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	virtual void				onPktFriendRequestReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	virtual void				onPktAdminAvail				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	//packet type 250 and greater not allowed
	typedef void (PktHandlerBase::*RC_SYS_BASE_PKT_FUNCTION)( std::shared_ptr<VxSktBase>&, VxPktHdr* );  
protected:
	//=== vars ====//
	RC_SYS_BASE_PKT_FUNCTION m_aBaseSysPktFuncTable[ MAX_PKT_TYPE_CNT + 2 ];
};