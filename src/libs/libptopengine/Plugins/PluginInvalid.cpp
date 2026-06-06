//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "PluginInvalid.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PluginInvalid::PluginInvalid( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
{
	m_ePluginType = ePluginTypeInvalid;
}

//============================================================================
PluginInvalid::~PluginInvalid()
{
}

//============================================================================
//=== overrides ===//
//============================================================================

//============================================================================
//! handle app state change
void PluginInvalid::onAppStateChange( EAppState eAppState )
{
	LogMsg( LOG_INFO, "PluginInvalid::onAppStateChange %d", eAppState );
	switch( eAppState)
	{
	case eAppStateStartup:	// app has started
		break;
	case eAppStateShutdown:	// app shutdown
		break;
	case eAppStateSleep:	// app sleep
		break;
	case eAppStateWake:		// app wake
		break;
	case eAppStatePause:	// pause app
		break;
	case eAppStateResume:	// resume
		break;
	default:
		LogMsg( LOG_ERROR, "PluginInvalid::onAppStateChange unknown state");
	}
}

//============================================================================
//=== plugin session ===//
//============================================================================
//============================================================================
//! called to start service or session with remote friend
bool PluginInvalid::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
    return false;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginInvalid::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{

}
//============================================================================
//! return true if is plugin session
bool PluginInvalid::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return true;
}

//============================================================================
EPluginAccess PluginInvalid::canAcceptNewSession( VxNetIdent* netIdent )
{
	return ePluginAccessDisabled;
}

//============================================================================
//=== methods ===//
//============================================================================



