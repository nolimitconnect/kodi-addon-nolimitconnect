//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginPeerUserHost.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsHostSearch.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif
 
//============================================================================
PluginPeerUserHost::PluginPeerUserHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
    : PluginBaseHostService( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeHostPeerUser );
}

//============================================================================
void PluginPeerUserHost::pluginStartup( void )
{
    PluginBaseHostService::pluginStartup();
}
