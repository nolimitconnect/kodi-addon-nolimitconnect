//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ThumbXferMgr.h"

#include <PktLib/PktsThumbXfer.h>
#include <NetLib/VxSktBase.h>

//============================================================================
PktBaseGetReq* ThumbXferMgr::createPktBaseGetReq( void )
{
    return new PktThumbGetReq();
}

//============================================================================
PktBaseGetReply* ThumbXferMgr::createPktBaseGetReply( void )
{
    return new PktThumbGetReply();
}

//============================================================================
PktBaseSendReq* ThumbXferMgr::createPktBaseSendReq( void )
{
    return new PktThumbSendReq();
}

//============================================================================
PktBaseSendReply* ThumbXferMgr::createPktBaseSendReply( void )
{
    return new PktThumbSendReply();
}

//============================================================================
PktBaseChunkReq* ThumbXferMgr::createPktBaseChunkReq( void )
{
    return new PktThumbChunkReq();
}

//============================================================================
PktBaseChunkReply* ThumbXferMgr::createPktBaseChunkReply( void )
{
    return new PktThumbChunkReply();
}

//============================================================================
PktBaseGetCompleteReq* ThumbXferMgr::createPktBaseGetCompleteReq( void )
{
    return new PktThumbGetCompleteReq();
}

//============================================================================
PktBaseGetCompleteReply* ThumbXferMgr::createPktBaseGetCompleteReply( void )
{
    return new PktThumbGetCompleteReply();
}

//============================================================================
PktBaseSendCompleteReq* ThumbXferMgr::createPktBaseSendCompleteReq( void )
{
    return new PktThumbSendCompleteReq();
}

//============================================================================
PktBaseSendCompleteReply* ThumbXferMgr::createPktBaseSendCompleteReply( void )
{
    return new PktThumbSendCompleteReply();
}

//============================================================================
PktBaseXferErr* ThumbXferMgr::createPktBaseXferErr( void )
{
    return new PktThumbXferErr();
}
