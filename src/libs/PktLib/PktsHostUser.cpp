//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsHostUser.h"

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
PktHostUserInfoReq::PktHostUserInfoReq()
{
    setPktType( PKT_TYPE_HOST_USER_INFO_REQ );
    setPktLength( sizeof( PktHostUserInfoReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktHostUserInfoReply::PktHostUserInfoReply()
{
    setPktType( PKT_TYPE_HOST_USER_INFO_REPLY );
    setPktLength( sizeof( PktHostUserInfoReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktHostUserInfoReply::calcPktLen( void )
{
    uint16_t pktLen = ( uint16_t )sizeof( PktHostUserInfoReply ) - sizeof( PktBlobEntry );
    pktLen += getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
//============================================================================
PktHostUserStatusReq::PktHostUserStatusReq()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_HOST_USER_STATUS_REQ );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostUserStatusReq ) ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktHostUserStatusReply::PktHostUserStatusReply()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_HOST_USER_STATUS_REPLY );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostUserStatusReply ) ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
//============================================================================
PktHostUserListReq::PktHostUserListReq()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_HOST_USER_LIST_REQ );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostUserListReq ) ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktHostUserListReq::setHostId( HostedId& hostId )
{
    setHostType( hostId.getHostType() );
    setHostOnlineId( hostId.getHostOnlineId() );
}

//============================================================================
HostedId PktHostUserListReq::getHostId( void )
{
    return HostedId( getHostOnlineId(), getHostType() );
}

//============================================================================
PktHostUserListReply::PktHostUserListReply()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_HOST_USER_LIST_REPLY );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostUserListReply ) ) );
    // LogMsg( LOG_DEBUG, "PktHostUserReq size %d %d", sizeof( PktHostUserReq ), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktHostUserListReply::calcPktLen()
{
    uint16_t pktLen = ( uint16_t )sizeof( PktHostUserListReply ) - sizeof( PktBlobEntry );
    uint16_t blobLen = getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen + blobLen ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktHostUserListMoreReq::PktHostUserListMoreReq()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_HOST_USER_LIST_MORE_REQ );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostUserListMoreReq ) ) );

    vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
PktHostUserListMoreReply::PktHostUserListMoreReply()
    : VxPktHdr()
    , m_BlobEntry()
{
    setPktType( PKT_TYPE_HOST_USER_LIST_MORE_REPLY );
    setPktLength( sizeof( PktHostUserListMoreReply ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktHostUserListMoreReply::calcPktLen()
{
    uint16_t pktLen = ( uint16_t)sizeof( PktHostUserListMoreReply ) - sizeof(PktBlobEntry);
    uint16_t blobLen = getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen + blobLen ) ); 

    // LogMsg( LOG_DEBUG, "PktHostUserMoreReply calcPktLen blob %d len %d %d", blobLen, getPktLength(), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == (getPktLength() & 0x0f) );
}
