//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPktHdr.h"

#include "PktTypes.h"
#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxDebug.h>

#include <memory.h>


//============================================================================
VxPktHdrPrefix::VxPktHdrPrefix( const VxPktHdrPrefix& rhs )
    : m_u16PktLen( rhs.m_u16PktLen )
    , m_u16PktType( rhs.m_u16PktType )
    , m_u8PluginNum( rhs.m_u8PluginNum )
    , m_u8SeqNum( rhs.m_u8SeqNum )
    , m_RouteFlags( rhs.m_RouteFlags )
    , m_u8PktVersion( rhs.m_u8PktVersion )
{
}

//============================================================================
VxPktHdrPrefix&  VxPktHdrPrefix::operator = ( const VxPktHdrPrefix& rhs )
{
    if( this != &rhs )
    {
        m_u16PktLen = rhs.m_u16PktLen;
        m_u16PktType = rhs.m_u16PktType;
        m_u8PluginNum = rhs.m_u8PluginNum;
        m_u8SeqNum = rhs.m_u8SeqNum;
        m_RouteFlags = rhs.m_RouteFlags;
        m_u8PktVersion = rhs.m_u8PktVersion;
    }

    return *this;
}

//============================================================================
bool VxPktHdrPrefix::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_u16PktLen );
    result &= blob.setValue( m_u16PktType );
    result &= blob.setValue( m_u8PluginNum );
    result &= blob.setValue( m_u8SeqNum );
    result &= blob.setValue( m_RouteFlags );
    result &= blob.setValue( m_u8PktVersion );
    return result;
}

//============================================================================
bool VxPktHdrPrefix::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_u16PktLen );
    result &= blob.getValue( m_u16PktType );
    result &= blob.getValue( m_u8PluginNum );
    result &= blob.getValue( m_u8SeqNum );
    result &= blob.getValue( m_RouteFlags );
    result &= blob.getValue( m_u8PktVersion );
    return result;
}

//============================================================================
//=== return true if valid pkt type and length ===//
bool VxPktHdrPrefix::isValidPktPrefix( bool logIfInvalid )
{
	uint16_t u16PktLen = getPktLength();
	uint16_t u16PktType = getPktType();
    if( (16 > u16PktLen) 
		|| ( u16PktLen > MAX_PKT_LEN ) 
		|| ( u16PktLen & 0x0f ) 
		|| ( 1 > u16PktType ) 
		|| ( MAX_PKT_TYPE_CNT <= u16PktType ) )
    {
        if( logIfInvalid )
        {
            LogMsg( LOG_ERROR, "%s invalid packet len %d type %d", __func__, u16PktLen, u16PktType );
        }
        
		return false;
    }

    return true;
}

//============================================================================
bool VxPktHdrPrefix::isNetServicePkt( void )
{
    uint16_t u16PktType = getPktType();
    return PKT_TYPE_TEST_CONN_TEST_REQ == u16PktType ||
        PKT_TYPE_TEST_CONN_PING_REQ == u16PktType ||
        PKT_TYPE_TEST_CONN_PING_REPLY == u16PktType ||
        PKT_TYPE_TEST_CONN_TEST_REPLY == u16PktType ||
        PKT_TYPE_QUERY_HOST_URL_REQ == u16PktType ||
        PKT_TYPE_QUERY_HOST_URL_REPLY == u16PktType;
}

//============================================================================
//! return true if data length is large enough to contain this packet
bool VxPktHdrPrefix::isPktAllHere(int iDataLen)
{
    return (iDataLen >= ntohs(m_u16PktLen))?true:false;
}

//============================================================================
void VxPktHdrPrefix::setPktLength( uint16_t pktLen )
{
    m_u16PktLen = htons( pktLen );
}

//============================================================================
//! return length of packet
uint16_t VxPktHdrPrefix::getPktLength( void ) const
{
    return ntohs( m_u16PktLen );
}

//============================================================================
void VxPktHdrPrefix::setPktType( uint16_t u16PktType )
{
	m_u16PktType = htons( u16PktType );
}

//============================================================================
uint16_t VxPktHdrPrefix::getPktType( void ) const
{
	return ntohs( m_u16PktType );
}

//============================================================================
void VxPktHdrPrefix::setPluginNum( uint8_t u8PluginNum )
{
	m_u8PluginNum = u8PluginNum;
}

//============================================================================
uint8_t	VxPktHdrPrefix::getPluginNum( void )
{
	return m_u8PluginNum;
}

//============================================================================
void VxPktHdrPrefix::setPktVersionNum( uint8_t  u8PktVersionNum )
{
	m_u8PktVersion = u8PktVersionNum;
}

//============================================================================
uint8_t	VxPktHdrPrefix::getPktVersionNum( void )
{
	return m_u8PktVersion;
}

//============================================================================
VxPktHdr::VxPktHdr( const VxPktHdr& rhs )
    : VxPktHdrPrefix( rhs )
    , m_SrcOnlineId( rhs.m_SrcOnlineId )
    , m_DestOnlineId( rhs.m_DestOnlineId )
{
}

//============================================================================
VxPktHdr&  VxPktHdr::operator = ( const VxPktHdr& rhs )
{
    if( this != &rhs )
    {
        m_SrcOnlineId = rhs.m_SrcOnlineId;
        m_DestOnlineId = rhs.m_DestOnlineId;
    }

    return *this;
}

//============================================================================
bool VxPktHdr::addToBlob( PktBlobEntry& blob )
{
    bool result = VxPktHdrPrefix::addToBlob( blob );
    result &= blob.setValue( m_SrcOnlineId );
    result &= blob.setValue( m_DestOnlineId );
    return result;
}

//============================================================================
bool VxPktHdr::extractFromBlob( PktBlobEntry& blob )
{
    bool result = VxPktHdrPrefix::extractFromBlob( blob );
    result &= blob.getValue( m_SrcOnlineId );
    result &= blob.getValue( m_DestOnlineId );
    return result;
}

//============================================================================
VxGUID VxPktHdr::getSrcOnlineId( void )
{
    return getGuidInHostOrder( m_SrcOnlineId );
}

//============================================================================
VxGUID VxPktHdr::getDestOnlineId( void )
{
    return getGuidInHostOrder( m_DestOnlineId );
}

//============================================================================
//! make a copy of this packet
VxPktHdr* VxPktHdr::makeCopy( void )
{
    uint8_t * pu8Copy = new uint8_t[ getPktLength() ];
    memcpy( pu8Copy, this, getPktLength() );
    return (VxPktHdr*)pu8Copy;
}

//============================================================================
void VxPktHdr::setGuidToNetOrder( VxGUID& srcGuid, VxGUID& destGuid )
{
    destGuid = srcGuid;
    destGuid.setToNetOrder();
}

//============================================================================
VxGUID VxPktHdr::getGuidInHostOrder( VxGUID& srcGuid )
{
    VxGUID guidCopy = srcGuid;
    guidCopy.setToHostOrder();
    return guidCopy;
}

//============================================================================
std::string VxPktHdr::describePktHdr( void )
{
    std::string pktDesc = describePktType( getPktType() );
    pktDesc += " ";
    pktDesc += std::to_string( getPktType() );
    pktDesc += " len ";
    pktDesc += std::to_string( getPktLength() );
    pktDesc += " plugin ";
    pktDesc += std::to_string( getPluginNum() );

    return pktDesc;
}

//============================================================================
const char* VxPktHdr::describePktType( uint16_t pktType )
{
    switch( pktType )
    {
    case PKT_TYPE_ANNOUNCE:							return "PktAnnounce";					// 1
    case PKT_TYPE_ANN_LIST:							return "PktAnnList";				    // 2 ( 0x02 )		
    case PKT_TYPE_SCAN_REQ:							return "PktPktScanReq";                 // 3 ( 0x03 )
    case PKT_TYPE_SCAN_REPLY:						return "PktPktScanReply";			    // 4 ( 0x04 )
    case PKT_TYPE_PLUGIN_OFFER_REQ:					return "PktPluginOfferReq";			    // 5 ( 0x05 )
    case PKT_TYPE_PLUGIN_OFFER_REPLY:				return "PktPluginOfferRepl";		    // 6 ( 0x06 )
    case PKT_TYPE_CHAT_REQ:							return "PkChatReq";                     // 7 ( 0x07 )
    case PKT_TYPE_CHAT_REPLY:						return "PktChatReply";				    // 8 ( 0x08 )
    case PKT_TYPE_VOICE_REQ:						return "PktVoiceReq";			        // 9 ( 0x09 )
    case PKT_TYPE_VOICE_REPLY:						return "PktVoiceReply";			        // 10 ( 0x0a )
    case PKT_TYPE_VIDEO_FEED_REQ:					return "PktVideoFeedReq";			    // 11 ( 0x0b )
    case PKT_TYPE_VIDEO_FEED_STATUS:				return "PktVideoFeedStatus";		    // 12 ( 0x0c )
    case PKT_TYPE_VIDEO_FEED_PIC:					return "PktVideoFeedPic";               // 13 ( 0x0d )
    case PKT_TYPE_VIDEO_FEED_PIC_CHUNK:				return "PktVideoFeedPicChunk";		    // 14 ( 0x0e )
    case PKT_TYPE_VIDEO_FEED_PIC_ACK:				return "PktVideoFeedPicAck";	        // 15 ( 0x0f )
    case PKT_TYPE_SESSION_START_REQ:				return "PktSessionStartReq";	        // 16 ( 0x10 )
    case PKT_TYPE_SESSION_START_REPLY:				return "PktSessionStartReply";	        // 17 ( 0x11 )
    case PKT_TYPE_SESSION_STOP_REQ:					return "PktSessionStopReq";             // 18 ( 0x12 )
    case PKT_TYPE_SESSION_STOP_REPLY:				return "PktSessionStopReply";		    // 19 ( 0x13 )
    case PKT_TYPE_FILE_GET_REQ:						return "PktFileGetReq";                 // 20 ( 0x14 )
    case PKT_TYPE_FILE_GET_REPLY:					return "PktFileGetReply";			    // 21 ( 0x15 )
    case PKT_TYPE_FILE_SEND_REQ:					return "PktFileSendReq";		        // 22 ( 0x16 )
    case PKT_TYPE_FILE_SEND_REPLY:					return "PktFileSendReply";		        // 23 ( 0x17 )
    case PKT_TYPE_FILE_CHUNK_REQ:					return "PktFileChunkReq";		        // 24 ( 0x18 )
    case PKT_TYPE_FILE_CHUNK_REPLY:					return "PktFileChunkReply";		        // 25 ( 0x19 )
    case PKT_TYPE_FILE_GET_COMPLETE_REQ:			return "PkFileGetCompleteReq";			// 26 ( 0x1a )
    case PKT_TYPE_FILE_GET_COMPLETE_REPLY:			return "PktFileGetCompleteReply";	    // 27 ( 0x1b )
    case PKT_TYPE_FILE_SEND_COMPLETE_REQ:			return "PktFileSendCompleteReq";	    // 28 ( 0x1c )
    case PKT_TYPE_FILE_SEND_COMPLETE_REPLY:			return "PktFileSendCompleteReply";	    // 29 ( 0x1d )
    case  PKT_TYPE_FILE_XFER_CANCEL:                return "PktFileXferCancel";	            // 30 ( 0x1e )

    case PKT_TYPE_FILE_FIND_REQ:					return "PktFindFileReq";                // 31 ( 0x1f )
    case PKT_TYPE_FILE_FIND_REPLY:					return "PktFindFileReply";			    // 32 ( 0x20 )
    case PKT_TYPE_FILE_LIST_REQ:					return "PktFileListReq";			    // 33 ( 0x21 )
    case PKT_TYPE_FILE_LIST_REPLY:					return "PktFileListReply";			    // 34 ( 0x22 )
    case PKT_TYPE_FILE_INFO_REQ:					return "PktFileInfoReq";			    // 35 ( 0x23 )
    case PKT_TYPE_FILE_INFO_REPLY:					return "PktFileInfoReply";			    // 36 ( 0x24 )
    case PKT_TYPE_FILE_SHARE_ERR:					return "PktFileShareErr";			    // 37 ( 0x25 )
    case PKT_TYPE_ASSET_GET_REQ:					return "PktAssetGetReq";	            // 38 ( 0x26 )
    case PKT_TYPE_ASSET_GET_REPLY:					return "PktAssetGetReply";			    // 39 ( 0x27 )
    case PKT_TYPE_ASSET_SEND_REQ:					return "PktAssetSendReq";				// 40 ( 0x28 )
    case PKT_TYPE_ASSET_SEND_REPLY:					return "PktAssetSendReply";				// 41 ( 0x29 )
    case PKT_TYPE_ASSET_CHUNK_REQ:					return "PktAssetChunkReq";				// 42 ( 0x2A )
    case PKT_TYPE_ASSET_CHUNK_REPLY:				return "PktAssetChunkReply";            // 43 ( 0x2B )
    case PKT_TYPE_ASSET_GET_COMPLETE_REQ:			return "PktAssetGetCompleteReq";        // 44 ( 0x2C )
    case PKT_TYPE_ASSET_GET_COMPLETE_REPLY:			return "PktAssetGetCompleteReply";	    // 45 ( 0x2D )
    case PKT_TYPE_ASSET_SEND_COMPLETE_REQ:			return "PktAssetSendCompleteReq";		// 46 ( 0x2E )
    case PKT_TYPE_ASSET_SEND_COMPLETE_REPLY:		return "PktAssetSendCompleteReply";		// 47 ( 0x2F )
    case PKT_TYPE_ASSET_XFER_ERR:					return "PktAssetXferErr";               // 49 ( 0x31 )

    case PKT_TYPE_THUMB_GET_REQ:					return "PktThumbGetReq";                
    case PKT_TYPE_THUMB_GET_REPLY:					return "PktThumbGetReply";              // 50 ( 0x32 ) 
    case PKT_TYPE_THUMB_SEND_REQ:					return "PktThumbSendReq";               // 51 ( 0x33 ) 
    case PKT_TYPE_THUMB_SEND_REPLY:				    return "PktThumbSendReply";             // 52 ( 0x34 )
    case PKT_TYPE_THUMB_CHUNK_REQ:				    return "PktThumbChunkReq";              // 53 ( 0x35 )
    case PKT_TYPE_THUMB_CHUNK_REPLY:				return "PktThumbChunkReply";		    // 54 ( 0x36 )
    case PKT_TYPE_THUMB_GET_COMPLETE_REQ:			return "PktThumbGetCompleteReq";		// 55 ( 0x37 )
    case PKT_TYPE_THUMB_GET_COMPLETE_REPLY:			return "PktThumbGetCompleteReply";      // 56 ( 0x38 )
    case PKT_TYPE_THUMB_SEND_COMPLETE_REQ:		    return "PktThumbSendCompleteReq";       // 57 ( 0x39 ) 
    case PKT_TYPE_THUMB_SEND_COMPLETE_REPLY:        return "PktThumbSendCompleteReply";     // 58 ( 0x3A ) 
    case PKT_TYPE_THUMB_XFER_ERR:					return "PktThumbXferErr";               // 69 ( 0x3B )	

    case PKT_TYPE_MSESSION_REQ:						return "PktMultiSessionReq";            // 60 ( 0x3C )	
    case PKT_TYPE_MSESSION_REPLY:					return "PktMultiSessionReq";            // 61 ( 0x3D ) 

    case PKT_TYPE_TCP_PUNCH:						return "PktTcpPunch";                   // 62 ( 0x3E )
    case PKT_TYPE_PING_REQ:							return "PktPingReq";                    // 63 ( 0x3F ) 
    case PKT_TYPE_PING_REPLY:						return "PktPingReply";                  // 64 ( 0x40 ) 
    case PKT_TYPE_IM_ALIVE_REQ:						return "PktImAliveReq";                 // 65 ( 0x41 )   
    case PKT_TYPE_IM_ALIVE_REPLY:					return "PktImAliveReply";

    case PKT_TYPE_TOD_GAME_STATS:					return "PktTodGameStats";               // 67 ( 0x43 )
    case PKT_TYPE_TOD_GAME_ACTION:					return "PktTodGameAction";			    // 68 ( 0x44 )     
    case PKT_TYPE_TOD_GAME_VALUE:					return "PktTodGameValue";               // 69 ( 0x45 )     

    case PKT_TYPE_BLOB_SEND_REQ:					return "PkBlobSendReq";                 // 70 ( 0x46 )
    case PKT_TYPE_BLOB_SEND_REPLY:				    return "PktBlobSendReply";			    // 71 ( 0x47 )
    case PKT_TYPE_BLOB_CHUNK_REQ:				    return "PktBlobChunkReq";			    // 72 ( 0x48 )
    case PKT_TYPE_BLOB_CHUNK_REPLY:				    return "PktBlobChunkReply";			    // 73 ( 0x49 )
    case PKT_TYPE_BLOB_SEND_COMPLETE_REQ:		    return "PktBlobSendCompleteReq";		// 74 ( 0x4a )
    case PKT_TYPE_BLOB_SEND_COMPLETE_REPLY:		    return "PktBlobSendCompleteReply ";	    // 75 ( 0x4b )
    case PKT_TYPE_BLOB_XFER_ERR:					return "PktBlobXferErr";

    case PKT_TYPE_HOST_JOIN_REQ:		            return "PktHostJoinReq";	            // 77 ( 0x4d ) 
    case PKT_TYPE_HOST_JOIN_REPLY:					return "PktHostJoinReply";	            // 78 ( 0x4e )
    case PKT_TYPE_HOST_UNJOIN_REQ:					return "PktHostUnJoinReq";				// 79 ( 0x4f ) 
    case PKT_TYPE_HOST_UNJOIN_REPLY:				return "PktHostUnJoinReply";	        // 80 ( 0x50 )
    case PKT_TYPE_HOST_LEAVE_REQ:		            return "PktHostLeaveReq";	            // 81 ( 0x51 ) 
    case PKT_TYPE_HOST_LEAVE_REPLY:					return "PktHostLeaveReply";

    case PKT_TYPE_HOST_OFFER_REQ:		            return "PktHostOfferReq";	            // 83 ( 0x53 )  
    case PKT_TYPE_HOST_OFFER_REPLY:					return "PktHostOfferReply";

    case PKT_TYPE_HOST_SEARCH_REQ:		            return "PktHostSearchReq";	            // 85 ( 0x55 )
    case PKT_TYPE_HOST_SEARCH_REPLY:                return "PktHostSearchReply";

    case PKT_TYPE_FRIEND_OFFER_REQ:		            return "PktFriendOfferReq";	            // 87 ( 0x57 )
    case PKT_TYPE_FRIEND_OFFER_REPLY:               return "PktFriendOfferReply";

    case PKT_TYPE_OFFER_SEND_REQ:					return "PktOfferSendReq";               // 89 ( 0x59 )
    case PKT_TYPE_OFFER_SEND_REPLY:					return "PktOfferSendReply";			    // 90 ( 0x5A )
    case PKT_TYPE_OFFER_CHUNK_REQ:					return "PktOfferChunkReq";			    // 91 ( 0x5B ) 
    case PKT_TYPE_OFFER_CHUNK_REPLY:				return "PktOfferChunkReply";			// 92 ( 0x5C ) 
    case PKT_TYPE_OFFER_SEND_COMPLETE_REQ:			return "PktOfferSendCompleteReq";       // 93 ( 0x5D ) 
    case PKT_TYPE_OFFER_SEND_COMPLETE_REPLY:		return "PktOfferSendCompleteReply";	    // 94 ( 0x5E ) 
    case PKT_TYPE_OFFER_XFER_ERR:					return "PktOfferXferErr";

    case PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REQ:		return "PktMyPicSendReq";				// 96 ( 0x60 )
    case PKT_TYPE_WEB_SERVER_MY_PIC_SEND_REPLY:		return "PktMyPicSendReply";             // 97 ( 0x61 ) 
    case PKT_TYPE_WEB_SERVER_PIC_CHUNK_TX:			return "PktWebServerPicChunkTx";        // 98 ( 0x62 )
    case PKT_TYPE_WEB_SERVER_PIC_CHUNK_ACK:			return "PktWebServerPicChunkAck";		// 99 ( 0x63 )
    case PKT_TYPE_WEB_SERVER_GET_CHUNK_TX:			return "PktWebServerGetChunkTx";        // 100 ( 0x64 )
    case PKT_TYPE_WEB_SERVER_GET_CHUNK_ACK:			return "PktWebServerGetChunkAck";       // 101 ( 0x65 )
    case PKT_TYPE_WEB_SERVER_PUT_CHUNK_TX:			return "PktWebServerPutChunkTx";        // 102 ( 0x66 ) 
    case PKT_TYPE_WEB_SERVER_PUT_CHUNK_ACK:			return "PktWebServerPutChunkAck";

    case PKT_TYPE_PUSH_TO_TALK_REQ:				    return "PktPushToTalkReq";		        // 104 ( 0x68 ) 
    case PKT_TYPE_PUSH_TO_TALK_REPLY:				return "PktPushToTalkReply";            // 105 ( 0x69 ) 
    case PKT_TYPE_PUSH_TO_TALK_START:	            return "PktPushToTalkStart";	        // 106 ( 0x6A )
    case PKT_TYPE_PUSH_TO_TALK_STOP:	            return "PktPushToTalkStop";

    case PKT_TYPE_HOST_INFO_REQ:				    return "PktHostInfoReq";                // 108 ( 0x6C ) 
    case PKT_TYPE_HOST_INFO_REPLY:			        return "PktHostInfoReply";

    case PKT_TYPE_HOST_INVITE_ANN_REQ:				return "PktHostInviteAnnReq";		    // 110 ( 0x6E ) 
    case PKT_TYPE_HOST_INVITE_ANN_REPLY:			return "PktHostInviteAnnReply";         // 111 ( 0x6F )
    case PKT_TYPE_HOST_INVITE_SEARCH_REQ:			return "PktHostInviteSearchReq";		// 112 ( 0x70 )
    case PKT_TYPE_HOST_INVITE_SEARCH_REPLY:			return "PktHostInviteSearchReply";      // 113 ( 0x71 ) 
    case PKT_TYPE_HOST_INVITE_MORE_REQ:				return "PktHostInviteMoreReq";          // 114 ( 0x72 ) 
    case PKT_TYPE_HOST_INVITE_MORE_REPLY:			return "PktHostInviteMoreReply";

    case PKT_TYPE_GROUPIE_INFO_REQ:				    return "PktGroupieInfoReq";             // 116 ( 0x74 ) 
    case PKT_TYPE_GROUPIE_INFO_REPLY:			    return "PktGroupieInfoReply";

    case PKT_TYPE_GROUPIE_ANN_REQ:				    return "PktGroupieAnnReq";              // 118 ( 0x76 ) 
    case PKT_TYPE_GROUPIE_ANN_REPLY:			    return "PktGroupieAnnReply";            // 119 ( 0x77 ) 
    case PKT_TYPE_GROUPIE_SEARCH_REQ:				return "PktGroupieSearchReq";		    // 120 ( 0x78 ) 
    case PKT_TYPE_GROUPIE_SEARCH_REPLY:			    return "PktGroupieSearchReply";         // 121 ( 0x79 )	
    case PKT_TYPE_GROUPIE_MORE_REQ:				    return "PktGroupieMoreReq";             // 122 ( 0x7A )	
    case PKT_TYPE_GROUPIE_MORE_REPLY:               return "PktGroupieMoreReply";

    case PKT_TYPE_FILE_INFO_INFO_REQ:				return "PktFileInfoReq";                // 124 ( 0x7C )
    case PKT_TYPE_FILE_INFO_INFO_REPLY:			    return "PktFileInfoReply";

    case PKT_TYPE_FILE_INFO_ANN_REQ:				return "PktFileInfoAnnReq";             // 126 ( 0x7E )
    case PKT_TYPE_FILE_INFO_ANN_REPLY:			    return "PktFileInfoAnnReply";           // 127 ( 0x7F )
    case PKT_TYPE_FILE_INFO_SEARCH_REQ:				return "PktFileInfoSearchReq";          // 128 ( 0x80 )
    case PKT_TYPE_FILE_INFO_SEARCH_REPLY:			return "PktFileInfoSearchReply";        // 129 ( 0x81 )
    case PKT_TYPE_FILE_INFO_MORE_REQ:				return "PktFileInfoMoreReq";            // 130 ( 0x82 )
    case PKT_TYPE_FILE_INFO_MORE_REPLY:			    return "PktFileInfoMoreReply";

    case PKT_TYPE_MEMBERSHIP_REQ:				    return "PktMembershipReq";  		    // 132 ( 0x84 )	
    case PKT_TYPE_MEMBERSHIP_REPLY:				    return "PktMembershipReply";
	
    case PKT_TYPE_RELAY_USER_DISCONNECT:			return "PktRelayUserDisconnect";
	 
    case PKT_TYPE_HOST_USER_INFO_REQ:				return "PktHostUserInfoReq";  		    // 135 ( 0x87 )		
    case PKT_TYPE_HOST_USER_INFO_REPLY:				return "PktHostUserInfoReply";     		// 136 ( 0x88 )		
    case PKT_TYPE_HOST_USER_STATUS_REQ:				return "PktHostUserStatusReq";  		// 137 ( 0x89 )		
    case PKT_TYPE_HOST_USER_STATUS_REPLY:			return "PktHostUserStatusReply";

    case PKT_TYPE_HOST_USER_LIST_REQ:				return "PktHostUserListReq";  		    // 139 ( 0x8B )		 
    case PKT_TYPE_HOST_USER_LIST_REPLY:				return "PktHostUserListReply";  		// 140 ( 0x8C )		 
    case PKT_TYPE_HOST_USER_LIST_MORE_REQ:		    return "PktHostUserListMoreReq"; 		// 141 ( 0x8D )		 
    case PKT_TYPE_HOST_USER_LIST_MORE_REPLY:		return "PktHostUserListMoreReq";

    case PKT_TYPE_TEST_CONN_TEST_REQ:				return "PktTestConnTestReq";   		    // 143 ( 0x8F )		
    case PKT_TYPE_TEST_CONN_TEST_REPLY:				return "PktTestConnTestReply";  		// 144 ( 0x90 )		
    case PKT_TYPE_TEST_CONN_PING_REQ:				return "PktTestConnPingReq";   			// 145 ( 0x91 )	
    case PKT_TYPE_TEST_CONN_PING_REPLY:				return "PktTestConnPingReply";

    case PKT_TYPE_QUERY_HOST_URL_REQ:				return "PktQueryHostUrlReq";  			// 147 ( 0x93 )	
    case PKT_TYPE_QUERY_HOST_URL_REPLY:				return "PktQueryHostUrlReply";
	
    case PKT_TYPE_STREAM_CTRL_REQ:				    return "PktStreamCtrlReq";  			// 149 ( 0x95 )	
    case PKT_TYPE_STREAM_CTRL_REPLY:				return "PktStreamCtrlReply";

    case PKT_TYPE_RAND_CONNECT_REQ:				    return "PktRandConnectReq";  			// 151 ( 0x97 )	
    case PKT_TYPE_RAND_CONNECT_REPLY:				return "PktRandConnectReply";           // 152 ( 0x98 )	

    case PKT_TYPE_FRIEND_REQUEST_REQ:				return "PktFriendRequestReq";  			// 153 ( 0x99 )
    case PKT_TYPE_FRIEND_REQUEST_REPLY:				return "PktFriendRequestReply";  		// 154 ( 0x9A )

    case PKT_TYPE_ADMIN_AVAIL:				        return "PktAdminAvail";  		        // 155 ( 0x9B )

    case BASE_PKT_TYPE:                                                                                
        return "PktType 0 Not Allowed";   

    default:                                                                                
        return "PktUnknown";                                                                
    }
}

//============================================================================
bool VxPktHdr::isValidPktHdr( void )
{
    uint16_t u16PktLen = getPktLength();
	uint16_t u16PktType = getPktType();
    if( (16 > u16PktLen) 
		|| (u16PktLen > MAX_PKT_LEN ) 
		|| (u16PktLen & 0x0f) 
		|| (1 > u16PktType) 
		|| ( MAX_PKT_TYPE_CNT <= u16PktType ) )
    {
        LogMsg( LOG_ERROR, "%s invalid packet len %d type %d", __func__, u16PktLen, u16PktType );
		return false;
    }

    if( !getSrcOnlineId().isValid() )
    {
        LogMsg( LOG_ERROR, "%s invalid src online id", __func__ );
		return false;
    }

    if( !getDestOnlineId().isValid() )
    {
        LogMsg( LOG_ERROR, "%s invalid dest online id", __func__ );
		return false;
    }

    return true;
}