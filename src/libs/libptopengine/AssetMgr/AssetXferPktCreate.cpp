//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetXferMgr.h"

#include <PktLib/PktsAssetXfer.h>

//============================================================================
PktBaseGetReq* AssetXferMgr::createPktBaseGetReq( void )
{
    return new PktAssetGetReq();
}

//============================================================================
PktBaseGetReply* AssetXferMgr::createPktBaseGetReply( void )
{
    return new PktAssetGetReply();
}

//============================================================================
PktBaseSendReq* AssetXferMgr::createPktBaseSendReq( void )
{
    return new PktAssetSendReq();
}

//============================================================================
PktBaseSendReply* AssetXferMgr::createPktBaseSendReply( void )
{
    return new PktAssetSendReply();
}

//============================================================================
PktBaseChunkReq* AssetXferMgr::createPktBaseChunkReq( void )
{
    return new PktAssetChunkReq();
}

//============================================================================
PktBaseChunkReply* AssetXferMgr::createPktBaseChunkReply( void )
{
    return new PktAssetChunkReply();
}

//============================================================================
PktBaseGetCompleteReq* AssetXferMgr::createPktBaseGetCompleteReq( void )
{
    return new PktAssetGetCompleteReq();
}

//============================================================================
PktBaseGetCompleteReply* AssetXferMgr::createPktBaseGetCompleteReply( void )
{
    return new PktAssetGetCompleteReply();
}

//============================================================================
PktBaseSendCompleteReq* AssetXferMgr::createPktBaseSendCompleteReq( void )
{
    return new PktAssetSendCompleteReq();
}

//============================================================================
PktBaseSendCompleteReply* AssetXferMgr::createPktBaseSendCompleteReply( void )
{
    return new PktAssetSendCompleteReply();
}

//============================================================================
PktBaseXferErr* AssetXferMgr::createPktBaseXferErr( void )
{
    return new PktAssetXferErr();
}
