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

#include "PluginBaseNetworkService.h"
#include "HostServerMgr.h"

class PluginNetworkHost : public PluginBaseNetworkService
{
public:
    PluginNetworkHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginNetworkHost() override = default;

    EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

    void                        updateHostSearchList( EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;
    void				        fromGuiSendAnnouncedList( EHostType hostType, VxGUID& sessionId ) override;
    void				        fromGuiListAction( EListAction listAction ) override;

    int                         getAnnouncedHostCount( EHostType hostType );

protected:
    void				        onPktHostInviteAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
};
