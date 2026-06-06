//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "BlobXferMgr.h"
#include "BlobInfo.h"
#include "BlobMgr.h"

#include "../Plugins/PluginBase.h"
#include "../Plugins/PluginMgr.h"
#include "../Plugins/PluginMessenger.h"
#include "BlobTxSession.h"
#include "BlobRxSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>

#include <PktLib/PktsBlobXfer.h>
#include <PktLib/VxCommon.h>
#include <NetLib/VxSktBase.h>

#include <CoreLib/AppErr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

//============================================================================
BlobXferMgr::BlobXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface )
    : AssetBaseXferMgr( engine, assetMgr, xferInterface )
{
}

//============================================================================
void BlobXferMgr::onPktBlobSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseChunkReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseChunkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void BlobXferMgr::onPktBlobXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_INFO, "AssetXferMgr::onPktAssetXferErr");
    AssetBaseXferMgr::onPktAssetBaseXferErr( sktBase, pktHdr, netIdent );
}
