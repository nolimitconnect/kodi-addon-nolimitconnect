//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginNetServices.h"
#include <P2PEngine/P2PEngine.h>
#include <NetServices/NetServiceHdr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>

//============================================================================
PluginNetServices::PluginNetServices( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseService( engine, pluginMgr, myIdent, pluginType )
, m_NetServicesMgr( engine.getNetServicesMgr() )
{
	if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "PluginNetServices::PluginNetServices" );
	setPluginType( ePluginTypeNetServices );
}

//============================================================================
void PluginNetServices::testIsMyPortOpen( void )
{
	m_NetServicesMgr.addNetActionToQueue( eNetActionIsPortOpen );
}
