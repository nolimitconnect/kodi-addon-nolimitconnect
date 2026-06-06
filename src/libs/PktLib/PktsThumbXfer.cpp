//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktsThumbXfer.h"

#include <CoreLib/VxDebug.h>

//============================================================================
// PktAssetSendReq
//============================================================================
PktThumbGetReq::PktThumbGetReq()
    : PktBaseGetReq() 
{ 
    setPktType( PKT_TYPE_THUMB_GET_REQ ); 
    setPktLength( sizeof( PktThumbGetReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktThumbGetReply::PktThumbGetReply()
    : PktBaseGetReply()
{ 
    setPktType( PKT_TYPE_THUMB_GET_REPLY ); 
    setPktLength( sizeof( PktThumbGetReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
//  PktThumbSendReq
//============================================================================
 PktThumbSendReq:: PktThumbSendReq()
    : PktBaseSendReq() 
{ 
    setPktType( PKT_TYPE_THUMB_SEND_REQ ); 
    setPktLength( sizeof(  PktThumbSendReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
 PktThumbSendReply:: PktThumbSendReply()
    : PktBaseSendReply()
{ 
    setPktType( PKT_TYPE_THUMB_SEND_REPLY ); 
    setPktLength( sizeof(  PktThumbSendReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
 PktThumbChunkReq:: PktThumbChunkReq()
    : PktBaseChunkReq()
{
    setPktType( PKT_TYPE_THUMB_CHUNK_REQ );
    setPktLength( emptyLength() );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
 PktThumbChunkReply:: PktThumbChunkReply()
    : PktBaseChunkReply()
{
    setPktType( PKT_TYPE_THUMB_CHUNK_REPLY );
    setPktLength( (uint16_t)sizeof(  PktThumbChunkReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

 //============================================================================
 PktThumbGetCompleteReq::PktThumbGetCompleteReq()
     : PktBaseGetCompleteReq()
 {
     setPktType(  PKT_TYPE_THUMB_GET_COMPLETE_REQ );
     setPktLength( (uint16_t)sizeof( PktThumbGetCompleteReq ) );
     vx_assert( 0 == ( getPktLength() & 0x0f ) );}

 //============================================================================
 PktThumbGetCompleteReply::PktThumbGetCompleteReply()
     : PktBaseGetCompleteReply()
 {
     setPktType( PKT_TYPE_THUMB_GET_COMPLETE_REPLY );
     setPktLength( (uint16_t)sizeof( PktThumbGetCompleteReply ) );
     vx_assert( 0 == ( getPktLength() & 0x0f ) );
 }

//============================================================================
 PktThumbSendCompleteReq:: PktThumbSendCompleteReq()
    : PktBaseSendCompleteReq()
{
    setPktType(  PKT_TYPE_THUMB_SEND_COMPLETE_REQ );
    setPktLength( (uint16_t)sizeof(  PktThumbSendCompleteReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );}

//============================================================================
 PktThumbSendCompleteReply:: PktThumbSendCompleteReply()
    : PktBaseSendCompleteReply()
{
    setPktType( PKT_TYPE_THUMB_SEND_COMPLETE_REPLY );
    setPktLength( (uint16_t)sizeof(  PktThumbSendCompleteReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
 PktThumbXferErr:: PktThumbXferErr()
    : PktBaseXferErr() 
{
    setPktType( PKT_TYPE_THUMB_XFER_ERR ); 
    setPktLength( sizeof(  PktThumbXferErr ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}
