//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <NetLib/VxPeerMgr.h>
#include <PktLib/PktTypes.h>

namespace
{
    bool IsAssetTransferPacketType( uint8_t pktType )
    {
        return pktType == PKT_TYPE_ASSET_SEND_REQ ||
            pktType == PKT_TYPE_ASSET_SEND_REPLY ||
            pktType == PKT_TYPE_ASSET_CHUNK_REQ ||
            pktType == PKT_TYPE_ASSET_CHUNK_REPLY ||
            pktType == PKT_TYPE_ASSET_SEND_COMPLETE_REQ ||
            pktType == PKT_TYPE_ASSET_SEND_COMPLETE_REPLY ||
            pktType == PKT_TYPE_ASSET_XFER_ERR;
    }
}

//============================================================================
bool PluginMgr::pluginApiSktConnectTo(		EPluginType			pluginType,	// plugin id
											VxNetIdent*	        netIdent,		// identity of contact to connect to
											int					pvUserData,		// plugin defined data
											std::shared_ptr<VxSktBase>&		    ppoRetSkt, 		// returned Socket
											EConnectReason		connectReason )
{
	std::shared_ptr<VxSktBase> sktBase( nullptr );
	ppoRetSkt.reset();
	bool newConnection = false;
	if( true == m_Engine.connectToContact( netIdent->getConnectInfo(), sktBase, newConnection, connectReason ) )
	{
		ppoRetSkt = sktBase;
		return true;
	}

	return false;
}

//============================================================================
//! close socket connection
void PluginMgr::pluginApiSktClose( ESktCloseReason closeReason, std::shared_ptr<VxSktBase>& sktBase )
{
	sktBase->closeSkt(closeReason);
}

//============================================================================
//! close socket immediate.. don't bother to flush buffer
void PluginMgr::pluginApiSktCloseNow( ESktCloseReason closeReason, std::shared_ptr<VxSktBase>& sktBase )
{
	sktBase->closeSkt(closeReason,  false);
}

//============================================================================
bool PluginMgr::pluginApiTxPacket(  EPluginType			pluginType,
                                    const VxGUID&       onlineId,
                                    std::shared_ptr<VxSktBase>&          sktBase,
                                    VxPktHdr*           pktHdr,
                                    EPluginType         overridePlugin )
{
    // when sending packets they are typically from plugin to the same remote plugin
    // for host/client we convert host to client and client to hot
    EPluginType hostClientType = ePluginTypeInvalid;
    switch( pluginType )
    {
    case ePluginTypeCamServer:
        hostClientType = ePluginTypeCamClient;
        break;
    case ePluginTypeCamClient:
        hostClientType = ePluginTypeCamServer;
        break;
    case ePluginTypeClientChatRoom:
        hostClientType = ePluginTypeHostChatRoom;
        break;
    case ePluginTypeFileShareServer:
        hostClientType = ePluginTypeFileShareClient;
        break;
    case ePluginTypeFileShareClient:
        hostClientType = ePluginTypeFileShareServer;
        break;
    case ePluginTypeHostChatRoom:
        hostClientType = ePluginTypeClientChatRoom;
        break;
    case ePluginTypeClientConnectTest:
        hostClientType = ePluginTypeHostConnectTest;
        break;
    case ePluginTypeHostConnectTest:
        hostClientType = ePluginTypeClientConnectTest;
        break;
    case ePluginTypeClientGroup:
        hostClientType = ePluginTypeHostGroup;
        break;
    case ePluginTypeHostGroup:
        hostClientType = ePluginTypeClientGroup;
        break;
    case ePluginTypeClientPeerUser:
        hostClientType = ePluginTypeHostPeerUser;
        break;
    case ePluginTypeHostPeerUser:
        hostClientType = ePluginTypeClientPeerUser;
        break;
    case ePluginTypeClientRandomConnect:
        hostClientType = ePluginTypeHostRandomConnect;
        break;
    case ePluginTypeHostRandomConnect:
        hostClientType = ePluginTypeClientRandomConnect;
        break;
    case ePluginTypeAboutMePageServer:
        hostClientType = ePluginTypeAboutMePageClient;
        break;
    case ePluginTypeAboutMePageClient:
        hostClientType = ePluginTypeAboutMePageServer;
        break;
    case ePluginTypeStoryboardServer:
        hostClientType = ePluginTypeStoryboardClient;
        break;
    case ePluginTypeStoryboardClient:
        hostClientType = ePluginTypeStoryboardServer;
        break;

    default:
        break;
    }

    bool keepClientPluginForAssetXfer = overridePlugin == ePluginTypeInvalid &&
        IsAssetTransferPacketType( pktHdr->getPktType() ) &&
        ( pluginType == ePluginTypeClientGroup || pluginType == ePluginTypeClientRandomConnect );

    if( overridePlugin != ePluginTypeInvalid )
    {
        pktHdr->setPluginNum( ( uint8_t )overridePlugin );
    }
    else if( keepClientPluginForAssetXfer )
    {
        pktHdr->setPluginNum( ( uint8_t )pluginType );
    }
    else if( hostClientType != ePluginTypeInvalid )
    {
        pktHdr->setPluginNum( ( uint8_t )hostClientType );
    }
    else
    {
        pktHdr->setPluginNum( ( uint8_t )pluginType );
    }

    pktHdr->setSrcOnlineId( m_Engine.getMyOnlineId() );

    if( onlineId == m_Engine.getMyOnlineId() && sktBase->isLoopbackSocket() )
    {
        // destination is ourself
        if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "pluginApiTxPacket type %d len %d loopback %s", pktHdr->getPktType(), pktHdr->getPktLength(), DescribePluginType( pluginType ) );
        pktHdr->setDestOnlineId( onlineId );
        return sktBase->txPacketWithDestId( pktHdr ) == 0;
    }

    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "pluginApiTxPacket type %d len %d plugin %s override %s final %s to %s", pktHdr->getPktType(), pktHdr->getPktLength(), 
        DescribePluginType( pluginType ), DescribePluginType( overridePlugin ), DescribePluginType( (EPluginType)pktHdr->getPluginNum() ), sktBase->getRemoteIpAddress() );
    return m_Engine.getPeerMgr().txPacket( sktBase, onlineId, pktHdr );
}

