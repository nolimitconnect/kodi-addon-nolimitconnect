//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "UserJoinMgr.h"
#include "UserJoinInfo.h"
#include "UserJoinInfoDb.h"
#include "UserJoinCallbackInterface.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <BaseInfo/BaseSessionInfo.h>
#include <Membership/MemberActiveMgr.h>
#include <Plugins/PluginMgr.h>
#include <Plugins/PluginBaseHostClient.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxPtopUrl.h>
#include <NetLib/VxSktBase.h>

namespace
{
    const bool CAN_JOIN_MULTIPLE_HOSTS = true;
}

//============================================================================
UserJoinMgr::UserJoinMgr( P2PEngine& engine, const char* dbName, const char* dbJoinedLastName )
: m_Engine( engine )
, m_UserJoinInfoDb( engine, *this, dbName )
, m_UserJoinedLastDb( engine, *this, dbJoinedLastName )
{
     LogMsg( LOG_VERBOSE, "UserJoinMgr::UserJoinMgr" );
}

//============================================================================
bool UserJoinMgr::deleteDatabase( void )
{
    bool result = m_UserJoinInfoDb.deleteDatabase();
    result &= m_UserJoinedLastDb.deleteDatabase();
    return result;
}

//============================================================================
void UserJoinMgr::fromGuiUserLoggedOn( void )
{
    LogModule( eLogStartup, LOG_VERBOSE, "UserJoinMgr::fromGuiUserLoggedOn start" );
    // dont call HostBaseMgr::fromGuiUserLoggedOn because we never generate sha hash for thumbnails
    if( !m_Initialized )
    {
        m_Initialized = true;
        // user specific directory should be set
        std::string dbFileName = VxGetSettingsDirectory();
        dbFileName += m_UserJoinInfoDb.getDatabaseName(); 

        std::string dbLastJoinedFileName = VxGetSettingsDirectory();
        dbLastJoinedFileName += m_UserJoinedLastDb.getDatabaseName();

        lockResources();
        m_UserJoinInfoDb.dbShutdown();
        m_UserJoinInfoDb.dbStartup( USER_JOIN_DB_VERSION, dbFileName );

        m_UserJoinedLastDb.dbShutdown();
        m_UserJoinedLastDb.dbStartup( USER_JOINED_LAST_DB_VERSION, dbLastJoinedFileName );

        clearUserJoinInfoList();
        m_UserJoinInfoDb.getAllUserJoins( m_UserJoinInfoList );
        for( auto iter = m_UserJoinInfoList.begin();  iter != m_UserJoinInfoList.end(); ++iter )
        {
            UserJoinInfo* userJoinInfo = iter->second;
            if( !userJoinInfo->getNetIdent().isValidNetIdent() )
            {
                VxNetIdent* netIdent = m_Engine.getBigListMgr().findBigListInfo( userJoinInfo->getOnlineId() );
                if( netIdent )
                {
                    userJoinInfo->setNetIdent( netIdent );
                }
                else if( userJoinInfo->getOnlineId() == m_Engine.getMyOnlineId() && m_Engine.getMyNetIdent()->isValidNetIdent() )
                {
                    // is myself
                    userJoinInfo->setNetIdent( m_Engine.getMyNetIdent() );
                }
            }
        }

        GroupieId lastJoinedGroupieId;
        if( getLastJoinedGroupieIdFromDb( eHostTypeChatRoom, lastJoinedGroupieId ) )
        {
            if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s last joined host %s", __func__,
                lastJoinedGroupieId.describeGroupieId().c_str() );
            setLastJoined( lastJoinedGroupieId );
        }

        if( getLastJoinedGroupieIdFromDb( eHostTypeGroup, lastJoinedGroupieId ) )
        {
            if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s last joined host %s", __func__,
                lastJoinedGroupieId.describeGroupieId().c_str() );
            setLastJoined( lastJoinedGroupieId );
        }

        if( getLastJoinedGroupieIdFromDb( eHostTypeRandomConnect, lastJoinedGroupieId ) )
        {
            if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s last joined host %s", __func__,
                lastJoinedGroupieId.describeGroupieId().c_str() );
            setLastJoined( lastJoinedGroupieId );
        }

        EHostType lastHostType{ eHostTypeUnknown };
        if( m_UserJoinedLastDb.getJoinedLastHostType( lastHostType ) )
        {
            setLastJoinedHostType( lastHostType );
        }

        m_UserJoinListInitialized = true;
        unlockResources();
    }

    LogModule( eLogStartup, LOG_VERBOSE, "UserJoinMgr::fromGuiUserLoggedOn done" );
}

//============================================================================
bool UserJoinMgr::fromGuiLeaveHost( HostedId& adminId, bool unjoinAlso )
{
    GroupieId groupieId( m_Engine.getMyOnlineId(), adminId );
    EHostType hostType = adminId.getHostType();
    PluginBase* plugin = m_Engine.getPluginMgr().findHostClientPlugin( hostType );
    if( plugin )
    {
        if( unjoinAlso )
        {
            plugin->fromGuiUnJoinHost( adminId );
        }
        else
        {
            plugin->fromGuiLeaveHost( adminId );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "Plugin not found for host %d", hostType );
        vx_assert( false );
        return false;
    }

    lockResources();
    UserJoinInfo* userJoinInfo = findUserJoinInfo( groupieId );
    EJoinState joinState = userJoinInfo->getJoinState();
    if( eJoinStateJoinIsGranted == joinState )
    {
        userJoinInfo->setJoinState( unjoinAlso ? eJoinStateJoinLeaveHost : eJoinStateJoinWasGranted );
        announceUserJoinUpdated( userJoinInfo );
    }

    unlockResources();

    return true;
}

//============================================================================
bool UserJoinMgr::fromGuiUnJoinHost( HostedId& adminId )
{
    return fromGuiLeaveHost( adminId, true );
}

//============================================================================
void UserJoinMgr::callbackOnlineStatusChange( VxGUID& onlineId, bool isOnline )
{
    if( isOnline )
    {
        return;
    }

    lockResources();

    for( auto& joinPair : m_UserJoinInfoList )
    {
        if( const_cast<GroupieId&>(joinPair.first).getHostOnlineId() == onlineId )
        {
            UserJoinInfo* userJoinInfo = joinPair.second;
            EJoinState joinState = userJoinInfo->getJoinState();
            if( eJoinStateJoinIsGranted == joinState )
            {
                userJoinInfo->setJoinState( eJoinStateJoinWasGranted );
                announceUserJoinUpdated( userJoinInfo );
            }
        }
    }

    unlockResources();
}

//============================================================================
void UserJoinMgr::addUserJoinMgrClient( UserJoinCallbackInterface * client, bool enable )
{
    lockClientList();
    if( enable )
    {
        m_UserJoinClients.emplace_back( client );
    }
    else
    {
        std::vector<UserJoinCallbackInterface *>::iterator iter;
        for( iter = m_UserJoinClients.begin(); iter != m_UserJoinClients.end(); ++iter )
        {
            if( *iter == client )
            {
                m_UserJoinClients.erase( iter );
                break;
            }
        }
    }

    unlockClientList();
}

//============================================================================
void UserJoinMgr::announceUserJoinRequested( UserJoinInfo* userJoinInfo )
{
    if( userJoinInfo )
    {
        updateUserIsJoined( userJoinInfo );

        if(LogEnabled(eLogHostJoin))LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s state %s %s", __func__,
                      DescribeJoinState( userJoinInfo->getJoinState() ), m_Engine.describeGroupieId( userJoinInfo->getGroupieId() ).c_str() );

        lockClientList();
        for( auto client : m_UserJoinClients )
        {
            client->callbackUserJoinAdded( userJoinInfo );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::announceUserJoinRequested dynamic_cast failed" );
    }
}

//============================================================================
void UserJoinMgr::announceUserJoinUpdated( UserJoinInfo * userJoinInfo )
{
    if( userJoinInfo )
    {
        updateUserIsJoined( userJoinInfo );

        EJoinState joinState = userJoinInfo->getJoinState();
        if( eJoinStateJoinIsGranted == joinState )
        {
            // make sure member active is updated before the join update happens so that member active can be checked when the join update occurs
            m_Engine.getMemberActiveMgr().updateMemberActive( userJoinInfo->getGroupieId(), true );
        }

        if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s state %s %s", __func__, DescribeJoinState( joinState ),
                m_Engine.describeGroupieId( userJoinInfo->getGroupieId() ).c_str() );

        lockClientList();
        for( auto client : m_UserJoinClients )
        {
            client->callbackUserJoinUpdated( userJoinInfo );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::%s null", __func__ );
    }
}

//============================================================================
void UserJoinMgr::announceUserUnJoin( UserJoinInfo* userJoinInfo )
{
    if( userJoinInfo )
    {
        updateUserIsJoined( userJoinInfo );

        if(LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s state %s %s", __func__, DescribeJoinState( userJoinInfo->getJoinState() ),
                userJoinInfo->getGroupieId().describeGroupieId().c_str() );
        lockClientList();
        for( auto client : m_UserJoinClients )
        {
            client->callbackUserUnJoin( userJoinInfo );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::announceUserUnJoin null" );
    }
}

//============================================================================
void UserJoinMgr::announceUserJoinRemoved( GroupieId& groupieId )
{    
    removeFromDatabase( groupieId, false );
    LogMsg( LOG_VERBOSE, "UserJoinMgr::announceUserJoinRemoved %s", groupieId.describeGroupieId().c_str() );
	lockClientList();
	for( auto& client : m_UserJoinClients )
	{
		client->callbackUserJoinRemoved( groupieId );
	}

	unlockClientList();
}

//============================================================================
void UserJoinMgr::announceUserJoinAHostStatus( EHostType hostType, VxGUID& sessionId, EConnectStatus connectStatus )
{
	lockClientList();
	for( auto& client : m_UserJoinClients )
	{
		client->callbackUserJoinAHostStatus( hostType, sessionId, connectStatus );
	}

	unlockClientList();
}

//============================================================================
void UserJoinMgr::clearUserJoinInfoList( void )
{
    for( auto iter = m_UserJoinInfoList.begin(); iter != m_UserJoinInfoList.end(); ++iter )
    {
        delete iter->second;
    }

    m_UserJoinInfoList.clear();
}

//============================================================================
void UserJoinMgr::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::onUserJoinedHost invalid groupieId" );
    }

    bool updatedHostIdentNeeded = false;
    bool wasAdded = false;
    lockResources();
    UserJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( !joinInfo )
    {
        joinInfo = new UserJoinInfo();
        joinInfo->setGroupieId( groupieId );
        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        wasAdded = true;
        if( netIdent->getMyOnlineId() == groupieId.getHostOnlineId() )
        {
            if( !netIdent->getIsJoined( groupieId.getHostType() ) )
            {
                // mark is joined so will have guest privelege if applicable
                netIdent->setIsJoined( groupieId.getHostType(), true );
                updatedHostIdentNeeded = true;
            }
        }
        else
        {
            // joined a different host

        }
    }

    joinInfo->setNetIdent( netIdent );
    int64_t timeNowMs = GetTimeStampMs();
    joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
    joinInfo->setJoinState( eJoinStateJoinIsGranted );

    joinInfo->setHostUrl( netIdent->getMyOnlineUrl() );

    joinInfo->setConnectionId( sktBase->getSocketId() );
    joinInfo->setSessionId( sessionInfo.getSessionId() );

    joinInfo->setInfoModifiedTime( timeNowMs );
    joinInfo->setLastConnectTime( timeNowMs );
    joinInfo->setLastJoinTime( timeNowMs );
    if( wasAdded )
    {
        m_UserJoinInfoList[groupieId] = joinInfo;
    }

    saveToDatabase( joinInfo, true );

    unlockResources();

    if( updatedHostIdentNeeded )
    {
        m_Engine.toGuiContactAnythingChange( netIdent );
    }

    if( groupieId.getHostOnlineId() != m_Engine.getMyOnlineId()
            && groupieId.getUserOnlineId() == m_Engine.getMyOnlineId())
    {
        // we joined a host as a client but we can only join one of each host type
        GroupieId prevLastJoined = getLastJoined( groupieId.getHostType() );

        if( prevLastJoined.isValid() && prevLastJoined != groupieId )
        {
            if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "UserJoinMgr::%s leaving previous host %s", __func__, 
                prevLastJoined.describeGroupieId().c_str() );

            m_Engine.getPluginMgr().leavePreviousHost( prevLastJoined );
            if( prevLastJoined.getHostOnlineId() != groupieId.getHostOnlineId() )
            {
                m_Engine.getConnectIdListMgr().disconnectIfIsOnlyUser( prevLastJoined );
            }
        }
        else
        {
            m_Engine.getConnectIdListMgr().userJoinedHost( sktBase, groupieId );
        }

        setLastJoined( groupieId );
    }

    m_Engine.getThumbMgr().queryThumbIfNeeded( sktBase, netIdent, sessionInfo.getHostPluginType() );

    if( wasAdded )
    {
        announceUserJoinRequested( joinInfo );
    }
    else
    {
        announceUserJoinUpdated( joinInfo );
    } 
}

//============================================================================
void UserJoinMgr::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    bool wasAdded = false;
    lockResources();
    UserJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( !joinInfo )
    {
        joinInfo = new UserJoinInfo();
        joinInfo->setGroupieId( groupieId );
        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        wasAdded = true;
    }

    joinInfo->setNetIdent( netIdent );
    int64_t timeNowMs = GetTimeStampMs();
    joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
    joinInfo->setJoinState( eJoinStateJoinLeaveHost );
    joinInfo->setHostUrl( netIdent->getMyOnlineUrl() );

    joinInfo->setConnectionId( sktBase->getSocketId() );
    joinInfo->setSessionId( sessionInfo.getSessionId() );

    joinInfo->setInfoModifiedTime( timeNowMs );
    joinInfo->setLastConnectTime( timeNowMs );
    joinInfo->setLastJoinTime( timeNowMs );
    if( wasAdded )
    {
        m_UserJoinInfoList[groupieId] = joinInfo;
    }

    saveToDatabase( joinInfo, true );

    unlockResources();

    if( wasAdded )
    {
        announceUserJoinRequested( joinInfo );
    }
    else
    {
        announceUserJoinUpdated( joinInfo );
    }
}

//============================================================================
void UserJoinMgr::onUserLeftHost( GroupieId& groupieId )
{
    changeJoinState( groupieId, eJoinStateJoinLeaveHost );
}

//============================================================================
void UserJoinMgr::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    lockResources();
    UserJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostPluginType() ) );
        joinInfo->setJoinState( eJoinStateJoinLeaveHost );
        joinInfo->setHostUrl( netIdent->getMyOnlineUrl() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( 0 );


        saveToDatabase( joinInfo, true );

        unlockResources();

        announceUserUnJoin( joinInfo );
    }
}

//============================================================================
UserJoinInfo* UserJoinMgr::findUserJoinInfo( GroupieId& groupieId )
{
    auto iter = m_UserJoinInfoList.find( groupieId );
    if( iter != m_UserJoinInfoList.end() )
    {
        return iter->second;
    }

    return nullptr;
}

//============================================================================
bool UserJoinMgr::saveToDatabase( UserJoinInfo* joinInfo, bool isLocked )
{
    bool result{ false };
    if( joinInfo->getOnlineId() == m_Engine.getMyOnlineId() )
    {
        // do not add ourself to database. If we joined then we are the admin
        // and we may join another host at the same time
        return true;
    }

    if( !isLocked )
    {
        lockResources();
    }

    VxGUID onlineId;
    VxPtopUrl ptopUrl( joinInfo->getHostUrl() );
    if( ptopUrl.isValid() )
    {
        onlineId = ptopUrl.getOnlineId();
    }

    if( onlineId.isValid() )
    {
        result = m_UserJoinInfoDb.addUserJoin( joinInfo );

        result &= m_UserJoinedLastDb.setJoinedLast( joinInfo->getHostType(), onlineId, joinInfo->getLastJoinTime(), joinInfo->getHostUrl() );

        result &= m_UserJoinedLastDb.setJoinedLastHostType( joinInfo->getHostType() );
    }

    if( !isLocked )
    {
        unlockResources();
    }

    return result;
}

//============================================================================
void UserJoinMgr::removeFromDatabase( GroupieId& groupieId, bool resourcesLocked )
{
    if( !resourcesLocked )
    {
        lockResources();
    }

    m_UserJoinInfoDb.removeUserJoin( groupieId );
    if( !resourcesLocked )
    {
        unlockResources();
    }
}

//============================================================================
bool UserJoinMgr::saveToJoinedLastDatabase( UserJoinInfo* joinInfo, bool isLocked )
{
    if( joinInfo->getOnlineId() == m_Engine.getMyOnlineId() )
    {
        // do not add ourself to database. If we joined then we are the admin
        // and we may join another host at the same time
        return true;
    }

    if( !isLocked )
    {
        lockResources();
    }

    bool result = m_UserJoinInfoDb.addUserJoin( joinInfo );

    if( !isLocked )
    {
        unlockResources();
    }

    return result;
}

//============================================================================
void UserJoinMgr::removeFromJoinedLastDatabase( GroupieId& groupieId, bool resourcesLocked )
{
    if( !resourcesLocked )
    {
        lockResources();
    }

    m_UserJoinInfoDb.removeUserJoin( groupieId );
    if( !resourcesLocked )
    {
        unlockResources();
    }
}

//============================================================================
void UserJoinMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase, VxGUID& connectionId, VxGUID& peerOnlineId )
{
    // TODO BRJ handle disconnect
}

//============================================================================
bool UserJoinMgr::getLastJoinedHostUrlFromDb( EHostType hostType, std::string& retHostUrl )
{
    VxGUID onlineId;
    int64_t lastJoinMs;
    bool result = m_UserJoinedLastDb.getJoinedLast( hostType, onlineId, lastJoinMs, retHostUrl );
    if( result )
    {
        VxPtopUrl ptopUrl( retHostUrl );
        result = ptopUrl.isValid();
        if( !result )
        {
            LogMsg( LOG_ERROR, "UserJoinMgr::%s invalid url for host type %s", __func__, DescribeHostType( hostType ) );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::%s no last joined host url for host type %s", __func__, DescribeHostType( hostType ) );
    }

    return result;
}


//============================================================================
bool UserJoinMgr::getLastJoinedGroupieIdFromDb( EHostType hostType, GroupieId& retGroupieId )
{
    VxGUID onlineId;
    int64_t lastJoinMs;
    std::string hostUrl;
    bool result = m_UserJoinedLastDb.getJoinedLast( hostType, onlineId, lastJoinMs, hostUrl );
    if( result )
    {
        VxPtopUrl ptopUrl( hostUrl );
        result = ptopUrl.isValid();
        if( !result )
        {
            LogMsg( LOG_ERROR, "UserJoinMgr::%s invalid url for host type %s", __func__, DescribeHostType( hostType ) );
        }
        else
        {
            retGroupieId = GroupieId( m_Engine.getMyOnlineId(), onlineId, hostType );
            return true;
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::%s no last joined host url for host type %s", __func__, DescribeHostType( hostType ) );
    }

    return false;
}


//============================================================================
void UserJoinMgr::changeJoinState( GroupieId& groupieId, EJoinState joinState )
{
    lockResources();
    UserJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo && joinInfo->setJoinState( joinState ) )
    {
        if( groupieId.getUserOnlineId() != m_Engine.getMyOnlineId() )
        {
            saveToDatabase( joinInfo, false );
        }

        announceUserJoinUpdated( joinInfo );
    }

    unlockResources();
}

//============================================================================
EJoinState UserJoinMgr::getUserJoinState( GroupieId& groupieId )
{
    if( VxGetShowMyselfInLists() && groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() && groupieId.getHostOnlineId() == m_Engine.getMyOnlineId() )
    {
        // is myself and allowed
        return eJoinStateJoinIsGranted;
    }

    EJoinState joinState{ eJoinStateNone };
    lockResources();
    UserJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        joinState = joinInfo->getJoinState();
    }

    unlockResources();

    return joinState;
}

//============================================================================
void UserJoinMgr::queryUserListFromHost( GroupieId& groupieId )
{
    PluginBaseHostClient* plugin = dynamic_cast< PluginBaseHostClient*>(m_Engine.getPluginMgr().findHostClientPlugin( groupieId.getHostType() ) );
    if( plugin )
    {
        plugin->queryUserListFromHost( groupieId );
    }
}

//============================================================================
void UserJoinMgr::updateUserIsJoined( UserJoinInfo * userJoinInfo )
{
    if( !userJoinInfo )
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::%s null userJoinInfo", __func__ );
        vx_assert( false );
        return;
    }

    EHostType hostType = userJoinInfo->getHostType();
    if( eHostTypeUnknown == hostType )
    {
        LogMsg( LOG_ERROR, "UserJoinMgr::%s Invalid Host Type", __func__ );
        vx_assert( false );
        return;
    }

    EJoinState joinState = userJoinInfo->getJoinState();
    bool isJoinedGranted = eJoinStateJoinIsGranted == joinState;
    VxNetIdent* netIdent = &userJoinInfo->getNetIdent();
    if( netIdent && isJoinedGranted != netIdent->getIsJoined( hostType ) )
    {
        netIdent->setIsJoined( hostType, isJoinedGranted );
        m_Engine.toGuiContactAnythingChange( netIdent );
    }
}
