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
#include "AssetXferMgr.h"
#include "AssetInfo.h"
#include "AssetMgr.h"

#include "../Plugins/PluginBase.h"
#include "../Plugins/PluginMgr.h"
#include "../Plugins/PluginMessenger.h"
#include "AssetTxSession.h"
#include "AssetRxSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>

#include <PktLib/PktsAssetXfer.h>
#include <PktLib/VxCommon.h>
#include <NetLib/VxSktBase.h>

#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/AppErr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

//============================================================================
AssetXferMgr::AssetXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface )
: AssetBaseXferMgr( engine, assetMgr, xferInterface )
{
}

//============================================================================
void AssetXferMgr::onPktAssetGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseGetReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseGetReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseChunkReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseChunkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseGetCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseGetCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void AssetXferMgr::onPktAssetXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_INFO, "AssetXferMgr::onPktAssetXferErr");
    AssetBaseXferMgr::onPktAssetBaseXferErr( sktBase, pktHdr, netIdent );
}

