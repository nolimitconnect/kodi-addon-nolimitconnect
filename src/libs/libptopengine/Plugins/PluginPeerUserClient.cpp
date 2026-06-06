//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginPeerUserClient.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <P2PEngine/P2PConnectList.h>
#include <BigListLib/BigListInfo.h>

#include <NetLib/VxSktBase.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
PluginPeerUserClient::PluginPeerUserClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseHostClient( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeClientPeerUser );
}

//============================================================================
void PluginPeerUserClient::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiAnnounceHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginPeerUserClient::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiJoinHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginPeerUserClient::fromGuiUnJoinHost( HostedId& adminId )
{
    m_HostClientMgr.fromGuiUnJoinHost( adminId );
}

//============================================================================
void PluginPeerUserClient::fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )
{
    m_HostClientMgr.fromGuiSearchHost( hostType, searchParams, enable );
}

//============================================================================
void PluginPeerUserClient::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginPeerUserClient got join request" );
}

//============================================================================
void PluginPeerUserClient::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginPeerUserClient got join reply" );
    m_HostClientMgr.onPktHostJoinReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginPeerUserClient::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginPeerUserClient got search reply" );
    m_HostClientMgr.onPktHostSearchReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginPeerUserClient::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginPeerUserClient got join offer request" );
}

//============================================================================
void PluginPeerUserClient::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginPeerUserClient got join offer reply" );
}
