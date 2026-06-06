//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostServerJoinMgr.h"

#include "HostJoinInfo.h"
#include "HostJoinInfoDb.h"
#include "HostJoinCallbackInterface.h"

#include <BaseInfo/BaseSessionInfo.h>
#include <BigListLib/BigListInfo.h>
#include <Membership/MemberActiveMgr.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktAnnounce.h>

//============================================================================
HostServerJoinMgr::HostServerJoinMgr( P2PEngine& engine, const char* dbName, const char* dbJoinedLastName )
: m_Engine( engine )
, m_HostJoinInfoDb( engine, *this, dbName )
, m_HostJoinedLastDb( engine, *this, dbJoinedLastName )
{
    LogMsg( LOG_VERBOSE, "HostServerJoinMgr::HostServerJoinMgr" );
}

//============================================================================
bool HostServerJoinMgr::deleteDatabase( void )
{
    bool result = m_HostJoinInfoDb.deleteDatabase();
    result &= m_HostJoinedLastDb.deleteDatabase();
    return result;
}

//============================================================================
void HostServerJoinMgr::fromGuiUserLoggedOn( void )
{
    if( !m_Initialized )
    {
        m_Initialized = true;
        // user specific directory should be set
        std::string dbJoinFileName = VxGetSettingsDirectory();
        std::string dbJoinedLastName = VxGetSettingsDirectory();
        dbJoinFileName += m_HostJoinInfoDb.getDatabaseName();
        dbJoinedLastName += m_HostJoinedLastDb.getDatabaseName();

        lockHostJoinInfoList();
        m_HostJoinInfoDb.dbShutdown();
        m_HostJoinInfoDb.dbStartup( USER_HOST_JOIN_DB_VERSION, dbJoinFileName );

        m_HostJoinedLastDb.dbShutdown();
        m_HostJoinedLastDb.dbStartup( JOINED_LAST_DB_VERSION, dbJoinedLastName );      

        clearHostJoinInfoList();
        m_HostJoinInfoDb.getAllHostJoins( m_HostJoinInfoList );
        for( auto iter = m_HostJoinInfoList.begin(); iter != m_HostJoinInfoList.end(); ++iter )
        {
            HostJoinInfo* hostJoinInfo = iter->second;
            if( !hostJoinInfo->getNetIdent() )
            {
                VxNetIdent* netIdent = m_Engine.getBigListMgr().findBigListInfo( hostJoinInfo->getOnlineId() );
                if( netIdent )
                {
                    hostJoinInfo->setNetIdent( netIdent );
                }
                else if( hostJoinInfo->getOnlineId() == m_Engine.getMyOnlineId() && m_Engine.getMyNetIdent()->isValidNetIdent() )
                {
                    // is myself
                    hostJoinInfo->setNetIdent( m_Engine.getMyNetIdent() );
                }
            }
        }

        m_HostJoinListInitialized = true;
        unlockHostJoinInfoList();
    }
}

//============================================================================
void HostServerJoinMgr::wantHostJoinMgrCallbacks( HostJoinCallbackInterface * client, bool enable )
{
    lockClientList();
    for( auto iter = m_HostJoinClients.begin(); iter != m_HostJoinClients.end(); ++iter )
    {
        if( *iter == client )
        {
            m_HostJoinClients.erase( iter );
            break;
        }
    }

    if( enable )
    {
        m_HostJoinClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void HostServerJoinMgr::announceHostJoinRequested( HostJoinInfo* hostJoinInfo )
{
    if( hostJoinInfo )
    {
        if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_INFO, "HostServerJoinMgr::%s %s", __func__, hostJoinInfo->describeHostJoin().c_str() );
	
	    lockClientList();
        for( auto& client : m_HostJoinClients )
        {
		    client->callbackHostJoinRequested( hostJoinInfo );
	    }

	    unlockClientList();
        if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_INFO, "HostServerJoinMgr::%s done", __func__ );
    }
    else
    {
        LogMsg( LOG_ERROR, "HostServerJoinMgr::%s null hostJoinInfo", __func__ );
    }
}

//============================================================================
void HostServerJoinMgr::announceHostJoinUpdated( HostJoinInfo* hostJoinInfo )
{
    if( hostJoinInfo )
    {
        if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_INFO, "HostServerJoinMgr::%s %s", __func__, hostJoinInfo->describeHostJoin().c_str() );

        lockClientList();
        for( auto& client : m_HostJoinClients )
        {
            client->callbackHostJoinUpdated( hostJoinInfo );
        }

        unlockClientList();
    }
    else
    {
        LogModule( eLogHostJoin, LOG_ERROR, "HostServerJoinMgr::%s null hostJoinInfo", __func__ );
    }
}

//============================================================================
void HostServerJoinMgr::announceHostUnJoin( GroupieId& groupieId )
{
    if(LogEnabled( eLogHostJoin))LogModule( eLogHostJoin, LOG_ERROR, "HostServerJoinMgr::%s %s", __func__, groupieId.describeGroupieId().c_str() );

    lockClientList();
    for( auto& client : m_HostJoinClients )
    {
        client->callbackHostUnJoin( groupieId );
    }

    unlockClientList();
}

//============================================================================
void HostServerJoinMgr::announceHostJoinRemoved( GroupieId& groupieId )
{
    if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_ERROR, "HostServerJoinMgr::%s %s", __func__, groupieId.describeGroupieId().c_str() );

    removeFromDatabase( groupieId, false );
	lockClientList();
    for( auto& client : m_HostJoinClients )
	{
		client->callbackHostJoinRemoved( groupieId );
	}

	unlockClientList();
}

//============================================================================
void HostServerJoinMgr::clearHostJoinInfoList( void )
{
    for( auto iter = m_HostJoinInfoList.begin(); iter != m_HostJoinInfoList.end(); ++iter )
    {
        delete iter->second;
    }

    m_HostJoinInfoList.clear();
}

//============================================================================
void HostServerJoinMgr::onHostJoinRequestedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, BaseSessionInfo& sessionInfo )
{
    bool wasAdded = false;
    GroupieId groupieId( sessionInfo.getGroupieId() );
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent )
    {
        lockHostJoinInfoList();
        HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
        if( !joinInfo )
        {
            joinInfo = new HostJoinInfo();
            joinInfo->setGroupieId( groupieId );
            wasAdded = true;
        }

        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
        joinInfo->setJoinState( eJoinStateJoinRequested );

        joinInfo->setUserUrl( netIdent->getMyOnlineUrl() );
        joinInfo->setFriendState( netIdent->getMyFriendshipToHim() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( timeNowMs );
        if( wasAdded )
        {
            m_HostJoinInfoList[joinInfo->getGroupieId()] = joinInfo;
        }
    
        saveToDatabase( joinInfo, true );
        unlockHostJoinInfoList();

        if( wasAdded )
        {
            announceHostJoinRequested( joinInfo );
        }
        else
        {
            announceHostJoinUpdated( joinInfo );
        }
    }
    else
    {
        LogModule( eLogHostJoin, LOG_ERROR, "HostServerJoinMgr::%s null netIdent", __func__ );
    }
}

//============================================================================
void HostServerJoinMgr::onHostUnJoinRequestedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, BaseSessionInfo& sessionInfo )
{
    GroupieId groupieId( sessionInfo.getGroupieId() );
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent )
    {
        lockHostJoinInfoList();
        HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
        if( !joinInfo )
        {
            unlockHostJoinInfoList();
            LogModule( eLogHostJoin, LOG_ERROR, "HostServerJoinMgr::%s null joinInfo", __func__ );
            return;
        }

        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType()  );
        joinInfo->setSessionId( sessionInfo.getSessionId() );
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
        joinInfo->setJoinState( eJoinStateNone );
        joinInfo->setUserUrl( netIdent->getMyOnlineUrl() );
        joinInfo->setFriendState( netIdent->getMyFriendshipToHim() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( timeNowMs );
        unlockHostJoinInfoList();

        removeFromDatabase( joinInfo->getGroupieId(), false );

        announceHostUnJoin( joinInfo->getGroupieId() );
    }
}

//============================================================================
void HostServerJoinMgr::onHostJoinedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, BaseSessionInfo& sessionInfo )
{
    bool wasAdded = false;
    GroupieId groupieId( sessionInfo.getGroupieId() );
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent )
    {

        lockHostJoinInfoList();
        HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
        if( !joinInfo )
        {
            joinInfo = new HostJoinInfo();
            joinInfo->setGroupieId( groupieId );
            wasAdded = true;
        }

        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
        joinInfo->setJoinState( eJoinStateJoinIsGranted );
        joinInfo->setUserUrl( netIdent->getMyOnlineUrl() );
        joinInfo->setFriendState( netIdent->getMyFriendshipToHim() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( timeNowMs );

        unlockHostJoinInfoList();

        saveToDatabase( joinInfo );

        m_Engine.getThumbMgr().queryThumbIfNeeded( sktBase, netIdent, sessionInfo.getHostPluginType() );

        if( wasAdded )
        {
            announceHostJoinRequested( joinInfo );
        }
        else
        {
            announceHostJoinUpdated( joinInfo );
        }
    }
}

//============================================================================
void HostServerJoinMgr::onHostLeftByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, BaseSessionInfo& sessionInfo )
{
    GroupieId groupieId( sessionInfo.getGroupieId() );
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent )
    {
        lockHostJoinInfoList();
        HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
        if( !joinInfo )
        {
            unlockHostJoinInfoList();
            return;
        }

        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
        joinInfo->setJoinState( eJoinStateJoinLeaveHost );
        joinInfo->setUserUrl( netIdent->getMyOnlineUrl() );
        joinInfo->setFriendState( netIdent->getMyFriendshipToHim() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( timeNowMs );

        unlockHostJoinInfoList();

        announceHostJoinUpdated( joinInfo );
    }
}

//============================================================================
void HostServerJoinMgr::onHostUnJoinedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, BaseSessionInfo& sessionInfo )
{
    GroupieId groupieId( sessionInfo.getGroupieId() );
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( groupieId.getUserOnlineId() );
    if( netIdent )
    {

        lockHostJoinInfoList();
        HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
        if( !joinInfo )
        {
            unlockHostJoinInfoList();
            return;
        }

        joinInfo->fillBaseInfo( netIdent, sessionInfo.getHostType() );

        joinInfo->setHostType( sessionInfo.getHostType() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );
        joinInfo->setNetIdent( netIdent );
        int64_t timeNowMs = GetTimeStampMs();
        joinInfo->setThumbId( netIdent->getThumbId( sessionInfo.getHostType() ) );
        joinInfo->setJoinState( eJoinStateJoinLeaveHost );
        joinInfo->setUserUrl( netIdent->getMyOnlineUrl() );
        joinInfo->setFriendState( netIdent->getMyFriendshipToHim() );

        joinInfo->setConnectionId( sktBase->getSocketId() );
        joinInfo->setSessionId( sessionInfo.getSessionId() );

        joinInfo->setInfoModifiedTime( timeNowMs );
        joinInfo->setLastConnectTime( timeNowMs );
        joinInfo->setLastJoinTime( timeNowMs );

        unlockHostJoinInfoList();

        removeFromDatabase( joinInfo->getGroupieId(), false );

        announceHostUnJoin( joinInfo->getGroupieId() );
    }
}

//============================================================================
HostJoinInfo* HostServerJoinMgr::findUserJoinInfo( GroupieId& groupieId )
{
    auto iter = m_HostJoinInfoList.find( groupieId );

    if( iter != m_HostJoinInfoList.end() )
    {
        return iter->second;
    }

    return nullptr;
}

//============================================================================
bool HostServerJoinMgr::saveToDatabase( HostJoinInfo* joinInfo, bool resourcesLocked )
{
    if( joinInfo->getOnlineId() == m_Engine.getMyOnlineId() )
    {
        // do not add ourself to database. If we joined then we are the admin
        // and we may join another host at the same time
        return true;
    }

    if( !resourcesLocked )
    {
        lockHostJoinInfoList();
    }

    bool result = m_HostJoinInfoDb.addHostJoin( joinInfo );
    if( !resourcesLocked )
    {
        unlockHostJoinInfoList();
    }

    return result;
}

//============================================================================
void HostServerJoinMgr::removeFromDatabase( GroupieId& groupieId, bool resourcesLocked )
{
    if( !resourcesLocked )
    {
        lockHostJoinInfoList();
    }

    m_HostJoinInfoDb.removeHostJoin( groupieId );
    if( !resourcesLocked )
    {
        unlockHostJoinInfoList();
    }
}

//============================================================================
void HostServerJoinMgr::fromGuiGetJoinedStateList( EPluginType pluginType, EJoinState joinState, std::vector<HostJoinInfo*>& hostJoinList )
{
    // NOTE: assumes resources have been locked
    hostJoinList.clear();
    for( auto iter = m_HostJoinInfoList.begin(); iter != m_HostJoinInfoList.end(); ++iter )
    {
        if( iter->second->getHostType() == PluginTypeToHostType( pluginType ) && iter->second->getJoinState() == joinState )
        {
            hostJoinList.emplace_back( iter->second );
        }
    }
}

//============================================================================
int HostServerJoinMgr::fromGuiGetJoinedListCount( EPluginType pluginType )
{
    int joinedCnt = 0;
    EHostType hostType = PluginTypeToHostType( pluginType );
    lockHostJoinInfoList();

    for( auto iter = m_HostJoinInfoList.begin(); iter != m_HostJoinInfoList.end(); ++iter)
    {
        if( iter->second->getHostType() == hostType )
        {
            joinedCnt++;
        }
    }

    unlockHostJoinInfoList();
    return joinedCnt;
}

//============================================================================
EJoinState HostServerJoinMgr::fromGuiQueryJoinState( EHostType hostType, VxNetIdent& netIdent )
{
    EJoinState hostJoinState = eJoinStateNone;
    GroupieId groupieId( netIdent.getMyOnlineId(), m_Engine.getMyOnlineId(), hostType );

    lockHostJoinInfoList();
    HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        hostJoinState = joinInfo->getJoinState();
    }
    else if( netIdent.getMyOnlineId() == m_Engine.getMyOnlineId() )
    {
        // if we are host we can always join our own hosted servers
        hostJoinState = eJoinStateJoinWasGranted;
    }

    unlockHostJoinInfoList();
    return hostJoinState;
}


//============================================================================
EMembershipState HostServerJoinMgr::fromGuiQueryMembership( EHostType hostType, VxNetIdent& netIdent )
{
    EMembershipState membershipState{ eMembershipStateNone };
    GroupieId groupieId( netIdent.getMyOnlineId(), m_Engine.getMyOnlineId(), hostType );
    lockHostJoinInfoList();
    HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        EJoinState hostJoinState = joinInfo->getJoinState();
        switch( hostJoinState )
        {
        case eJoinStateNone:
        case eJoinStateSending:
        case eJoinStateSendFail:
        case eJoinStateSendAcked:
        case eJoinStateJoinRequested:
        case eJoinStateJoinLeaveHost:
            return eMembershipStateCanBeRequested;

        case eJoinStateJoinIsGranted:
        case eJoinStateJoinWasGranted:
            return eMembershipStateJoined;

        case eJoinStateJoinDenied:
        default:
            return eMembershipStateJoinDenied;
        }
    }
    else if( netIdent.getMyOnlineId() == m_Engine.getMyOnlineId() )
    {
        // if we are host we can always join our own hosted servers
        membershipState = eMembershipStateCanBeRequested;
    }

    unlockHostJoinInfoList();
    return membershipState;
}

//============================================================================
void HostServerJoinMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase, VxGUID& connectionId, VxGUID& peerOnlineId )
{
    // TODO BRJ handle disconnect
}

//============================================================================
void HostServerJoinMgr::updateJoinState( GroupieId& groupieId, EJoinState joinState )
{
    lockHostJoinInfoList();
    HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        if( joinInfo->getJoinState() != joinState )
        {
            joinInfo->setJoinState( joinState );
            if( groupieId.getUserOnlineId() != m_Engine.getMyOnlineId() )
            {
                saveToDatabase( joinInfo, true );
            }

            announceHostJoinUpdated( joinInfo );
        }
    }

    unlockHostJoinInfoList();
    if( !joinInfo )
    {
        LogMsg( LOG_ERROR, "HostServerJoinMgr::%s NULL joinInfo for %s joinState %s", __func__, 
            groupieId.describeGroupieId().c_str(), DescribeJoinState( joinState ) );
    }
}

//============================================================================
void HostServerJoinMgr::fromGuiListAction( EListAction listAction )
{
    
}

//============================================================================
EJoinState HostServerJoinMgr::getHostJoinState( GroupieId& groupieId )
{
    EJoinState joinState{ eJoinStateNone };
    lockHostJoinInfoList();
    HostJoinInfo* joinInfo = findUserJoinInfo( groupieId );
    if( joinInfo )
    {
        joinState = joinInfo->getJoinState();
    }

    unlockHostJoinInfoList();
    if( joinInfo && groupieId.getHostOnlineId() != m_Engine.getMyOnlineId() )
    {
        // we do not want to lose the last state but need the correct connected and/or joined state
        if( m_Engine.getConnectIdListMgr().isUserOnline( groupieId.getHostOnlineId() ) )
        {
            // is online
            if( m_Engine.getMemberActiveMgr().isMemberActive( groupieId ) )
            {
                // is active member
                joinState = eJoinStateJoinIsGranted;
                joinInfo->setJoinState( joinState );
            }
            else
            {
                // is not active
                if( eJoinStateJoinIsGranted == joinState )
                {
                    joinState = eJoinStateJoinWasGranted;
                    joinInfo->setJoinState( joinState );
                }
            }
        }
        else
        {
            // is offline
            if( eJoinStateJoinIsGranted == joinState )
            {
                joinState = eJoinStateJoinWasGranted;
                joinInfo->setJoinState( joinState );
            }
        }
    }

    return joinState;
}

//============================================================================
bool HostServerJoinMgr::isUserJoinedToRelayHost( VxGUID& onlineId )
{
    bool isJoined{ false };
    lockHostJoinInfoList();
    for( auto& hostPair : m_HostJoinInfoList )
    {
        GroupieId& groupieId = const_cast<GroupieId&>(hostPair.first);
        if( groupieId.getUserOnlineId() == onlineId && IsHostARelayForUsers( groupieId.getHostType() ) )
        {
            if( hostPair.second->getJoinState() == eJoinStateJoinWasGranted || hostPair.second->getJoinState() == eJoinStateJoinIsGranted )
            {
                isJoined = true;
                break;
            }
        }
    }

    unlockHostJoinInfoList();
    return isJoined;
}
