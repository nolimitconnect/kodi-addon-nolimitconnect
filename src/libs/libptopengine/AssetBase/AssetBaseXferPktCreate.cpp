//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetBaseXferMgr.h"

#include <PktLib/PktsBaseXfer.h>

//============================================================================
PktBaseGetReq* AssetBaseXferMgr::createPktBaseGetReq( void )
{
    return new PktBaseGetReq();
}

//============================================================================
PktBaseGetReply* AssetBaseXferMgr::createPktBaseGetReply( void )
{
    return new PktBaseGetReply();
}

//============================================================================
PktBaseSendReq* AssetBaseXferMgr::createPktBaseSendReq( void )
{
    return new PktBaseSendReq();
}

//============================================================================
PktBaseSendReply* AssetBaseXferMgr::createPktBaseSendReply( void )
{
    return new PktBaseSendReply();
}

//============================================================================
PktBaseChunkReq* AssetBaseXferMgr::createPktBaseChunkReq( void )
{
    return new PktBaseChunkReq();
}

//============================================================================
PktBaseChunkReply* AssetBaseXferMgr::createPktBaseChunkReply( void )
{
    return new PktBaseChunkReply();
}

//============================================================================
PktBaseGetCompleteReq* AssetBaseXferMgr::createPktBaseGetCompleteReq( void )
{
    return new PktBaseGetCompleteReq();
}

//============================================================================
PktBaseGetCompleteReply* AssetBaseXferMgr::createPktBaseGetCompleteReply( void )
{
    return new PktBaseGetCompleteReply();
}

//============================================================================
PktBaseSendCompleteReq* AssetBaseXferMgr::createPktBaseSendCompleteReq( void )
{
    return new PktBaseSendCompleteReq();
}

//============================================================================
PktBaseSendCompleteReply* AssetBaseXferMgr::createPktBaseSendCompleteReply( void )
{
    return new PktBaseSendCompleteReply();
}

//============================================================================
PktBaseXferErr* AssetBaseXferMgr::createPktBaseXferErr( void )
{
    return new PktBaseXferErr();
}

//============================================================================

PktBaseListReq* AssetBaseXferMgr::createPktBaseListReq( void )
{
    return new PktBaseListReq();
}

//============================================================================
PktBaseListReply* AssetBaseXferMgr::createPktBaseListReply( void )
{
    return new PktBaseListReply();
}

