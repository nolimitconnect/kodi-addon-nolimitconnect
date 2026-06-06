//============================================================================
// Copyright (C) 2020 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostClientSearchMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginBase.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostJoin.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
HostClientSearchMgr::HostClientSearchMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase )
: HostBaseMgr(engine, pluginMgr, myIdent, pluginBase)
{
}
