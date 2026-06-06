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
#include "PktSysHandlerBase.h"

#include <NetLib/VxSktBase.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

PktHandlerBase::PktHandlerBase()
{
    for( size_t i = 0; i <= (sizeof( m_aBaseSysPktFuncTable )) / sizeof( void * ); i++ )
	{
		m_aBaseSysPktFuncTable[ i ] = &PktHandlerBase::onPktUnhandled;
	}

	int maxPktType = MAX_PKT_TYPE_CNT;
    vx_assert( 156 == maxPktType );

	m_aBaseSysPktFuncTable[ 0 ] = &PktHandlerBase::onPktInvalid;

	m_aBaseSysPktFuncTable[ PKT_TYPE_ANNOUNCE ]							= &PktHandlerBase::onPktAnnounce;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ANN_LIST ]							= &PktHandlerBase::onPktAnnList;

	m_aBaseSysPktFuncTable[PKT_TYPE_SCAN_REQ]							= &PktHandlerBase::onPktScanReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_SCAN_REPLY]							= &PktHandlerBase::onPktScanReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_PLUGIN_OFFER_REQ ]					= &PktHandlerBase::onPktPluginOfferReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PLUGIN_OFFER_REPLY ]				= &PktHandlerBase::onPktPluginOfferReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_CHAT_REQ ]							= &PktHandlerBase::onPktChatReq;

	m_aBaseSysPktFuncTable[ PKT_TYPE_VOICE_REQ ]						= &PktHandlerBase::onPktVoiceReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VOICE_REPLY ]						= &PktHandlerBase::onPktVoiceReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_REQ ]					= &PktHandlerBase::onPktVideoFeedReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_STATUS ]				= &PktHandlerBase::onPktVideoFeedStatus;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC ]					= &PktHandlerBase::onPktVideoFeedPic;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC_CHUNK ]				= &PktHandlerBase::onPktVideoFeedPicChunk;
	m_aBaseSysPktFuncTable[ PKT_TYPE_VIDEO_FEED_PIC_ACK ]				= &PktHandlerBase::onPktVideoFeedPicAck;

	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_START_REQ ]				= &PktHandlerBase::onPktSessionStartReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_START_REPLY ]				= &PktHandlerBase::onPktSessionStartReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_STOP_REQ ]					= &PktHandlerBase::onPktSessionStopReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_SESSION_STOP_REPLY ]				= &PktHandlerBase::onPktSessionStopReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_REQ ]						= &PktHandlerBase::onPktFileGetReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_REPLY ]					= &PktHandlerBase::onPktFileGetReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_REQ ]					= &PktHandlerBase::onPktFileSendReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_REPLY ]					= &PktHandlerBase::onPktFileSendReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_CHUNK_REQ ]					= &PktHandlerBase::onPktFileChunkReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_CHUNK_REPLY ]					= &PktHandlerBase::onPktFileChunkReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_COMPLETE_REQ ]			= &PktHandlerBase::onPktFileGetCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_GET_COMPLETE_REPLY ]			= &PktHandlerBase::onPktFileGetCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_COMPLETE_REQ ]			= &PktHandlerBase::onPktFileSendCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SEND_COMPLETE_REPLY ]			= &PktHandlerBase::onPktFileSendCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_XFER_CANCEL ]					= &PktHandlerBase::onPktFileXferCancel;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_FIND_REQ ]					= &PktHandlerBase::onPktFindFileReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_FIND_REPLY ]					= &PktHandlerBase::onPktFindFileReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_LIST_REQ ]					= &PktHandlerBase::onPktFileListReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_LIST_REPLY ]					= &PktHandlerBase::onPktFileListReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_INFO_REQ ]					= &PktHandlerBase::onPktFileInfoReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FILE_SHARE_ERR ]					= &PktHandlerBase::onPktFileShareErr;

    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_REQ ]					= &PktHandlerBase::onPktAssetGetReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_REPLY ]					= &PktHandlerBase::onPktAssetGetReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_REQ ]					= &PktHandlerBase::onPktAssetSendReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_REPLY ]					= &PktHandlerBase::onPktAssetSendReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_CHUNK_REQ ]					= &PktHandlerBase::onPktAssetChunkReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_CHUNK_REPLY ]				= &PktHandlerBase::onPktAssetChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_COMPLETE_REQ ]			= &PktHandlerBase::onPktAssetGetCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_GET_COMPLETE_REPLY ]			= &PktHandlerBase::onPktAssetGetCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_COMPLETE_REQ ]			= &PktHandlerBase::onPktAssetSendCompleteReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_SEND_COMPLETE_REPLY ]		= &PktHandlerBase::onPktAssetSendCompleteReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_ASSET_XFER_ERR ]					= &PktHandlerBase::onPktAssetXferErr;

	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_GET_REQ]						= &PktHandlerBase::onPktThumbGetReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_GET_REPLY]					= &PktHandlerBase::onPktThumbGetReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_SEND_REQ]						= &PktHandlerBase::onPktThumbSendReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_SEND_REPLY]					= &PktHandlerBase::onPktThumbSendReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_CHUNK_REQ]					= &PktHandlerBase::onPktThumbChunkReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_CHUNK_REPLY]					= &PktHandlerBase::onPktThumbChunkReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_GET_COMPLETE_REQ]				= &PktHandlerBase::onPktThumbGetCompleteReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_GET_COMPLETE_REPLY]			= &PktHandlerBase::onPktThumbGetCompleteReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_SEND_COMPLETE_REQ]			= &PktHandlerBase::onPktThumbSendCompleteReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_SEND_COMPLETE_REPLY]			= &PktHandlerBase::onPktThumbSendCompleteReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_THUMB_XFER_ERR]						= &PktHandlerBase::onPktThumbXferErr;

	m_aBaseSysPktFuncTable[ PKT_TYPE_MSESSION_REQ ]						= &PktHandlerBase::onPktMultiSessionReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_MSESSION_REPLY ]					= &PktHandlerBase::onPktMultiSessionReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_TCP_PUNCH]							= &PktHandlerBase::onPktTcpPunch;
	m_aBaseSysPktFuncTable[PKT_TYPE_PING_REQ]							= &PktHandlerBase::onPktPingReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_PING_REPLY]							= &PktHandlerBase::onPktPingReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_IM_ALIVE_REQ]						= &PktHandlerBase::onPktImAliveReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_IM_ALIVE_REPLY]						= &PktHandlerBase::onPktImAliveReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_STATS ]					= &PktHandlerBase::onPktTodGameStats;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_ACTION ]					= &PktHandlerBase::onPktTodGameAction;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TOD_GAME_VALUE ]					= &PktHandlerBase::onPktTodGameValue;
  
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_REQ ]                    = &PktHandlerBase::onPktBlobSendReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_REPLY ]                  = &PktHandlerBase::onPktBlobSendReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_CHUNK_REQ ]                   = &PktHandlerBase::onPktBlobChunkReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_CHUNK_REPLY ]                 = &PktHandlerBase::onPktBlobChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_COMPLETE_REQ ]           = &PktHandlerBase::onPktBlobSendCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_SEND_COMPLETE_REPLY ]         = &PktHandlerBase::onPktBlobSendCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_BLOB_XFER_ERR ]                    = &PktHandlerBase::onPktBlobXferErr;

    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_JOIN_REQ ]                    = &PktHandlerBase::onPktHostJoinReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_JOIN_REPLY ]                  = &PktHandlerBase::onPktHostJoinReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_UNJOIN_REQ ]					= &PktHandlerBase::onPktHostUnJoinReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_UNJOIN_REPLY ]				= &PktHandlerBase::onPktHostUnJoinReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_LEAVE_REQ ]					= &PktHandlerBase::onPktHostLeaveReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_LEAVE_REPLY ]					= &PktHandlerBase::onPktHostLeaveReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_OFFER_REQ ]                   = &PktHandlerBase::onPktHostOfferReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_OFFER_REPLY ]                 = &PktHandlerBase::onPktHostOfferReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_SEARCH_REQ]					= &PktHandlerBase::onPktHostSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_SEARCH_REPLY]					= &PktHandlerBase::onPktHostSearchReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_OFFER_REQ ]                 = &PktHandlerBase::onPktFriendOfferReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_OFFER_REPLY ]               = &PktHandlerBase::onPktFriendOfferReply;

    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_REQ ]                   = &PktHandlerBase::onPktOfferSendReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_REPLY ]                 = &PktHandlerBase::onPktOfferSendReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_CHUNK_REQ ]                  = &PktHandlerBase::onPktOfferChunkReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_CHUNK_REPLY ]                = &PktHandlerBase::onPktOfferChunkReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_COMPLETE_REQ ]          = &PktHandlerBase::onPktOfferSendCompleteReq;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_SEND_COMPLETE_REPLY ]        = &PktHandlerBase::onPktOfferSendCompleteReply;
    m_aBaseSysPktFuncTable[ PKT_TYPE_OFFER_XFER_ERR ]                   = &PktHandlerBase::onPktOfferXferErr;

	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REQ]			= &PktHandlerBase::onPktMyPicSendReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REPLY]		= &PktHandlerBase::onPktMyPicSendReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_PIC_CHUNK_TX]			= &PktHandlerBase::onPktWebServerPicChunkTx;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_PIC_CHUNK_ACK]			= &PktHandlerBase::onPktWebServerPicChunkAck;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_GET_CHUNK_TX]			= &PktHandlerBase::onPktWebServerGetChunkTx;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_GET_CHUNK_ACK]			= &PktHandlerBase::onPktWebServerGetChunkAck;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_PUT_CHUNK_TX]			= &PktHandlerBase::onPktWebServerPutChunkTx;
	m_aBaseSysPktFuncTable[PKT_TYPE_WEB_SERVER_PUT_CHUNK_ACK]			= &PktHandlerBase::onPktWebServerPutChunkAck;

	m_aBaseSysPktFuncTable[PKT_TYPE_PUSH_TO_TALK_REQ]					= &PktHandlerBase::onPktPushToTalkReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_PUSH_TO_TALK_REPLY]					= &PktHandlerBase::onPktPushToTalkReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PUSH_TO_TALK_START ]				= &PktHandlerBase::onPktPushToTalkStart;
	m_aBaseSysPktFuncTable[ PKT_TYPE_PUSH_TO_TALK_STOP ]				= &PktHandlerBase::onPktPushToTalkStop;

	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INFO_REQ]						= &PktHandlerBase::onPktHostInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INFO_REPLY]					= &PktHandlerBase::onPktHostInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_ANN_REQ]				= &PktHandlerBase::onPktHostInviteAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_ANN_REPLY]				= &PktHandlerBase::onPktHostInviteAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_SEARCH_REQ]				= &PktHandlerBase::onPktHostInviteSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_SEARCH_REPLY]			= &PktHandlerBase::onPktHostInviteSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_MORE_REQ]				= &PktHandlerBase::onPktHostInviteMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_HOST_INVITE_MORE_REPLY]				= &PktHandlerBase::onPktHostInviteMoreReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_INFO_REQ]					= &PktHandlerBase::onPktGroupieInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_INFO_REPLY]					= &PktHandlerBase::onPktGroupieInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_ANN_REQ]					= &PktHandlerBase::onPktGroupieAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_ANN_REPLY]					= &PktHandlerBase::onPktGroupieAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_SEARCH_REQ]					= &PktHandlerBase::onPktGroupieSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_SEARCH_REPLY]				= &PktHandlerBase::onPktGroupieSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_MORE_REQ]					= &PktHandlerBase::onPktGroupieMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_GROUPIE_MORE_REPLY]					= &PktHandlerBase::onPktGroupieMoreReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_INFO_REQ]					= &PktHandlerBase::onPktFileInfoInfoReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_INFO_REPLY]				= &PktHandlerBase::onPktFileInfoInfoReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_ANN_REQ]					= &PktHandlerBase::onPktFileInfoAnnReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_ANN_REPLY]				= &PktHandlerBase::onPktFileInfoAnnReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_SEARCH_REQ]				= &PktHandlerBase::onPktFileInfoSearchReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_SEARCH_REPLY]				= &PktHandlerBase::onPktFileInfoSearchReply;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_MORE_REQ]					= &PktHandlerBase::onPktFileInfoMoreReq;
	m_aBaseSysPktFuncTable[PKT_TYPE_FILE_INFO_MORE_REPLY]				= &PktHandlerBase::onPktFileInfoMoreReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_MEMBERSHIP_REQ ]					= &PktHandlerBase::onPktMembershipReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_MEMBERSHIP_REPLY ]					= &PktHandlerBase::onPktMembershipReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_RELAY_USER_DISCONNECT ]			= &PktHandlerBase::onPktRelayUserDisconnect;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_INFO_REQ ]				= &PktHandlerBase::onPktHostUserInfoReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_INFO_REPLY ]				= &PktHandlerBase::onPktHostUserInfoReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_STATUS_REQ ]				= &PktHandlerBase::onPktHostUserStatusReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_STATUS_REPLY ]			= &PktHandlerBase::onPktHostUserStatusReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_REQ ]				= &PktHandlerBase::onPktHostUserListReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_REPLY ]				= &PktHandlerBase::onPktHostUserListReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_MORE_REQ ]			= &PktHandlerBase::onPktHostUserListMoreReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_HOST_USER_LIST_MORE_REPLY ]		= &PktHandlerBase::onPktHostUserListMoreReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_TEST_REQ ]				= &PktHandlerBase::onPktTestConnTestReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_TEST_REPLY ]				= &PktHandlerBase::onPktTestConnTestReply;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_PING_REQ ]				= &PktHandlerBase::onPktTestConnPingReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_TEST_CONN_PING_REPLY ]				= &PktHandlerBase::onPktTestConnPingReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_QUERY_HOST_URL_REQ ]				= &PktHandlerBase::onPktQueryHostUrlReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_QUERY_HOST_URL_REPLY ]				= &PktHandlerBase::onPktQueryHostUrlReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_STREAM_CTRL_REQ ]					= &PktHandlerBase::onPktStreamCtrlReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_STREAM_CTRL_REPLY ]				= &PktHandlerBase::onPktStreamCtrlReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_RAND_CONNECT_REQ ]					= &PktHandlerBase::onPktRandConnectReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_RAND_CONNECT_REPLY ]				= &PktHandlerBase::onPktRandConnectReply;

	m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_REQUEST_REQ ]				= &PktHandlerBase::onPktFriendRequestReq;
	m_aBaseSysPktFuncTable[ PKT_TYPE_FRIEND_REQUEST_REPLY ]				= &PktHandlerBase::onPktFriendRequestReply;

	m_aBaseSysPktFuncTable[PKT_TYPE_ADMIN_AVAIL] =						&PktHandlerBase::onPktAdminAvail;
}

//============================================================================
//! Handle Incoming packet.. use function jump table for speed
void PktHandlerBase::handlePkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    if( VxIsAppShuttingDown() || !sktBase->isConnected() )
    {
        return;
    }

	uint16_t pktType = pktHdr->getPktType();
    if( pktType <= sizeof(  m_aBaseSysPktFuncTable ) / (sizeof( void * )) )
    {
		return (this->*m_aBaseSysPktFuncTable[ pktType ])(sktBase, pktHdr);
    }

	return onPktInvalid(sktBase, pktHdr);
}

//=== packet handlers ===//
//============================================================================
void PktHandlerBase::onPktUnhandled( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
}

//============================================================================
void PktHandlerBase::onPktInvalid( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	LogMsg( LOG_ERROR, "PktHandlerBase::onPktInvalid type 0x%x len 0x%x\n", pktHdr->getPktType(), pktHdr->getPktLength() );
}

//============================================================================
void PktHandlerBase::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktChatReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktChatReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVideoFeedReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVideoFeedStatus( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVideoFeedPic( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVideoFeedPicChunk( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktVideoFeedPicAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFindFileReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFindFileReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktFileSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktFileGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileShareErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktMultiSessionReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktMultiSessionReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktSessionStartReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktSessionStartReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktSessionStopReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktAnnList( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktHostUnJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktScanReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktScanReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktMyPicSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktMyPicSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerPicChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerPicChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerGetChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerGetChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerPutChunkTx( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktWebServerPutChunkAck( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktTodGameStats( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktTodGameAction( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktTodGameValue( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktTcpPunch( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktImAliveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}	

//============================================================================
void PktHandlerBase::onPktImAliveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktBlobXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostLeaveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFriendOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFriendOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktThumbXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

// offers

//============================================================================
void PktHandlerBase::onPktOfferSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktOfferXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
    onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktMembershipReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktMembershipReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktGroupieMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFileInfoMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktRelayUserDisconnect( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktTestConnTestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktTestConnTestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktTestConnPingReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktTestConnPingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktQueryHostUrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktQueryHostUrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktStreamCtrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktStreamCtrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktRandConnectReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktRandConnectReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFriendRequestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktFriendRequestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}

//============================================================================
void PktHandlerBase::onPktAdminAvail( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	onPktUnhandled( sktBase, pktHdr );
}
