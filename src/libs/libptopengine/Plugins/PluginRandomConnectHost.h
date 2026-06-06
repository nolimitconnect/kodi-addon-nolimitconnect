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

#include "PluginBaseHostService.h"

#include <Connections/IConnectRequest.h>

#include <PktLib/PktsHostInvite.h>
#include <CoreLib/VxMutex.h>

class PluginRandomConnectHost : public PluginBaseHostService, public IConnectRequestCallback
{
public:

    PluginRandomConnectHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginRandomConnectHost() override = default;

    EMediaModule			    getMediaModule( void ) override		{ return eMediaModuleRandomConnectHost; }

    void				        pluginStartup( void ) override;

    void				        onPktRandConnectReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

protected:
    /// return true if have use for this connection
    //=== callback overrides ==//
    bool                        onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return true;  };
    void                        onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, ERunTestStatus testStatus, 
                                                        EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    bool                        onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return false; };
    void                        onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, EConnectStatus connectStatus,
                                                      EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};
    bool                        onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return true;  };
    void                        onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};
    void                        onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};
    void                        onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};

    void				        fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing ) override;
};
