//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginFriendRequest.h"

#include "PluginMgr.h"

#include <GuiInterface/IToGui.h>
#include <FriendRequestMgr/FriendRequestMgr.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

#include <PktLib/PktsFriendRequest.h>

//============================================================================
PluginFriendRequest::PluginFriendRequest( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
{
    if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "PluginFriendRequest::%s", __func__ );
	setPluginType( ePluginTypeFriendRequest );
}

//============================================================================
void PluginFriendRequest::onPktFriendRequestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdentIn )
{
    PktFriendRequestReq* pkt = (PktFriendRequestReq*)pktHdr;

    VxNetIdent netIdent;
    PktBlobEntry& blobEntry = pkt->getBlobEntry();
    if( !netIdent.extractFromBlob( blobEntry ) )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s blob idx %d len %d", __func__, blobEntry.getDataIdx(), blobEntry.getBlobLen() );
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s extract failed", __func__ );
        return;
    }

    VxGUID requestId = pkt->getRequestId();
    if( !requestId.isValid() )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s invalid request id", __func__ );
        return;
    }

    std::string requestMsg = pkt->getRequestMsg();
    if( requestMsg.empty() )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s empty request message", __func__ );
        return;
    }

    P2PEngine& engine = GetPtoPEngine();
    engine.getFriendRequestMgr().rxedFriendRequest( sktBase, netIdent, requestId, requestMsg );

}
	
//============================================================================
void PluginFriendRequest::onPktFriendRequestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{

}

//============================================================================
bool PluginFriendRequest::fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestText, EFriendState myFriendshipToHim )
{
	P2PEngine& engine = GetPtoPEngine();
    std::shared_ptr<VxSktBase> sktBase =  engine.getConnectIdListMgr().findAnyUserOnlineConnection( onlineId );
    if( sktBase.get() == nullptr )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s no connection to user", __func__ );
        return false;
    }

    if( requestText.empty() )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s empty request message", __func__ );
        return false;
    }

    PktFriendRequestReq pkt;
    pkt.setRequestMsg( requestText.c_str() );
    VxGUID requestId;
    requestId.initializeWithNewVxGUID();
    pkt.setRequestId( requestId );
    PktAnnounce * pktAnn = engine.getMyPktAnnounce().makeAnnCopy();
    pktAnn->setHisFriendshipToMe( myFriendshipToHim ); // reversed on purpose
    PktBlobEntry& blobEntry = pkt.getBlobEntry();
    if( !pktAnn->getVxNetIdent()->addToBlob( blobEntry ) )
    {
        LogMsg( LOG_ERROR, "PluginFriendRequest::%s failed addToBlob", __func__ );
        return false;
    }

    blobEntry.resetRead();
    pkt.calcPktLen();
    LogMsg( LOG_VERBOSE, "PluginFriendRequest::%s pkt len %d blob idx %d len %d", __func__, pkt.getPktLength(), blobEntry.getDataIdx(), blobEntry.getBlobLen() );

    delete pktAnn;
    return txPacket( onlineId, sktBase, &pkt );
}