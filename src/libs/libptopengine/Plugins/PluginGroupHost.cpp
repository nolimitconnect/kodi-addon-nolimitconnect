//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginGroupHost.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>

#include <NetLib/VxSktBase.h>

#include <PktLib/PktAdminAvail.h>
#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsHostSearch.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif
 

//============================================================================
PluginGroupHost::PluginGroupHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
    : PluginBaseHostService( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeHostGroup );
}

//============================================================================
void PluginGroupHost::pluginStartup( void )
{
    PluginBaseHostService::pluginStartup();
}

//============================================================================
EMembershipState PluginGroupHost::getMembershipState( VxNetIdent* netIdent )
{
    if( eFriendStateIgnore == getPluginPermission() )
    {
        return eMembershipStateJoinDenied;
    }

    return m_HostServerMgr.getMembershipState( netIdent, getHostType() );
}

//============================================================================
void PluginGroupHost::fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing )
{
    if( pluginType != getPluginType() )
    {
        return;
    }

    GroupieId groupieId( m_Engine.getMyOnlineId(), m_Engine.getMyOnlineId(), eHostTypeGroup );
    PktAdminAvail pktAdminAvail;
    pktAdminAvail.setAdminAvailable( adminIsViewing );
    pktAdminAvail.setAdminGroupieId( groupieId );

    VxGUID excludeId;
    broadcastToClients( &pktAdminAvail, excludeId );
}
