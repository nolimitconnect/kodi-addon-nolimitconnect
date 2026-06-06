//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginChatRoomClient.h"
#include "PluginMgr.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

#include <PktLib/PktAdminAvail.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
PluginChatRoomClient::PluginChatRoomClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseMultimedia( engine, pluginMgr, myIdent, pluginType )
, m_HostClientMgr(engine, pluginMgr, myIdent, *this)
{
	setPluginType( ePluginTypeClientChatRoom );
}

//============================================================================
void PluginChatRoomClient::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiAnnounceHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginChatRoomClient::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    m_HostClientMgr.fromGuiJoinHost( adminId, sessionId, ptopUrl );
}

//============================================================================
void PluginChatRoomClient::fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )
{
    m_HostClientMgr.fromGuiSearchHost( hostType, searchParams, enable );
}

//============================================================================
void PluginChatRoomClient::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogChatRoom))LogModule( eLogChatRoom, LOG_DEBUG, "PluginChatRoomClient got join request" );
}

//============================================================================
void PluginChatRoomClient::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogChatRoom ) )LogModule( eLogChatRoom, LOG_DEBUG, "PluginChatRoomClient got join reply" );
    m_HostClientMgr.onPktHostJoinReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogChatRoom ) )LogModule( eLogChatRoom, LOG_DEBUG, "PluginChatRoomClient got search reply" );
    m_HostClientMgr.onPktHostSearchReply( sktBase, pktHdr,  netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogChatRoom ) )LogModule( eLogChatRoom, LOG_DEBUG, "PluginChatRoomClient got join offer request" );
}

//============================================================================
void PluginChatRoomClient::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( LogEnabled( eLogChatRoom ) )LogModule( eLogChatRoom, LOG_DEBUG, "PluginChatRoomClient got join offer reply" );
}

//============================================================================
void PluginChatRoomClient::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteSearchReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    handlePktHostInviteMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostLeaveReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserListMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserInfoReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginChatRoomClient::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostClientMgr.onPktHostUserStatusReply( sktBase, pktHdr, netIdent );
}
