#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBase.h"

//! implements Invalid plugin to catch errors
class PluginInvalid : public PluginBase
{
public:
	PluginInvalid( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginInvalid();

	EMediaModule				getMediaModule( void ) override { return eMediaModuleInvalid;  }

	//! handle app state change
	virtual void				onAppStateChange( EAppState eAppState );
	//! called to start service or session with remote friend
    virtual bool                fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	//! called to stop service or session with remote friend
    virtual void				fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	//! return true if is plugin session
    virtual bool				fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
	//! can accept a new connection/session
    virtual EPluginAccess		canAcceptNewSession( VxNetIdent* netIdent ) override;

    virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override {};
	//! called when new better connection from user
    virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override{};
    virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override {};

	void						onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override {};

	//=== methods ===//
	bool						fromGuiStartPluginSession( PluginSessionBase* poOffer ) override { return false; };

};
