//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktsAssetXfer.h"

#include <CoreLib/VxDebug.h>

//============================================================================
// PktAssetSendReq
//============================================================================
PktAssetGetReq::PktAssetGetReq()
    : PktBaseGetReq() 
{ 
    setPktType( PKT_TYPE_ASSET_GET_REQ ); 
    setPktLength( sizeof( PktAssetGetReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetGetReply::PktAssetGetReply()
    : PktBaseGetReply()
{ 
    setPktType( PKT_TYPE_ASSET_GET_REPLY ); 
    setPktLength( sizeof( PktAssetGetReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
// PktAssetSendReq
//============================================================================
PktAssetSendReq::PktAssetSendReq()
: PktBaseSendReq() 
{ 
	setPktType( PKT_TYPE_ASSET_SEND_REQ ); 
    setPktLength( sizeof( PktAssetSendReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetSendReply::PktAssetSendReply()
: PktBaseSendReply()
{ 
	setPktType( PKT_TYPE_ASSET_SEND_REPLY ); 
	setPktLength( sizeof( PktAssetSendReply ) );
    int pktLen = sizeof( PktAssetSendReply );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetChunkReq::PktAssetChunkReq()
: PktBaseChunkReq()
{
	setPktType( PKT_TYPE_ASSET_CHUNK_REQ );
	setPktLength( emptyLength() );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetChunkReply::PktAssetChunkReply()
: PktBaseChunkReply()
{
	setPktType( PKT_TYPE_ASSET_CHUNK_REPLY );
	setPktLength( (uint16_t)sizeof( PktAssetChunkReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetGetCompleteReq::PktAssetGetCompleteReq()
    : PktBaseGetCompleteReq()
{
    setPktType(  PKT_TYPE_ASSET_GET_COMPLETE_REQ );
    setPktLength( (uint16_t)sizeof( PktAssetGetCompleteReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );}

//============================================================================
PktAssetGetCompleteReply::PktAssetGetCompleteReply()
    : PktBaseGetCompleteReply()
{
    setPktType( PKT_TYPE_ASSET_GET_COMPLETE_REPLY );
    setPktLength( (uint16_t)sizeof( PktAssetGetCompleteReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetSendCompleteReq::PktAssetSendCompleteReq()
: PktBaseSendCompleteReq()
{
	setPktType(  PKT_TYPE_ASSET_SEND_COMPLETE_REQ );
	setPktLength( (uint16_t)sizeof( PktAssetSendCompleteReq ) );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );}

//============================================================================
PktAssetSendCompleteReply::PktAssetSendCompleteReply()
: PktBaseSendCompleteReply()
{
	setPktType( PKT_TYPE_ASSET_SEND_COMPLETE_REPLY );
	setPktLength( (uint16_t)sizeof( PktAssetSendCompleteReply ) );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAssetXferErr::PktAssetXferErr()
: PktBaseXferErr() 
{
	setPktType( PKT_TYPE_ASSET_XFER_ERR ); 
	setPktLength( sizeof( PktAssetXferErr ) );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}
