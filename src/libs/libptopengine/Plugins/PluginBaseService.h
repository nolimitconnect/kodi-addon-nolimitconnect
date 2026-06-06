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

#include "PluginBase.h"
#include <CoreLib/HostedId.h>

class PluginBaseService : public PluginBase
{
public:
    PluginBaseService( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginBaseService() override = default;

    EHostType                   getHostType( void ) override        { return m_HostType; }
    HostedId&                   getHostedId( void )                 { return m_HostedId; }

    void                        broadcastToClients( VxPktHdr* pktHdr, VxGUID& requesterOnlineId, std::shared_ptr<VxSktBase>& sktBaseRequester, bool includeRequester = true ) override;
    void                        broadcastToClients( VxPktHdr* pktHdr, VxGUID& excludedOnlineId ) override;

protected:
    EConnectReason              getHostAnnounceConnectReason( void );
    EConnectReason              getHostJoinConnectReason( void );
    EConnectReason              getHostSearchConnectReason( void );

    //=== vars ===//
    EHostType                   m_HostType{ eHostTypeUnknown };
    HostedId                    m_HostedId;
};
