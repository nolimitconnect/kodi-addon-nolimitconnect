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
#include "PktPluginHandlerBase.h"
#include <GuiInterface/IDefs.h>

#include <NetLib/VxSktBase.h>
#include <CoreLib/VxDebug.h>

#include <memory.h>

PktPluginHandlerBase::PktPluginHandlerBase()
{
	memset( m_aBaseSysPktFuncTable, 0, sizeof( m_aBaseSysPktFuncTable ) );
	for( int i = 0; i < MAX_PKT_TYPE_CNT; i++ )
	{
		m_aBaseSysPktFuncTable[ i ] = &PktPluginHandlerBase::onPktUnhandled;
	}

	m_aBaseSysPktFuncTable[ 0 ] = &PktPluginHandlerBase::onPktInvalid;

	m_aBaseSysPktFuncTable[ PKT_TYPE_ANNOUNCE ]							= &PktPluginHandlerBase::onPktAnnounce;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ANN_LIST ]							= &PktPluginHandlerBase::onPktAnnList;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_UNJOIN_REQ ]							= &PktPluginHandlerBase::onPktHostUnJoinReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_UNJOIN_REPLY ]						= &PktPluginHandlerBase::onPktHostUnJoinReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_PLUGIN_OFFER_REQ ]					= &PktPluginHandlerBase::onPktPluginOfferReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PLUGIN_OFFER_REPLY ]				= &PktPluginHandlerBase::onPktPluginOfferReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_CHAT_REQ ]							= &PktPluginHandlerBase::onPktChatReq;

	m_aBaseSysPktFuncTable[ PKT_TYPE_VOICE_REQ ]						= &PktPluginHandlerBase::onPktVoiceReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VOICE_REPLY ]						= &PktPluginHandlerBase::onPktVoiceReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_REQ ]					= &PktPluginHandlerBase::onPktVideoFeedReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_STATUS ]				= &PktPluginHandlerBase::onPktVideoFeedStatus;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC ]					= &PktPluginHandlerBase::onPktVideoFeedPic;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC_CHUNK ]				= &PktPluginHandlerBase::onPktVideoFeedPicChunk;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC_ACK ]				= &PktPluginHandlerBase::onPktVideoFeedPicAck;

	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_START_REQ ]				= &PktPluginHandlerBase::onPktSessionStartReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_START_REPLY ]				= &PktPluginHandlerBase::onPktSessionStartReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_STOP_REQ ]					= &PktPluginHandlerBase::onPktSessionStopReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_STOP_REPLY ]				= &PktPluginHandlerBase::onPktSessionStopReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_REQ ]						= &PktPluginHandlerBase::onPktFileGetReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_REPLY ]					= &PktPluginHandlerBase::onPktFileGetReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_REQ ]					= &PktPluginHandlerBase::onPktFileSendReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_REPLY ]					= &PktPluginHandlerBase::onPktFileSendReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_CHUNK_REQ ]					= &PktPluginHandlerBase::onPktFileChunkReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_CHUNK_REPLY ]					= &PktPluginHandlerBase::onPktFileChunkReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_COMPLETE_REQ ]			= &PktPluginHandlerBase::onPktFileGetCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_COMPLETE_REPLY ]			= &PktPluginHandlerBase::onPktFileGetCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_COMPLETE_REQ ]			= &PktPluginHandlerBase::onPktFileSendCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_COMPLETE_REPLY ]			= &PktPluginHandlerBase::onPktFileSendCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_XFER_CANCEL ]					= &PktPluginHandlerBase::onPktFileXferCancel;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_FIND_REQ ]					= &PktPluginHandlerBase::onPktFindFileReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_FIND_REPLY ]					= &PktPluginHandlerBase::onPktFindFileReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_LIST_REQ ]					= &PktPluginHandlerBase::onPktFileListReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_LIST_REPLY ]					= &PktPluginHandlerBase::onPktFileListReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_INFO_REQ ]					= &PktPluginHandlerBase::onPktFileInfoReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SHARE_ERR ]					= &PktPluginHandlerBase::onPktFileShareErr;

    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_REQ ]					= &PktPluginHandlerBase::onPktAssetGetReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_REPLY ]					= &PktPluginHandlerBase::onPktAssetGetReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_REQ ]					= &PktPluginHandlerBase::onPktAssetSendReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_REPLY ]					= &PktPluginHandlerBase::onPktAssetSendReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_CHUNK_REQ ]					= &PktPluginHandlerBase::onPktAssetChunkReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_CHUNK_REPLY ]				= &PktPluginHandlerBase::onPktAssetChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_COMPLETE_REQ ]			= &PktPluginHandlerBase::onPktAssetGetCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_COMPLETE_REPLY ]			= &PktPluginHandlerBase::onPktAssetGetCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_COMPLETE_REQ ]			= &PktPluginHandlerBase::onPktAssetSendCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_COMPLETE_REPLY ]		= &PktPluginHandlerBase::onPktAssetSendCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_XFER_ERR ]					= &PktPluginHandlerBase::onPktAssetXferErr;

	m_aBaseSysPktFuncTable[ PKT_TYPE_MSESSION_REQ ]						= &PktPluginHandlerBase::onPktMultiSessionReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_MSESSION_REPLY ]					= &PktPluginHandlerBase::onPktMultiSessionReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_SCAN_REQ ]						    = &PktPluginHandlerBase::onPktScanReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SCAN_REPLY ]						= &PktPluginHandlerBase::onPktScanReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REQ ]		= &PktPluginHandlerBase::onPktMyPicSendReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REPLY ]		= &PktPluginHandlerBase::onPktMyPicSendReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_PIC_CHUNK_TX ]			= &PktPluginHandlerBase::onPktWebServerPicChunkTx;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_PIC_CHUNK_ACK ]			= &PktPluginHandlerBase::onPktWebServerPicChunkAck;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_GET_CHUNK_TX ]			= &PktPluginHandlerBase::onPktWebServerGetChunkTx;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_GET_CHUNK_ACK ]			= &PktPluginHandlerBase::onPktWebServerGetChunkAck;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_PUT_CHUNK_TX ]			= &PktPluginHandlerBase::onPktWebServerPutChunkTx;
	m_aBaseSysPktFuncTable[ PKT_TYPE_WEB_SERVER_PUT_CHUNK_ACK ]			= &PktPluginHandlerBase::onPktWebServerPutChunkAck;

	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_STATS ]					= &PktPluginHandlerBase::onPktTodGameStats;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_ACTION ]					= &PktPluginHandlerBase::onPktTodGameAction;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_VALUE ]					= &PktPluginHandlerBase::onPktTodGameValue;

	m_aBaseSysPktFuncTable[ PKT_TYPE_TCP_PUNCH ]						= &PktPluginHandlerBase::onPktTcpPunch;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PING_REQ ]							= &PktPluginHandlerBase::onPktPingReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PING_REPLY ]						= &PktPluginHandlerBase::onPktPingReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_IM_ALIVE_REQ ]						= &PktPluginHandlerBase::onPktImAliveReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_IM_ALIVE_REPLY ]					= &PktPluginHandlerBase::onPktImAliveReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_REQ ]                    = &PktPluginHandlerBase::onPktBlobSendReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_REPLY ]                  = &PktPluginHandlerBase::onPktBlobSendReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_CHUNK_REQ ]                   = &PktPluginHandlerBase::onPktBlobChunkReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_CHUNK_REPLY ]                 = &PktPluginHandlerBase::onPktBlobChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_COMPLETE_REQ ]           = &PktPluginHandlerBase::onPktBlobSendCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_COMPLETE_REPLY ]         = &PktPluginHandlerBase::onPktBlobSendCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_XFER_ERR ]                    = &PktPluginHandlerBase::onPktBlobXferErr;

    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_JOIN_REQ ]                    = &PktPluginHandlerBase::onPktHostJoinReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_JOIN_REPLY ]                  = &PktPluginHandlerBase::onPktHostJoinReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_LEAVE_REQ ]					= &PktPluginHandlerBase::onPktHostLeaveReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_LEAVE_REPLY ]					= &PktPluginHandlerBase::onPktHostLeaveReply;


    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_SEARCH_REQ ]                  = &PktPluginHandlerBase::onPktHostSearchReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_SEARCH_REPLY ]                = &PktPluginHandlerBase::onPktHostSearchReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_OFFER_REQ ]                   = &PktPluginHandlerBase::onPktHostOfferReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_OFFER_REPLY ]                 = &PktPluginHandlerBase::onPktHostOfferReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_OFFER_REQ ]                 = &PktPluginHandlerBase::onPktFriendOfferReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_OFFER_REPLY ]               = &PktPluginHandlerBase::onPktFriendOfferReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_GET_REQ ]					= &PktPluginHandlerBase::onPktThumbGetReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_GET_REPLY ]					= &PktPluginHandlerBase::onPktThumbGetReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_SEND_REQ ]                   = &PktPluginHandlerBase::onPktThumbSendReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_SEND_REPLY ]                 = &PktPluginHandlerBase::onPktThumbSendReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_CHUNK_REQ ]                  = &PktPluginHandlerBase::onPktThumbChunkReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_CHUNK_REPLY ]                = &PktPluginHandlerBase::onPktThumbChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_GET_COMPLETE_REQ ]			= &PktPluginHandlerBase::onPktThumbGetCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_GET_COMPLETE_REPLY ]			= &PktPluginHandlerBase::onPktThumbGetCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_SEND_COMPLETE_REQ ]          = &PktPluginHandlerBase::onPktThumbSendCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_SEND_COMPLETE_REPLY ]        = &PktPluginHandlerBase::onPktThumbSendCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_THUMB_XFER_ERR ]                   = &PktPluginHandlerBase::onPktThumbXferErr;

    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_REQ ]                   = &PktPluginHandlerBase::onPktOfferSendReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_REPLY ]                 = &PktPluginHandlerBase::onPktOfferSendReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_CHUNK_REQ ]                  = &PktPluginHandlerBase::onPktOfferChunkReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_CHUNK_REPLY ]                = &PktPluginHandlerBase::onPktOfferChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_COMPLETE_REQ ]          = &PktPluginHandlerBase::onPktOfferSendCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_COMPLETE_REPLY ]        = &PktPluginHandlerBase::onPktOfferSendCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_XFER_ERR ]                   = &PktPluginHandlerBase::onPktOfferXferErr;

	m_aBaseSysPktFuncTable[PKT_TYPE_PUSH_TO_TALK_REQ]					= &PktPluginHandlerBase::onPktPushToTalkReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_PUSH_TO_TALK_REPLY]					= &PktPluginHandlerBase::onPktPushToTalkReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PUSH_TO_TALK_START ]				= &PktPluginHandlerBase::onPktPushToTalkStart;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PUSH_TO_TALK_STOP ]				= &PktPluginHandlerBase::onPktPushToTalkStop;

	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INFO_REQ]						= &PktPluginHandlerBase::onPktHostInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INFO_REPLY]					= &PktPluginHandlerBase::onPktHostInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_ANN_REQ]				= &PktPluginHandlerBase::onPktHostInviteAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_ANN_REPLY]				= &PktPluginHandlerBase::onPktHostInviteAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_SEARCH_REQ]				= &PktPluginHandlerBase::onPktHostInviteSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_SEARCH_REPLY]			= &PktPluginHandlerBase::onPktHostInviteSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_MORE_REQ]				= &PktPluginHandlerBase::onPktHostInviteMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_MORE_REPLY]				= &PktPluginHandlerBase::onPktHostInviteMoreReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_INFO_REQ]					= &PktPluginHandlerBase::onPktGroupieInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_INFO_REPLY]					= &PktPluginHandlerBase::onPktGroupieInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_ANN_REQ]					= &PktPluginHandlerBase::onPktGroupieAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_ANN_REPLY]					= &PktPluginHandlerBase::onPktGroupieAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_SEARCH_REQ]					= &PktPluginHandlerBase::onPktGroupieSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_SEARCH_REPLY]				= &PktPluginHandlerBase::onPktGroupieSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_MORE_REQ]					= &PktPluginHandlerBase::onPktGroupieMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_MORE_REPLY]					= &PktPluginHandlerBase::onPktGroupieMoreReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_INFO_REQ]					= &PktPluginHandlerBase::onPktFileInfoInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_INFO_REPLY]				= &PktPluginHandlerBase::onPktFileInfoInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_ANN_REQ]					= &PktPluginHandlerBase::onPktFileInfoAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_ANN_REPLY]				= &PktPluginHandlerBase::onPktFileInfoAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_SEARCH_REQ]				= &PktPluginHandlerBase::onPktFileInfoSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_SEARCH_REPLY]				= &PktPluginHandlerBase::onPktFileInfoSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_MORE_REQ]					= &PktPluginHandlerBase::onPktFileInfoMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_MORE_REPLY]				= &PktPluginHandlerBase::onPktFileInfoMoreReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_MEMBERSHIP_REQ ]					= &PktPluginHandlerBase::onPktMembershipReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_MEMBERSHIP_REPLY ]					= &PktPluginHandlerBase::onPktMembershipReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_RELAY_USER_DISCONNECT ]			= &PktPluginHandlerBase::onPktRelayUserDisconnect;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_INFO_REQ ]				= &PktPluginHandlerBase::onPktHostUserInfoReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_INFO_REPLY ]				= &PktPluginHandlerBase::onPktHostUserInfoReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_STATUS_REQ ]				= &PktPluginHandlerBase::onPktHostUserStatusReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_STATUS_REPLY ]			= &PktPluginHandlerBase::onPktHostUserStatusReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_REQ ]				= &PktPluginHandlerBase::onPktHostUserListReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_REPLY ]				= &PktPluginHandlerBase::onPktHostUserListReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_MORE_REQ ]			= &PktPluginHandlerBase::onPktHostUserListMoreReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_MORE_REPLY ]		= &PktPluginHandlerBase::onPktHostUserListMoreReply;
	
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_TEST_REQ ]				= &PktPluginHandlerBase::onPktTestConnTestReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_TEST_REPLY ]				= &PktPluginHandlerBase::onPktTestConnTestReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_PING_REQ ]				= &PktPluginHandlerBase::onPktTestConnPingReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_PING_REPLY ]				= &PktPluginHandlerBase::onPktTestConnPingReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_QUERY_HOST_URL_REQ ]				= &PktPluginHandlerBase::onPktQueryHostUrlReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_QUERY_HOST_URL_REPLY ]				= &PktPluginHandlerBase::onPktQueryHostUrlReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_STREAM_CTRL_REQ ]					= &PktPluginHandlerBase::onPktStreamCtrlReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_STREAM_CTRL_REPLY ]				= &PktPluginHandlerBase::onPktStreamCtrlReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_RAND_CONNECT_REQ ]					= &PktPluginHandlerBase::onPktRandConnectReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_RAND_CONNECT_REPLY ]				= &PktPluginHandlerBase::onPktRandConnectReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_REQUEST_REQ ]				= &PktPluginHandlerBase::onPktFriendRequestReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_REQUEST_REPLY ]				= &PktPluginHandlerBase::onPktFriendRequestReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_ADMIN_AVAIL]						= &PktPluginHandlerBase::onPktAdminAvail;
}

//============================================================================
//! Handle Incoming packet.. use function jump table for speed
void PktPluginHandlerBase::handlePkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PktPluginHandlerBase::handlePkt type %d len %d plugin #%d %s from %s", pktHdr->getPktType(), pktHdr->getPktLength(),
        pktHdr->getPluginNum(), DescribePluginType( (EPluginType)pktHdr->getPluginNum() ), sktBase->getRemoteIpAddress() );
    if( pktHdr->getPktType() <= sizeof(  m_aBaseSysPktFuncTable ) / (sizeof( void *)) )
		return (this->*m_aBaseSysPktFuncTable[ pktHdr->getPktType() ])(sktBase, pktHdr, netIdent);
	return onPktInvalid(sktBase, pktHdr, netIdent);
}

//=== packet handlers ===//
//============================================================================
void PktPluginHandlerBase::onPktUnhandled( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_WARN, "PktPluginHandlerBase::onPktUnhandled pkt %s", pktHdr->describePktHdr().c_str() );
}

//============================================================================
void PktPluginHandlerBase::onPktInvalid( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_WARN, "PktPluginHandlerBase::onPktInvalid pkt  %s", pktHdr->describePktHdr().c_str() );
}

//============================================================================
void PktPluginHandlerBase::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFindFileReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFindFileReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktFileSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktFileGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktFileShareErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktAssetGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktAssetGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktMultiSessionReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktMultiSessionReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktSessionStartReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktSessionStartReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktSessionStopReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktAnnList( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktHostUnJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktScanReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktScanReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktMyPicSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktMyPicSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerPicChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerPicChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerGetChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerGetChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerPutChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktWebServerPutChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktTcpPunch( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktImAliveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktImAliveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}	

//============================================================================
void PktPluginHandlerBase::onPktBlobSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktBlobXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostLeaveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFriendOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFriendOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktThumbXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

// offers
//============================================================================
void PktPluginHandlerBase::onPktOfferSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktOfferXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktMembershipReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktMembershipReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktGroupieMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFileInfoMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktRelayUserDisconnect( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktTestConnTestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktTestConnTestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktTestConnPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktTestConnPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktQueryHostUrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktQueryHostUrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktStreamCtrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktStreamCtrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktRandConnectReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktRandConnectReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFriendRequestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktFriendRequestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}

//============================================================================
void PktPluginHandlerBase::onPktAdminAvail( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktUnhandled( sktBase, pktHdr, netIdent );
}
