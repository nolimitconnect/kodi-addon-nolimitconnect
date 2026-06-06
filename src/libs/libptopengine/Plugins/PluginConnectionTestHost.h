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

#include <Connections/IConnectRequest.h>

class NetServicesMgr;

class PluginConnectionTestHost : public PluginBaseNetworkService
{
public:

    PluginConnectionTestHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginConnectionTestHost() override = default;

    EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

    void                        testIsMyPortOpen( void );
    int32_t                       handlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr ) override;
    int32_t                       internalHandlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr );

protected:
    NetServicesMgr&				m_NetServicesMgr;
};
