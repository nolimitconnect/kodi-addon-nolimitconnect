//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SessionMgrBase.h"
#include "PluginBase.h"
#include "PluginMgr.h"
#include <GuiInterface/IToGui.h>

//============================================================================
SessionMgrBase::SessionMgrBase( P2PEngine& engine, PluginBase& plugin, PluginMgr& pluginMgr )
: m_Engine( engine )
, m_Plugin( plugin )
, m_PluginMgr( pluginMgr )
{
}

//============================================================================
IToGui&	SessionMgrBase::getToGui( void )
{ 
    return IToGui::getIToGui(); 
}

//============================================================================
EPluginType  SessionMgrBase::getPluginType( void )                       
{ 
    return m_Plugin.getPluginType(); 
}

//============================================================================
bool SessionMgrBase::isPluginSingleSession( void )
{
	bool isSingleSessionPlugin = false;
	switch( m_Plugin.getPluginType() )
	{
	case ePluginTypeVoicePhone:
	case ePluginTypeVideoChat:
	case ePluginTypeTruthOrDare:
	case ePluginTypeMessenger:
	case ePluginTypeCamServer:
	case ePluginTypeAboutMePageClient:
	case ePluginTypeStoryboardClient:
		isSingleSessionPlugin = true;
		break;
	case ePluginTypeAboutMePageServer:
	case ePluginTypeStoryboardServer: 
	case ePluginTypeFileShareServer:
	case ePluginTypePersonFileXfer:
	default:
		break;
	}

	return isSingleSessionPlugin;
}
