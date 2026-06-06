//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginRandomConnectClient.h"

#include <P2PEngine/P2PEngine.h>
#include <RandConnect/RandConnectMgr.h>

#include <CoreLib/VxDebug.h>

#include <PktLib/PktAdminAvail.h>
#include <PktLib/PktsRandConnect.h>

//============================================================================
PluginRandomConnectClient::PluginRandomConnectClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseHostClient( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeClientRandomConnect );
}
//============================================================================
void PluginRandomConnectClient::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiAnnounceHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginRandomConnectClient::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiJoinHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginRandomConnectClient::fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )
{
    m_HostClientMgr.fromGuiSearchHost( hostType, searchParams, enable );
}

//============================================================================
void PluginRandomConnectClient::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogRandomConnect ) )LogModule( eLogRandomConnect, LOG_DEBUG, "PluginRandomConnectClient got join request" );
}

//============================================================================
void PluginRandomConnectClient::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogRandomConnect ) )LogModule( eLogRandomConnect, LOG_DEBUG, "PluginRandomConnectClient got join reply" );
    m_HostClientMgr.onPktHostJoinReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogRandomConnect ) )LogModule( eLogRandomConnect, LOG_DEBUG, "PluginRandomConnectClient got search reply" );
    m_HostClientMgr.onPktHostSearchReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogRandomConnect ) )LogModule( eLogRandomConnect, LOG_DEBUG, "PluginRandomConnectClient got join offer request" );
}

//============================================================================
void PluginRandomConnectClient::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogRandomConnect ) )LogModule( eLogRandomConnect, LOG_DEBUG, "PluginRandomConnectClient got join offer reply" );
}

//============================================================================
void PluginRandomConnectClient::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteSearchReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostLeaveReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserInfoReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserStatusReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginRandomConnectClient::onPktRandConnectReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktRandConnectReply* pktReply = (PktRandConnectReply*)pktHdr;
    GroupieId groupieId = pktReply->getGroupieId();
    VxGUID sessionId = pktReply->getSessionId();
    m_Engine.getRandConnectMgr().updateRandConnectStatus( groupieId,
                                                           pktReply->getToUserOnlineId(),
                                                           pktReply->getRandAction(),
                                                           sessionId,
                                                           pktReply->getTimeRequestedMs(),
                                                           pktReply->getOfferType() );
}
