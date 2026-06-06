#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseHostClient.h"

#include <Connections/IConnectRequest.h>

#include <CoreLib/VxMutex.h>

#include <PktLib/PktsHostInvite.h>

/**
* client side avatar and search and group management services
*/
class PluginPeerUserClient : public PluginBaseHostClient
{
public:

    PluginPeerUserClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginPeerUserClient() override = default;

    EMediaModule			    getMediaModule( void ) override { return eMediaModulePeerUserClient; }

    //=== hosting ===//
    virtual void				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;
    virtual void				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;
    virtual void				fromGuiUnJoinHost( HostedId& adminId ) override;
    virtual void				fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable ) override;

protected:

    virtual void				onPktHostJoinReq                ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostJoinReply              ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostSearchReply            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostOfferReq               ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostOfferReply             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    //=== vars ===//
};
