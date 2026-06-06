//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MemberActiveMgr.h"

#include "MemberActiveCallback.h"
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListMgr.h>

#include <CoreLib/VxDebug.h>

#include <algorithm>

//============================================================================
void MemberActiveMgr::memberActiveStartup( void )
{
    GetPtoPEngine().getConnectIdListMgr().wantConnectIdListCallback( this, true );
}

//============================================================================
// return true if new active member
bool MemberActiveMgr::updateMemberActive( GroupieId& groupieId, bool memberActive )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "MemberActiveMgr::updateSktIdent invalid skt id" );
        return false;
    }

    bool wasUpdated = false;
    lockMemberList();

    bool found{ false };
    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); ++iter )
    {
        if( *iter == groupieId )
        {
            found = true;
            if( !memberActive )
            {
                iter = m_MemberList.erase( iter );
                wasUpdated = true;
            }

            break;
        }
    }

    if( memberActive && !found )
    {
        m_MemberList.emplace_back( groupieId );
        wasUpdated = true;
    }

    unlockMemberList();

    if( wasUpdated )
    {
        if( groupieId.getUserOnlineId() == GetPtoPEngine().getMyOnlineId() )
        {
            if( memberActive )
            {
                LogModule( eLogHostJoin, LOG_VERBOSE, "MemberActiveMgr myself member active with host %s", GetPtoPEngine().describeHostedId( groupieId.getHostedId() ).c_str() );
            }
            else
            {
                LogModule( eLogHostJoin, LOG_VERBOSE, "MemberActiveMgr myself member NOT active with host %s", GetPtoPEngine().describeHostedId( groupieId.getHostedId() ).c_str() );
            }
        }
        else
        {
            if( memberActive )
            {
                LogModule( eLogHostJoin, LOG_VERBOSE, "MemberActiveMgr member %s active with my host %s", GetPtoPEngine().describeUser( groupieId.getUserOnlineId() ).c_str(), DescribeHostType( groupieId.getHostType() ) );
            }
            else
            {
                LogModule( eLogHostJoin, LOG_VERBOSE, "MemberActiveMgr member %s NOT active with my host %s", GetPtoPEngine().describeUser( groupieId.getUserOnlineId() ).c_str(), DescribeHostType( groupieId.getHostType() ) );
            }
        }

        announceMemberActive( groupieId, memberActive );
    }

    return memberActive && !found && wasUpdated;
}

//============================================================================
bool MemberActiveMgr::isMemberActive( GroupieId& groupieId )
{
    lockMemberList();
    bool isActive = std::find( m_MemberList.begin(), m_MemberList.end(), groupieId ) != m_MemberList.end();
    unlockMemberList();

    return isActive;
}

//============================================================================
bool MemberActiveMgr::isActiveMemberOfAny( VxGUID& onlineId )
{
    bool isActive{ false };
    lockMemberList();
    for( auto groupieId : m_MemberList )
    {
        if( groupieId.getUserOnlineId() == onlineId || groupieId.getHostOnlineId() == onlineId )
        {
            isActive = true;
            break;
        }
    }

    unlockMemberList();

    return isActive;
}

//============================================================================
void MemberActiveMgr::announceMemberActive( GroupieId& groupieId, bool memberActive )
{
    LogModule( eLogMembership, LOG_INFO, "MemberActiveMgr::announceMemberActive %d %s", memberActive, GetPtoPEngine().describeGroupieId( groupieId ).c_str() );
    VxNetIdent* netIdent = GetPtoPEngine().getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent && memberActive != netIdent->getIsJoined( groupieId.getHostType() ) )
    {
        netIdent->setIsJoined( groupieId.getHostType(), memberActive );
        GetPtoPEngine().toGuiContactAnythingChange( netIdent );
    }

    lockClientList();

    for( auto& client : m_MemberClients )
    {
        client->callbackMemberActive( groupieId, memberActive );
    }

    unlockClientList();
}

//============================================================================
void MemberActiveMgr::wantMemberActiveCallbacks( MemberActiveCallback* client, bool enable )
{
    lockClientList();

    bool found{ false };
    for( auto iter = m_MemberClients.begin(); iter != m_MemberClients.end(); ++iter )
    {
        if( *iter == client )
        {
            found = true;
            if( !enable )
            {
                m_MemberClients.erase( iter );
            }
            else
            {
                LogMsg( LOG_ERROR, "MemberActiveMgr::wantMemberActiveCallbacks ignored because already in list" );
            }

            break;
        }
    }
    
    if( !found && enable )
    {
        m_MemberClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void MemberActiveMgr::callbackConnectionStatusChange( ConnectId& connectId, bool isOnline )
{
    if( isOnline )
    {
        return;
    }
    
    VxGUID onlineId = connectId.getUserOnlineId();
    EHostType hostType = connectId.getHostType();
    std::vector<GroupieId> offlineMemberList;
    lockMemberList();

    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); )
    {
        if( *(iter) == connectId.getGroupieId() )
        {
            offlineMemberList.emplace_back( *iter );
            iter = m_MemberList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockMemberList();

    for( auto& groupieId : offlineMemberList )
    {
        announceMemberActive( groupieId, false );
    }
}

//============================================================================
bool MemberActiveMgr::isUserJoinedToRelayHost( VxGUID& memberOnlineId, VxGUID& hostOnlineId )
{
    bool isJoined{ false };
    lockMemberList();
    for( auto& groupieId : m_MemberList )
    {
        if( groupieId.getUserOnlineId() == memberOnlineId && groupieId.getHostOnlineId() == hostOnlineId )
        {
            if( IsHostARelayForUsers( groupieId.getHostType() ) )
            {
                isJoined = true;
                break;
            }
        }
    }

    unlockMemberList();

    return isJoined;
}