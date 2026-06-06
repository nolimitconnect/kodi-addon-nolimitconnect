//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginGroupClient.h"
#include "PluginMgr.h"
#include "P2PSession.h"
#include "RxSession.h"
#include "TxSession.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

#include <PktLib/PktAdminAvail.h>

//============================================================================
PluginGroupClient::PluginGroupClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseHostClient( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeClientGroup );
}

//============================================================================
void PluginGroupClient::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiAnnounceHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginGroupClient::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiJoinHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginGroupClient::fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )
{
    m_HostClientMgr.fromGuiSearchHost( hostType, searchParams, enable );
}

//============================================================================
void PluginGroupClient::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogGroup ) )LogModule( eLogGroup, LOG_DEBUG, "PluginGroupClient got join request" );
}

//============================================================================
void PluginGroupClient::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogGroup ) )LogModule( eLogGroup, LOG_DEBUG, "PluginGroupClient got join reply" );
    m_HostClientMgr.onPktHostJoinReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogGroup ) )LogModule( eLogGroup, LOG_DEBUG, "PluginGroupClient got search reply" );
    m_HostClientMgr.onPktHostSearchReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogGroup ) )LogModule( eLogGroup, LOG_DEBUG, "PluginGroupClient got join offer request" );
}

//============================================================================
void PluginGroupClient::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogGroup ) )LogModule( eLogGroup, LOG_ERROR, "PluginGroupClient got join offer reply" );
}

//============================================================================
void PluginGroupClient::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteSearchReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostLeaveReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserInfoReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginGroupClient::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserStatusReply( sktBase, pktHdr, netIdent );
}
