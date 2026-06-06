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
#include "ThumbInfo.h"
#include "ThumbMgr.h"

#include "../Plugins/PluginBase.h"
#include "../Plugins/PluginMgr.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <AssetBase/AssetBaseMgr.h>
#include <AssetMgr/AssetMgr.h>

#include <PktLib/PktsAssetXfer.h>
#include <PktLib/VxCommon.h>
#include <NetLib/VxSktBase.h>

#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/AppErr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdarg.h>

//============================================================================
ThumbXferMgr::ThumbXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface )
    : AssetBaseXferMgr( engine, assetMgr, xferInterface )
{
}

//============================================================================
void ThumbXferMgr::onPktThumbGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseGetReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseGetReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseSendReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseSendReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseChunkReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseChunkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseGetCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseXferMgr::onPktAssetBaseGetCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s from %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbXferMgr::%s to %s", __func__, netIdent->getOnlineName() );
    AssetBaseXferMgr::onPktAssetBaseSendCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void ThumbXferMgr::onPktThumbXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_INFO, "ThumbXferMgr::%s", __func__ );
    AssetBaseXferMgr::onPktAssetBaseXferErr( sktBase, pktHdr, netIdent );
}

//============================================================================
bool ThumbXferMgr::requestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb )
{
    if( !netIdent || !thumbId.isValid() )
    {
        LogMsg( LOG_ERROR, "ThumbXferMgr::%s invalid param", __func__ );
        vx_assert( false );
        return false;
    }

    if( !sktBase || !sktBase->isConnected() )
    {
        LogMsg( LOG_ERROR, "ThumbXferMgr::%s skt not connected", __func__ );
        return false;
    }

    ThumbInfo thumbInfo( netIdent->getMyOnlineId(), thumbId );
    if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, " ThumbXferMgr::%s requesting thumb %s from %s tmp? %d", __func__, 
        thumbId.toHexString().c_str(), netIdent->getOnlineName(), tmpThumb );

    return AssetBaseXferMgr::fromGuiRequestAssetBase( netIdent, thumbInfo, sktBase, tmpThumb );
}
