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

#include <NetServices/NetServicesMgr.h>

class PluginConnectionTestClient : public PluginBaseNetworkService
{
public:
    PluginConnectionTestClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginConnectionTestClient() override = default;
    PluginConnectionTestClient() = delete; // don't allow default constructor
    PluginConnectionTestClient( const PluginConnectionTestClient& ) = delete; // don't allow copy constructor

    EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

    virtual int32_t				handlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr ) override;

    void						testIsMyPortOpen( void );

protected:
    int32_t						internalHandlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr );

    virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override	{};
    virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override	{};
    virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override {};

    void						onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override {};

    //=== vars ===//
    NetServicesMgr&				m_NetServicesMgr;  
};
