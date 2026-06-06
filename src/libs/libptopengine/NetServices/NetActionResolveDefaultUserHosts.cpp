//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetActionResolveDefaultUserHosts.h"

#include <P2PEngine/P2PEngine.h>
#include <P2PEngine/EngineSettings.h>

//============================================================================
NetActionResolveDefaultUserHosts::NetActionResolveDefaultUserHosts( NetServicesMgr& netServicesMgr )
: NetActionBase( netServicesMgr )
{
}

//============================================================================
void NetActionResolveDefaultUserHosts::doAction( void )
{
    std::string chatUrl;
    m_Engine.getEngineSettings().getChatRoomHostUrl( chatUrl );
    if( !chatUrl.empty() )
    {
        m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeChatRoom, chatUrl );
    }

    std::string groupUrl;
    m_Engine.getEngineSettings().getGroupHostUrl( groupUrl );
    if( !groupUrl.empty() )
    {
        m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeGroup, groupUrl );
    }

    std::string randUrl;
    m_Engine.getEngineSettings().getRandomConnectUrl( randUrl );
    if( !randUrl.empty() )
    {
        m_Engine.getConnectionMgr().applyDefaultHostUrl( eHostTypeRandomConnect, randUrl );
    }
}


