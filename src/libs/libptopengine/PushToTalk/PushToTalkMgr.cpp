//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PushToTalkMgr.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

//============================================================================
void PushToTalkMgr::wantPushToTalkCallbacks( PushToTalkCallback* client, bool enable )
{
    if( !client )
    {
        LogMsg( LOG_ERROR, "PushToTalkMgr::wantPushToTalkCallbacks null client" );
        vx_assert( false );
        return;
    }

    if( VxIsAppShuttingDown() )
    {
        return;
    }

    lockClientList();
    for( auto iter = m_PushToTalkClients.begin(); iter != m_PushToTalkClients.end(); ++iter )
    {
        if( *iter == client && !enable )
        {
            m_PushToTalkClients.erase( iter );
            break;
        }
    }

    if( enable )
    {
        m_PushToTalkClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void PushToTalkMgr::pushToTalkStatusChange( VxGUID& onlineId, enum EPushToTalkStatus pushToTalkStatus )
{
    if( VxIsAppShuttingDown() )
    {
        return;
    }

    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "PushToTalkMgr::%s invalid online id", __func__ );
        vx_assert( false );
        return;
    }

    EPushToTalkStatus beforeStatus = getPushToTalkStatus( onlineId );

    if( beforeStatus != pushToTalkStatus )
    {
        if( LogEnabled( eLogVoice ) ) LogModule( eLogVoice, LOG_DEBUG, "PushToTalkMgr::%s from %s to %s", __func__,
                      DescribePushToTalkStatus( beforeStatus ), DescribePushToTalkStatus( pushToTalkStatus ), GetPtoPEngine().describeUser( onlineId ).c_str() );
        setPushToTalkStatus( onlineId, pushToTalkStatus );
        lockClientList();
        for( auto& client : m_PushToTalkClients )
        {
            client->callbackPushToTalkStatus( onlineId, pushToTalkStatus );
        }

        unlockClientList();
    }
}

//============================================================================
void PushToTalkMgr::setPushToTalkStatus( VxGUID& onlineId, EPushToTalkStatus pushToTalkStatus )
{
    if( onlineId.isValid() )
    {
        lockStatusMap();
        m_PushToTalkStatusMap[onlineId] = pushToTalkStatus;
        unlockStatusMap();
    }
}

//============================================================================
EPushToTalkStatus PushToTalkMgr::getPushToTalkStatus( VxGUID& onlineId )
{
    EPushToTalkStatus pushToTalkStatus{ ePushToTalkStatusNotActive };
    lockStatusMap();
    auto iter = m_PushToTalkStatusMap.find( onlineId );
    if( iter != m_PushToTalkStatusMap.end() )
    {
        pushToTalkStatus = iter->second;
    }

    unlockStatusMap();
    return pushToTalkStatus;
}
