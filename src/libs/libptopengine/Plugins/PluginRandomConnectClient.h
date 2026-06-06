#pragma once
//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseHostClient.h"

class PluginRandomConnectClient : public PluginBaseHostClient
{
public:
    PluginRandomConnectClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginRandomConnectClient() override = default;

    EMediaModule			    getMediaModule( void ) override		{ return eMediaModuleRandomConnectClient; }

    //=== hosting ===//
    void				        fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;
    void				        fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;

    void				        fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable ) override;

    void				        onPktRandConnectReply           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

protected:

    void				        onPktHostJoinReq                ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostJoinReply              ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostSearchReply            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostOfferReq               ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostOfferReply             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostInviteSearchReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostInviteMoreReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostLeaveReply             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostUserListReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostUserListMoreReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostUserInfoReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostUserStatusReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
};
