//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectIdListMgr.h"
#include "ConnectIdListCallback.h"

#include <BaseInfo/BaseSessionInfo.h>
#include <BigListLib/BigListInfo.h>
#include <HostServerJoinMgr/HostServerJoinMgr.h>

#include <Membership/MemberActiveMgr.h>
#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginBase.h>
#include <Plugins/PluginMgr.h>

#include <UserJoinMgr/UserJoinMgr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>

#include <PktLib/PktsRandConnectDefs.h>

#include <algorithm>

//============================================================================
ConnectIdListMgr::ConnectIdListMgr( P2PEngine& engine )
    : IdentListMgrBase( engine )
{
    setIdentListType( eUserViewTypeOnline );
}

//============================================================================
bool ConnectIdListMgr::getConnections( HostedId& hostId, std::set<ConnectId>& directConnectIdSet, std::set<ConnectId>& relayConnectIdSet )
{
    directConnectIdSet.clear();
    relayConnectIdSet.clear();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId&>(connectId).getHostedId() == hostId )
        {
            if( connectId.isRelayed() )
            {
                relayConnectIdSet.insert( connectId );
            }
            else
            {
                directConnectIdSet.insert( connectId );
            }      
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return !directConnectIdSet.empty() || !relayConnectIdSet.empty();
}

//============================================================================
bool ConnectIdListMgr::isDirectConnected( VxGUID& onlineId )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return true;
    }

    bool isDirectConnect = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId&>(connectId).getUserOnlineId() == onlineId )
        {
            if( !connectId.isRelayed() )
            {
                isDirectConnect = true;
                break;
            }
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return isDirectConnect;
}

//============================================================================
bool ConnectIdListMgr::isRelayed( VxGUID& onlineId )
{
    if( isDirectConnected( onlineId ) )
    {
        return false;
    }

    bool isRelayed = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId&>(connectId).getUserOnlineId() == onlineId )
        {
            if( connectId.isRelayed() )
            {
                isRelayed = true;
                break;
            }
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return isRelayed;
}

//============================================================================
bool ConnectIdListMgr::isHosted( VxGUID& onlineId )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return true;
    }

    bool isHosted = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        ConnectId& noConstConnectId = const_cast< ConnectId& >( connectId );
        if( noConstConnectId.getUserOnlineId() == onlineId && IsHostARelayForUsers( noConstConnectId.getHostType() ) )
        {
            isHosted = true;
            break;
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return isHosted;
}

//============================================================================
bool ConnectIdListMgr::isOnline( GroupieId& groupieId )
{
    bool isOnlined = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast< ConnectId& >( connectId ).getGroupieId() == groupieId )
        {
            isOnlined = true;
            break;
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return isOnlined;
}

//============================================================================
bool ConnectIdListMgr::addConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId )
{
    bool isRelayed = groupieId.getUserOnlineId() != sktBase->getPeerOnlineId();
    return addConnection( sktBase, groupieId, isRelayed );
}

//============================================================================
bool ConnectIdListMgr::addConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId, bool isRelayed )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s groupie id invalid", __func__ );
        return false;
    }

    if( !sktBase->isConnected() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s socket is no longer connected", __func__ );
        return false;
    }

    if( isRelayed )
    {
        LogModule( eLogConnect, LOG_VERBOSE, "ConnectIdListMgr::%s socket is relayed", __func__ );
    }

    VxGUID& sktConnectId = sktBase->getSocketId();

    if( !groupieId.isValid() || !sktConnectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::addConnection invalid id" );
        return false;
    }

    ConnectId connectId( sktConnectId, groupieId );
    connectId.setIsRelayed( isRelayed );
    return addConnection( connectId );
}

//============================================================================
bool ConnectIdListMgr::addConnection( ConnectId & connectId )
{
    if( connectionExists( connectId ) )
    {
        LogModule( eLogConnect, LOG_WARN, "ConnectIdListMgr::%s already exists %s", __func__, m_Engine.describeConnectId( connectId ).c_str() );
        return true;
    }

    VxGUID onlineId = connectId.getUserOnlineId();
    // make sure the user ident is updated before we announce connection so is known and has updated friendship if is a host member
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( onlineId );
    if( !netIdent )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s netIdent not found for id %s", __func__, onlineId.toOnlineIdString().c_str() );
        return false;
    }

    // avoid updating myself with info sent by host
    if( onlineId != m_Engine.getMyOnlineId() )
    {
        // make sure gui has updated ident before we change online status
        m_Engine.toGuiContactAnythingChange( netIdent );
    }

    bool wasOnline = isUserOnline( onlineId );
    bool becameOnline{ false };
    if( !wasOnline && !isUserExcluded( onlineId ) )
    {
        lockOnlineIdList();
        m_OnlineIdListList.insert( onlineId );
        unlockOnlineIdList();
        becameOnline = true;
    }
    
    VxGUID sktId = connectId.getSocketId();
    addOnlineConnectionPair( sktId, onlineId );

    lockConnectIdList();

    bool isInList = m_ConnectIdList.find( connectId ) != m_ConnectIdList.end();
    if( !isInList )
    {
        // new connection
        m_ConnectIdList.insert( connectId );
    }

    unlockConnectIdList();

    // TODO check if is a duplicate
    announceConnectionStatus( connectId, true );

    return true;
}

//============================================================================
void ConnectIdListMgr::removeConnection( ConnectId& connectId )
{
    if( !connectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s invalid id", __func__ );
        return;
    }

    bool connectIdWasRemoved = false;   

    lockConnectIdList();

    auto iter = m_ConnectIdList.find( connectId );
    if( iter != m_ConnectIdList.end() )
    {
        m_ConnectIdList.erase( iter );
        connectIdWasRemoved = true;
    }

    unlockConnectIdList();

    if( connectIdWasRemoved )
    {
        if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s ConnectId %s was removed", __func__, m_Engine.describeConnectId( connectId ).c_str() );
        announceConnectionStatus( connectId, false );
    }
    else
    {
        if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s ConnectId %s was NOT removed", __func__, m_Engine.describeConnectId( connectId ).c_str() );
    }
}

//============================================================================
void ConnectIdListMgr::removeConnection( VxGUID sktConnectId, GroupieId& groupieId )
{
    if( !groupieId.isValid() || !sktConnectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::removeConnection invalid id" );
        return;
    }

    if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "PluginBaseHostService::%s groupie %s ", __func__,
        m_Engine.describeGroupieId( groupieId ).c_str() );
    // might be a relayed and not relayed connection.. will have to remove them both
    ConnectId connectId( sktConnectId, groupieId );
    connectId.setIsRelayed( true );
    removeConnection( connectId );
    connectId.setIsRelayed( false );
    removeConnection( connectId );
}

//============================================================================
bool ConnectIdListMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
    if( sktBase->isTempConnection() )
    {
        return false;
    }

    return onConnectionLost( sktBase->getSocketId(), sktBase->isTempConnection() );
}

//============================================================================
bool ConnectIdListMgr::onConnectionLost( VxGUID& sktConnectId, bool tmpConnection )
{
    if( !sktConnectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::onConnectionLost invalid id" );
        return false;
    }

    if( isConnectIdExcluded( sktConnectId ) )
    {
        // was connection test or network host
        setExcludeConnectId( sktConnectId, false );

        // if !tmpConnection then probably is a admin of a host
        if( tmpConnection )
        {
            return false;
        }
    }

    if( !m_OnlineIdListList.size() && !m_ConnectIdList.size() && !m_OnlineConnectionPairs.size() )
    {
        // nothing to remove
        return false;
    }

    LogMsg( LOG_VERBOSE, "--ConnectIdListMgr::onConnectionLost start skt id %s cnt users %d connect ids %d pairs %d",
        sktConnectId.toHexString().c_str(), m_OnlineIdListList.size(), m_ConnectIdList.size(),
        m_OnlineConnectionPairs.size() );


    std::set<ConnectId> lostConnectList;

    lockConnectIdList();

    auto iter = m_ConnectIdList.begin();
    while( iter != m_ConnectIdList.end() )
    {
        ConnectId& connectId = const_cast<ConnectId&>( *iter );
        if( connectId.getSocketId() == sktConnectId )
        {
            lostConnectList.insert( connectId );
            iter = m_ConnectIdList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockConnectIdList();

    for( auto& connectId : lostConnectList )
    {
        announceConnectionStatus( const_cast<ConnectId&>( connectId ), false );
    }

    return true;
}

//============================================================================
void ConnectIdListMgr::userJoinedHost( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId )
{
    addConnection( sktBase, groupieId, false );
}

//============================================================================
void ConnectIdListMgr::userLeftHost( VxGUID& sktConnectId, GroupieId& groupieId )
{
    if( !groupieId.isValid() || !sktConnectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::userJoinedHost invalid id" );
        return;
    }

    LogMsg( LOG_ERROR, "ConnectIdListMgr::userLeftHost skt id %s groupie %s",
        sktConnectId.toHexString().c_str(), groupieId.describeGroupieId().c_str() );

    removeConnection( sktConnectId, groupieId );
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findDirectConnection( VxGUID& onlineId )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::findPeerConnection invalid id" );
        return nullptr;
    }

    VxGUID socketId;
    std::shared_ptr<VxSktBase> sktBase( nullptr );

    lockConnectIdList();
    for( auto& connectId : m_ConnectIdList )
    {
        if( !connectId.isRelayed() )
        {
            if( const_cast<ConnectId&>( connectId ).getUserOnlineId() == onlineId &&
                const_cast<ConnectId&>( connectId ).getHostOnlineId() == onlineId )
            {
                socketId = const_cast<ConnectId&>( connectId ).getSocketId();
                break;
            }
        }
    }

    unlockConnectIdList();

    if( socketId.isValid() )
    {
        sktBase = findSktBase( socketId );
    }

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findHostConnection( GroupieId& groupieId, bool tryPeerFirst )
{
    // host connection can only be a direct connection
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s invalid id", __func__ );
        return std::shared_ptr<VxSktBase>();
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    if( groupieId.isValid() )
    {
        if( groupieId.getHostOnlineId() == m_Engine.getMyOnlineId() && groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
        {
            return m_Engine.getSktLoopback();
        }

        if( tryPeerFirst )
        {
            sktBase = findPeerConnection( groupieId.getUserOnlineId() );
        }

        if( !sktBase )
        {
            VxGUID connectId;
            if( findConnectionId( groupieId, connectId ) )
            {
                sktBase = findSktBase( connectId );
            }
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s invalid groupieId", __func__ );
    }

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findAnyHostConnection( EHostType hostType )
{
    if( !IsHostARelayForUsers( hostType ) )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s invalid host type %d", __func__, hostType );
        return std::shared_ptr<VxSktBase>();
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectIdConst : m_ConnectIdList )
    {
        ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
        //if( connectId.getUserOnlineId() == onlineId && IsHostARelayForUsers( connectId.getHostType() ) )
        if( connectId.getHostType() == hostType )
        {
            sktBase = findSktBase( connectId.getSocketId() );
            if( sktBase && sktBase->isConnected() )
            {
                break;
            }
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findRelayMemberConnection( VxGUID& onlineId )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::findUserConnection invalid id" );
        return std::shared_ptr<VxSktBase>();
    }

    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return m_Engine.getSktLoopback();
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectIdConst : m_ConnectIdList )
    {
        ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
        //if( connectId.getUserOnlineId() == onlineId && IsHostARelayForUsers( connectId.getHostType() ) )
        if( connectId.getUserOnlineId() == onlineId && connectId.isRelayed() )
        {
            sktBase = findSktBase( connectId.getSocketId() );
            if( sktBase && sktBase->isConnected() )
            {
                break;
            }
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    if( sktBase )
    {
        LogModule( eLogRelay, LOG_VERBOSE, "ConnectIdListMgr::findRelayMemberConnection found connection %s %s for online id %s", 
                   sktBase->getSocketIdText().c_str(), sktBase->describeSktConnection().c_str(), onlineId.toOnlineIdString().c_str() );
    }
    else
    {
        LogModule( eLogRelay, LOG_VERBOSE, "ConnectIdListMgr::findRelayMemberConnection no connection for online id %s", 
                   onlineId.toOnlineIdString().c_str() );
    }

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findPeerConnection( VxGUID& onlineId )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::findPeerConnection invalid id" );
        return nullptr;
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    GroupieId groupieIdDirect( onlineId, onlineId, eHostTypePeerUser );

    VxGUID connectId;
    if( findConnectionId( groupieIdDirect, connectId ) )
    {
        sktBase = findSktBase( connectId );
    }
    else if( findRelayConnectionId( onlineId, connectId ) )
    {
        sktBase = findSktBase( connectId );
    }

    return sktBase;
}

//============================================================================
bool ConnectIdListMgr::findConnectionId( GroupieId& groupieId, VxGUID& retSktConnectId )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::findConnectionId invalid id" );
        return false;
    }

    bool foundConnection = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast< ConnectId& >( connectId ).getGroupieId() == groupieId )
        {
            retSktConnectId = const_cast< ConnectId& >( connectId ).getSocketId();
            foundConnection = retSktConnectId.isValid();
            break;
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return foundConnection;
}

//============================================================================
bool ConnectIdListMgr::findRelayConnectionId( VxGUID& onlineId, VxGUID& retSktConnectId )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::findRelayConnectionId invalid id" );
        return false;
    }

    bool foundConnection = false;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId&>(connectId).getUserOnlineId() == onlineId )
        {
            retSktConnectId = const_cast<ConnectId&>(connectId).getSocketId();
            foundConnection = retSktConnectId.isValid();
            break;
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return foundConnection;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findSktBase( VxGUID connectId )
{
    #if defined(DEBUG_SKT_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockSktBaseMgr", __func__ );
    #endif // defined(DEBUG_SKT_MGR_LOCK)
    m_Engine.getPeerMgr().lockSktBaseMgr();
    #if defined(DEBUG_SKT_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockSktBaseMgr done", __func__ );
    #endif // defined(DEBUG_SKT_MGR_LOCK)
    std::shared_ptr<VxSktBase> sktBase = m_Engine.getPeerMgr().findSktBase( connectId );
    if( sktBase )
    {
        sktBase = sktBase->isConnected() ? sktBase : nullptr;
    }

    #if defined(DEBUG_SKT_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockSktBaseMgr", __func__ );
    #endif // defined(DEBUG_SKT_MGR_LOCK)
    m_Engine.getPeerMgr().unlockSktBaseMgr();
    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findAnyHostOnlineConnection( VxGUID onlineId )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return m_Engine.getSktLoopback();
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    if( isUserExcluded( onlineId ) )
    {
        return sktBase;
    }

    std::set<VxGUID> sktConnectIdList;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId& >( connectId ).getGroupieId().getHostOnlineId() == onlineId )
        {
            sktConnectIdList.insert( const_cast< ConnectId& >( connectId ).getSocketId() );
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    for( auto sktConnectId : sktConnectIdList )
    {
        sktBase = findSktBase( sktConnectId );
        if( sktBase )
        {
            break;
        }
    }

    if( sktConnectIdList.empty() )
    {
        // check connection pairs
        lockOnlineIdList();
        // make a list of who used the connection and remove the connections from the list
        for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); iter++ )
        {
            if( iter->second == onlineId )
            {
                sktConnectIdList.insert( iter->first );
            }
        }

        unlockOnlineIdList();
    }

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findAnyUserOnlineConnection( VxGUID onlineId, EHostType hostType )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return m_Engine.getSktLoopback();
    }

    std::set<VxGUID> sktConnectIdList;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast<ConnectId& >( connectId ).getGroupieId().getUserOnlineId() == onlineId )
        {
            if( eHostTypeUnknown == hostType || connectId.getHostType() == hostType )
            {
                sktConnectIdList.insert( const_cast< ConnectId& >( connectId ).getSocketId() );
            }
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    if( sktConnectIdList.empty() )
    {
        // check connection pairs
        lockOnlineIdList();
        // make a list of who used the connection and remove the connections from the list
        for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); iter++ )
        {
            if( iter->second == onlineId )
            {
                sktConnectIdList.insert( iter->first );
            }
        }

        unlockOnlineIdList();
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    for( auto sktConnectId : sktConnectIdList )
    {
        sktBase = findSktBase( sktConnectId );
        if( sktBase )
        {
            break;
        }
    }

    return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findBestHostOnlineConnection( VxGUID& onlineId )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return m_Engine.getSktLoopback();
    }

    std::vector<ConnectId> connectIdList;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast< ConnectId& >( connectId ).getGroupieId().getHostOnlineId() == onlineId )
        {
            connectIdList.emplace_back( connectId );
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    VxGUID sktConnectId;
    // first check for direct connection
    GroupieId directGroupieId( onlineId, onlineId, eHostTypePeerUser );
    for( auto& connectId : connectIdList )
    {
        if( connectId.getGroupieId() == directGroupieId )
        {
            sktConnectId = connectId.getSocketId();
            break;
        }
    }

    if( sktConnectId.isValid() )
    {
        return findSktBase( sktConnectId );
    }

    return findAnyHostOnlineConnection( onlineId );
}


//============================================================================
std::shared_ptr<VxSktBase> ConnectIdListMgr::findBestUserOnlineConnection( VxGUID& onlineId, EPluginType pluginType )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        return m_Engine.getSktLoopback();
    }

    EHostType hostType{ eHostTypeUnknown };
    if( IsPluginARelayForUser( pluginType ) )
    {
        hostType = PluginTypeToHostType( pluginType );
    }

    // Relay-hosted sends must stay on the matching host/session connection instead of
    // falling back to an unrelated direct peer socket for the same online user.
    if( eHostTypeUnknown != hostType )
    {
        std::shared_ptr<VxSktBase> hostSktBase = findAnyUserOnlineConnection( onlineId, hostType );
        if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
            "ConnectIdListMgr::%s plugin %s host %s online %s selected host-scoped skt %d ip %s connected %s",
            __func__,
            DescribePluginType( pluginType ),
            DescribeHostType( hostType ),
            m_Engine.describeUser( onlineId ).c_str(),
            hostSktBase ? hostSktBase->getSktNumber() : -1,
            hostSktBase ? hostSktBase->getRemoteIpAddress() : "",
            ( hostSktBase && hostSktBase->isConnected() ) ? "true" : "false" );
        return hostSktBase;
    }
 
    std::vector<ConnectId> connectIdList;
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectId : m_ConnectIdList )
    {
        if( const_cast< ConnectId& >( connectId ).getGroupieId().getUserOnlineId() == onlineId )
        {
            connectIdList.emplace_back( connectId );
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    VxGUID sktConnectId;
    // first check for direct connection
    GroupieId directGroupieId( onlineId, onlineId, eHostTypePeerUser );
    for( auto& connectId : connectIdList )
    {
        if( connectId.getGroupieId() == directGroupieId )
        {
            sktConnectId = connectId.getSocketId();
            break;
        }
    }

    if( sktConnectId.isValid() )
    {
        std::shared_ptr<VxSktBase> sktBase = findSktBase( sktConnectId );
        if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
            "ConnectIdListMgr::%s plugin %s online %s selected direct skt %d ip %s connected %s",
            __func__,
            DescribePluginType( pluginType ),
            m_Engine.describeUser( onlineId ).c_str(),
            sktBase ? sktBase->getSktNumber() : -1,
            sktBase ? sktBase->getRemoteIpAddress() : "",
            ( sktBase && sktBase->isConnected() ) ? "true" : "false" );
        return sktBase;
    }

    std::shared_ptr<VxSktBase> sktBase = findAnyUserOnlineConnection( onlineId, hostType );
    if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
        "ConnectIdListMgr::%s plugin %s host %s online %s fallback skt %d ip %s connected %s",
        __func__,
        DescribePluginType( pluginType ),
        DescribeHostType( hostType ),
        m_Engine.describeUser( onlineId ).c_str(),
        sktBase ? sktBase->getSktNumber() : -1,
        sktBase ? sktBase->getRemoteIpAddress() : "",
        ( sktBase && sktBase->isConnected() ) ? "true" : "false" );
    return sktBase;
}

//============================================================================
void ConnectIdListMgr::wantConnectIdListCallback( ConnectIdListCallback* client, bool enable )
{
    if( !client )
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::wantConnectIdListCallback null client" );
        vx_assert( false );
        return;
    }

    lockConnectIdClientList();
    bool foundClient{ false };
    for( auto iter = m_ConnectIdCallbackClients.begin(); iter != m_ConnectIdCallbackClients.end(); ++iter )
    {
        if( *iter == client )
        {
            foundClient = true;
            if( !enable )
            {
                m_ConnectIdCallbackClients.erase( iter );
            }

            break;
        }
    }

    if( !foundClient && enable )
    {
        m_ConnectIdCallbackClients.emplace_back( client );
    }

    unlockConnectIdClientList();
}

//============================================================================
void ConnectIdListMgr::announceConnectionStatus( ConnectId& connectId, bool isConnected )
{
    if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s online %d connection %s", __func__,
        isConnected, m_Engine.describeConnectId( connectId ).c_str() );

    lockConnectIdClientList();

    for( auto& client : m_ConnectIdCallbackClients )
    {
        if( client )
        {
            client->callbackConnectionStatusChange( connectId, isConnected );
        }
        else
        {
            LogMsg( LOG_ERROR, "ConnectIdListMgr::announceConnectionStatus null client" );
        }
    }

    unlockConnectIdClientList();
}

//============================================================================
void ConnectIdListMgr::announceConnectionLost( VxGUID& sktConnectId )
{
    lockConnectIdClientList();

    for( auto& client : m_ConnectIdCallbackClients )
    {
        if( client )
        {
            client->callbackConnectionLost( sktConnectId );
        }
        else
        {
            LogMsg( LOG_ERROR, "ConnectIdListMgr::announceConnectionLost null client" );
        }
    }

    unlockConnectIdClientList();
}

//============================================================================
void ConnectIdListMgr::onGroupUserAnnounce( PktAnnounce* pktAnn, std::shared_ptr<VxSktBase>& sktBase, bool relayed )
{
    if( relayed )
    {
        onGroupRelayedUserAnnounce( pktAnn, sktBase );
        return;
    }

    VxGUID onlineId = pktAnn->getMyOnlineId();
    VxGUID connectionId = sktBase->getSocketId();
    // if relayed then the peer id should be the host that relayed the packet
    VxGUID hostOnlineId = sktBase->getPeerOnlineId();
    if( onlineId.isValid() && hostOnlineId.isValid() && connectionId.isValid() )
    {
        if( onlineId == hostOnlineId )
        {
            EHostType hostType{ eHostTypeUnknown };

            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            lockConnectIdList();
            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            for( auto& connectIdConst : m_ConnectIdList )
            {
                ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
                if( IsHostARelayForUsers( connectId.getHostType() ) )
                {
                    if( connectId.getHostedId().getHostOnlineId() == hostOnlineId )
                    {
                        hostType = connectId.getHostType();
                        hostOnlineId = connectId.getHostedId().getHostOnlineId();
                        break;
                    }
                }
            }

            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            unlockConnectIdList();

            if( hostType != eHostTypeUnknown )
            {
                GroupieId groupieId( onlineId, hostOnlineId, hostType );
                addConnection( sktBase, groupieId, relayed );
            }
            else
            {
                LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupUserAnnounce hostId not found" );
            }
        }
        else
        {
            LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupUserAnnounce onlineId != hostOnlineId" );
        }
    }
    else
    {
        LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupUserAnnounce invalid id" );
    }
}

//============================================================================
void ConnectIdListMgr::onGroupRelayedUserAnnounce( PktAnnounce* pktAnn, std::shared_ptr<VxSktBase>& sktBase )
{
    VxGUID socketId = sktBase->getSocketId();
    // if relayed then the peer id should be the host that relayed the packet
    VxGUID hostOnlineId = sktBase->getPeerOnlineId();
    VxGUID onlineId = pktAnn->getMyOnlineId();

    LogMsg( LOG_VERBOSE, "ConnectIdListMgr::onGroupRelayedUserAnnounce %s id %s hosted by %s id %s",
        pktAnn->getOnlineName(), onlineId.describeVxGUID().c_str(), sktBase->getPeerOnlineName().c_str(), hostOnlineId.toOnlineIdString().c_str());
    if( onlineId.isValid() && hostOnlineId.isValid() && socketId.isValid() )
    {
        if( onlineId != hostOnlineId )
        {
            std::set<EHostType> hostTypes;

            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            lockConnectIdList();
            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            for( auto& connectIdConst : m_ConnectIdList )
            {
                ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
                if( connectId.getSocketId() == socketId && IsHostARelayForUsers( connectId.getHostType() ) )
                {
                    if( connectId.getHostedId().getHostOnlineId() == hostOnlineId )
                    {
                        hostTypes.insert( connectId.getHostType() );
                    }
                }
            }

            #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
                LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
            #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
            unlockConnectIdList();

            if( !hostTypes.empty() )
            {
                for( auto hostType : hostTypes )
                {
                    LogMsg( LOG_VERBOSE, "ConnectIdListMgr::onGroupRelayedUserAnnounce %s from host %s %s",
                        pktAnn->getOnlineName(), DescribeHostType( hostType ), sktBase->getPeerOnlineName().c_str() );

                    GroupieId groupieId( onlineId, hostOnlineId, hostType );

                    addConnection( sktBase, groupieId, true );

                    // finally add the user group join info
                    PluginBase* plugin = m_Engine.getPluginMgr().findPlugin( HostTypeToClientPlugin( hostType ) );
                    if( plugin )
                    {
                        plugin->onGroupRelayedUserAnnounce( groupieId, sktBase, pktAnn );
                    }
                    else
                    {
                        LogMsg( LOG_VERBOSE, "ConnectIdListMgr::onGroupRelayedUserAnnounce %s from host %s %s Faield to get plugin",
                            pktAnn->getOnlineName(), DescribeHostType( hostType ), sktBase->getPeerOnlineName().c_str() );
                    }
                }
            }
            else
            {
                LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupRelayedUserAnnounce hostId not found" );
            }
        }
        else
        {
            LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupRelayedUserAnnounce onlineId != hostOnlineId" );
        }
    }
    else
    {
        LogMsg( LOG_WARNING, "ConnectIdListMgr::onGroupRelayedUserAnnounce invalid id" );
    }
}

//============================================================================
void ConnectIdListMgr::getOnlineMembers( HostedId& hostId, std::vector<VxGUID>& onlineIdList )
{
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectIdConst : m_ConnectIdList )
    {
        ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
        if( connectId.getHostedId() == hostId )
        {
            onlineIdList.emplace_back( connectId.getUserOnlineId() );
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
}

//============================================================================
bool ConnectIdListMgr::isMemberOnline( HostedId& hostId, VxGUID& onlineId )
{
    bool isOnline{ false };
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    for( auto& connectIdConst : m_ConnectIdList )
    {
        ConnectId& connectId = const_cast<ConnectId&>(connectIdConst);
        if( connectId.getHostedId() == hostId && connectId.getUserOnlineId() == onlineId )
        {
            isOnline = true;
            break;
        }
    }

    #if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
        LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
    #endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();
    return isOnline;
}

//============================================================================
void ConnectIdListMgr::pktAnnRecieved( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, VxNetIdent* peerNetIdent )
{
    VxGUID& sktConnectId = sktBase->getSocketId();
    VxGUID onlineId = pktAnn->getMyOnlineId();
    VxGUID hostOnlineId = peerNetIdent->getMyOnlineId();

    bool shouldAnnounce{ false };

    if( !sktConnectId.isValid() || !onlineId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s BAD PARAM", __func__ );
        vx_assert( false );
        return;
    }

    if( isUserExcluded( onlineId ) )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s excluded %s", __func__, onlineId.toOnlineIdString().c_str() );
        return;
    }

    if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s skt connect id %s user %s", __func__,
        sktConnectId.toHexString().c_str(), m_Engine.describeUser( onlineId ).c_str() );

    lockOnlineIdList();
    auto iter = std::find_if( m_OnlineConnectionPairs.begin(), m_OnlineConnectionPairs.end(),
                              [&]( const std::pair< VxGUID, VxGUID>& element ) { return element.first == sktConnectId && element.second == onlineId; } );
    if( iter == m_OnlineConnectionPairs.end() )
    {
        m_OnlineConnectionPairs.emplace_back( std::make_pair( sktConnectId, onlineId ) );
        if( m_OnlineIdListList.find( onlineId ) == m_OnlineIdListList.end() )
        {
            shouldAnnounce = true;
        }
    }

    unlockOnlineIdList();

    if( shouldAnnounce )
    {
        lockOnlineIdList();
        bool wasOnline = m_OnlineIdListList.find( onlineId ) != m_OnlineIdListList.end();
        if( !wasOnline )
        {
            m_OnlineIdListList.insert( onlineId );
        }

        unlockOnlineIdList();

        if( !wasOnline )
        {
            if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s announcing online %s", __func__,
                m_Engine.describeUser( onlineId ).c_str() );
            //announceOnlineStatus( onlineId, true );
        }
        else
        {
            if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s already was online %s", __func__,
                m_Engine.describeUser( onlineId ).c_str() );
        }
    }
    else if( LogEnabled( eLogOnline ) )
    {
        if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s not anouncing online %s", __func__,
            m_Engine.describeUser( onlineId ).c_str() );
    }

    bool wasAdded = checkUnconfirmedConnections( sktBase, onlineId );
    if( !wasAdded && !sktBase->isTempConnection() )
    {
        EHostType hostType = ConnectReasonToJoinHostType( sktBase->getConnectReason() );
        if( eHostTypeUnknown != hostType )
        {
            VxGUID hostOnlineId = sktBase->getPeerOnlineId();
            if( hostOnlineId.isValid() )
            {
                GroupieId groupieId( onlineId, hostOnlineId, hostType );
                bool isRelayed = hostOnlineId != onlineId;

                addConnection( sktBase, groupieId, isRelayed );
                if( isRelayed )
                {
                    // finally add the user group join info
                    PluginBase* plugin = m_Engine.getPluginMgr().findPlugin( HostTypeToClientPlugin( hostType ) );
                    if( plugin )
                    {
                        plugin->onGroupRelayedUserAnnounce( groupieId, sktBase, pktAnn );
                    }
                    else
                    {
                        LogMsg( LOG_VERBOSE, "ConnectIdListMgr::onGroupRelayedUserAnnounce %s from host %s %s Faield to get plugin",
                            pktAnn->getOnlineName(), DescribeHostType( hostType ), sktBase->getPeerOnlineName().c_str() );
                    }
                }
            }
        }
        else
        {
            VxGUID hostOnlineId = sktBase->getPeerOnlineId();
            if( hostOnlineId.isValid() )
            {
                if( hostOnlineId == onlineId )
                {
                    GroupieId groupieId( onlineId, hostOnlineId, eHostTypePeerUser );
                    addConnection( sktBase, groupieId, false );
                }
            }
        }
    }    
}

//============================================================================
void ConnectIdListMgr::removeOnlineConnectionPairs( VxGUID& sktConnectId, std::set<VxGUID>& lostConnUserList )
{
    if( !sktConnectId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::doOnlineIdConnectionLost BAD PARAM" );
        vx_assert( false );
        return;
    }

    std::set<VxGUID> onlineIdList;
    lockOnlineIdList();
    // make a list of who used the connection and remove the connections from the list
    for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); )
    {
        if( iter->first == sktConnectId )
        {
            if( iter->second != m_Engine.getMyOnlineId() )
            {
                onlineIdList.insert( iter->second );
            }
            
            LogModule( eLogUsers, LOG_VERBOSE, "ConnectIdListMgr::%s removed skt id %s user %s ", __func__,
                   sktConnectId.toHexString().c_str(), m_Engine.describeUser( iter->second ).c_str() );
            iter = m_OnlineConnectionPairs.erase( iter );
        }
        else
        {
            iter++;
        }
    }

    // for each user if there is no more connections to user then add to lostConnectiononIdList
    for( auto& onlineId : onlineIdList )
    {
        if( onlineId == m_Engine.getMyOnlineId() )
        {
            continue;
        }

        auto iter = std::find_if( m_OnlineConnectionPairs.begin(), m_OnlineConnectionPairs.end(),
                              [&]( const std::pair< VxGUID, VxGUID>& element ) { return element.second == onlineId; } );
        if( iter == m_OnlineConnectionPairs.end() )
        {
            lostConnUserList.insert( onlineId );
        }
    }

    unlockOnlineIdList();
}

//============================================================================
void ConnectIdListMgr::addOnlineConnectionPair( VxGUID& sktConnectId, VxGUID& onlineId )
{
    bool wasFound{ false };
    lockOnlineIdList();
    for( auto& connectionPair : m_OnlineConnectionPairs )
    {
        if( connectionPair.first == sktConnectId && connectionPair.second == onlineId )
        {
            wasFound = true;
            break;
        }
    }

    if( !wasFound )
    {
        LogModule( eLogUsers, LOG_VERBOSE, "ConnectIdListMgr::addOnlineConnectionPair added skt id %s user %s ",
                   sktConnectId.toHexString().c_str(), m_Engine.describeUser( onlineId ).c_str() );
        m_OnlineConnectionPairs.emplace_back( std::make_pair( sktConnectId, onlineId ) );
    }

    unlockOnlineIdList();
}

//============================================================================
void ConnectIdListMgr::removeOnlineConnectionPair( VxGUID& sktConnectId, VxGUID& onlineId )
{
    lockOnlineIdList();
    // make a list of who used the connection and remove the connections from the list
    for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); )
    {
        if( iter->first == sktConnectId && iter->second == onlineId )
        {
            LogModule( eLogUsers, LOG_VERBOSE, "ConnectIdListMgr::removeOnlineConnectionPair removed skt id %s user %s ",
                   sktConnectId.toHexString().c_str(), m_Engine.describeUser( onlineId ).c_str() );
            iter = m_OnlineConnectionPairs.erase( iter );
        }
        else
        {
            iter++;
        }
    }

    unlockOnlineIdList();
}

//============================================================================
bool ConnectIdListMgr::isUserOnline( VxGUID& onlineId )
{
    if( !onlineId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::isUserOnline BAD PARAM" );
        vx_assert( false );
        return false;
    }

    lockOnlineIdList();
    bool isOnline = m_OnlineIdListList.find( onlineId ) != m_OnlineIdListList.end();
    unlockOnlineIdList();

    return isOnline;
}


//============================================================================
void ConnectIdListMgr::updateOnlineExclusion( VxGUID onlineId, bool excludeFromOnlineStatus, bool isNetworkHost )
{
    if( !onlineId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s BAD PARAM", __func__ );
        vx_assert( false );
        return;
    }

    if( LogEnabled( eLogOnline ) )
    {
        if( excludeFromOnlineStatus )
        {
            LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s is network host %d excluded %s", __func__,
                excludeFromOnlineStatus, m_Engine.describeUser( onlineId ).c_str() );
        }
    }

    lockOnlineIdList();
    if( isNetworkHost )
    {
        m_NetworkHostOnlineId = onlineId;
    }

    auto iter = m_OnlineIdExclusionList.find( onlineId );
    if( excludeFromOnlineStatus )
    {
        if( iter == m_OnlineIdExclusionList.end() )
        {
            m_OnlineIdExclusionList.insert( onlineId );
        }
    }
    else
    {
        if( iter != m_OnlineIdExclusionList.end() )
        {
            m_OnlineIdExclusionList.erase( iter );
        }
    }

    unlockOnlineIdList();
}

//============================================================================
bool ConnectIdListMgr::isUserExcluded( VxGUID onlineId )
{
    if( !onlineId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s BAD PARAM", __func__ );
        vx_assert( false );
        return true;
    }

    lockOnlineIdList();
    bool isExcluded = m_OnlineIdExclusionList.find( onlineId ) != m_OnlineIdExclusionList.end();
    unlockOnlineIdList();

    if( isExcluded )
    {
        if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s TRUE %s", m_Engine.describeUser( onlineId ).c_str() );
        return true;
    }

    return isExcluded;
}

//============================================================================
bool ConnectIdListMgr::isConnectionInUse( VxGUID& sktConnectId )
{
    if( !sktConnectId.isValid() )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s BAD PARAM", __func__ );
        vx_assert( false );
        return false;
    }

    bool sktInUse{ false };
    lockOnlineIdList();
    auto iter = std::find_if( m_OnlineConnectionPairs.begin(), m_OnlineConnectionPairs.end(),
                              [&]( const std::pair< VxGUID, VxGUID>& element ) { return element.first == sktConnectId; } );
    if( iter != m_OnlineConnectionPairs.end() )
    {
         sktInUse = true;
    }

    unlockOnlineIdList();
    if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %d", __func__, sktInUse );
    return sktInUse;
}

//============================================================================
void ConnectIdListMgr::disconnectIfIsOnlyUser( GroupieId& groupieId )
{
    VxGUID userOnlineId = groupieId.getUserOnlineId();

    std::set<VxGUID> userSktConnectIds;
    std::vector<VxGUID> toDisconnectIds;

    lockOnlineIdList();
    for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); ++iter )
    {
        if( iter->second == userOnlineId )
        {
            userSktConnectIds.insert( iter->first );
        }
    }

    for( auto sktConnectId : userSktConnectIds )
    {
        bool inUseByOther{ false };
        for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); ++iter )
        {
            if( iter->first == sktConnectId && iter->second != userOnlineId )
            {
                inUseByOther = true;
                break;
            }
        }

        if( !inUseByOther )
        {
            toDisconnectIds.emplace_back( sktConnectId );
        }
    }

    unlockOnlineIdList();

    for( auto& sktConnectId : toDisconnectIds )
    {
        m_Engine.getPeerMgr().closeConnection( sktConnectId, eSktCloseNotNeeded );
    }
}

//============================================================================
void ConnectIdListMgr::setExcludeConnectId( VxGUID& sktConnectId, bool exclude )
{
    lockExclusionList();

    auto iter = m_ConnectIdExclusionList.find( sktConnectId );
    if( exclude )
    {
        if( iter == m_ConnectIdExclusionList.end() )
        {
            m_ConnectIdExclusionList.insert( sktConnectId );
        }
    }
    else
    {
        if( iter != m_ConnectIdExclusionList.end() )
        {
            m_ConnectIdExclusionList.erase( iter );
        }
    }

    unlockExclusionList();
}

//============================================================================
bool ConnectIdListMgr::isConnectIdExcluded( VxGUID& sktConnectId )
{
    lockExclusionList();
    bool isExcluded = m_ConnectIdExclusionList.find( sktConnectId ) != m_ConnectIdExclusionList.end();
    unlockExclusionList();

    if( isExcluded && LogEnabled( eLogOnline ) )
    {
        LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s excluded %s", __func__, sktConnectId.describeVxGUID().c_str() );
    }

    return isExcluded;
}

//============================================================================
void ConnectIdListMgr::fromGuiDisconnectFromUser( VxGUID& userOnlineId )
{
	std::set<VxGUID> userSktConnectIds;
    std::vector<VxGUID> toDisconnectIds;

    lockOnlineIdList();
    for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); ++iter )
    {
        if( iter->second == userOnlineId )
        {
            userSktConnectIds.insert( iter->first );
        }
    }

    for( auto sktConnectId : userSktConnectIds )
    {
        bool inUseByOther{ false };
        for( auto iter = m_OnlineConnectionPairs.begin(); iter != m_OnlineConnectionPairs.end(); ++iter )
        {
            if( iter->first == sktConnectId && iter->second != userOnlineId )
            {
                inUseByOther = true;
                break;
            }
        }

        if( !inUseByOther )
        {
            toDisconnectIds.emplace_back( sktConnectId );
        }
    }

    unlockOnlineIdList();

    for( auto& sktConnectId : toDisconnectIds )
    {
        m_Engine.getPeerMgr().closeConnection( sktConnectId, eSktCloseClosedByUser );
    }
}

//============================================================================
void ConnectIdListMgr::addHostConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId ) 
{
    updateOnlineExclusion( groupieId.getHostOnlineId(), false, false );
    ConnectId connectId( sktBase->getSocketId(), groupieId );
    addConnection( sktBase, groupieId, false );
}

//============================================================================
bool ConnectIdListMgr::connectionExists( ConnectId& connectId )
{
#if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList", __func__ );
#endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    lockConnectIdList();
#if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s lockConnectIdList done", __func__ );
#endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)

    bool exists = m_ConnectIdList.find( connectId ) != m_ConnectIdList.end();

#if defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    LogMsg( LOG_DEBUG, "ConnectIdListMgr::%s unlockConnectIdList", __func__ );
#endif // defined(DEBUG_CONNECT_ID_LIST_MGR_LOCK)
    unlockConnectIdList();

    return exists;
}

//============================================================================
bool ConnectIdListMgr::addUnconfirmedConnection( ConnectId& connectId )
{
    if( connectId.isValid() )
    {
        if( connectId.isRelayed() )
        {
            if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %s user %s relayed %d", __func__,
                connectId.describeConnectId().c_str(), m_Engine.describeUser( connectId.getUserOnlineId() ).c_str(), connectId.isRelayed() );
        }

        if( connectionExists( connectId ) )
        {
            if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %s connection exists for user %s relayed %d", __func__,
                connectId.describeConnectId().c_str(), m_Engine.describeUser( connectId.getUserOnlineId() ).c_str(), connectId.isRelayed() );
            return false;
        }

        m_UnonfirmedConnectIdListMutex.lock();

        addUnconfirmedConnection( m_UnconfirmedConnectIdList, connectId );

        m_UnonfirmedConnectIdListMutex.unlock();

        return true;
    }

    return false;
}

//============================================================================
void ConnectIdListMgr::addUnconfirmedConnection( std::vector<std::pair<int64_t,ConnectId>>& unonfirmedIdList, ConnectId& connectId )
{
    const int64_t UNCONFIRMED_TIMEOUT_MS = 20000;
    if( connectId.isValid() )
    {
        bool found{false};
        for( auto idPair : unonfirmedIdList )
        {
            if( idPair.second == connectId )
            {
                found = true;
                if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %s update timeout for user %s relayed %d", __func__,
                    connectId.describeConnectId().c_str(), m_Engine.describeUser( connectId.getUserOnlineId() ).c_str(), connectId.isRelayed() );
                idPair.first = GetGmtTimeMs() + UNCONFIRMED_TIMEOUT_MS;
                break;
            }
        }

        if( !found )
        {
            if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %s adding user %s relayed %d", __func__,
                connectId.describeConnectId().c_str(), m_Engine.describeUser( connectId.getUserOnlineId() ).c_str(), connectId.isRelayed() );
            unonfirmedIdList.emplace_back( std::make_pair( GetGmtTimeMs() + UNCONFIRMED_TIMEOUT_MS, connectId ) );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectIdListMgr::%s invalid connectId", __func__ );
    }
}

//============================================================================
bool ConnectIdListMgr::checkUnconfirmedConnections( std::shared_ptr<VxSktBase>& sktBasw, VxGUID& onlineId )
{
    m_UnonfirmedConnectIdListMutex.lock();
    bool wasAdded = checkUnconfirmedList( sktBasw, onlineId, m_UnconfirmedConnectIdList );
    m_UnonfirmedConnectIdListMutex.unlock();
    return wasAdded;
}

//============================================================================
bool ConnectIdListMgr::checkUnconfirmedList( std::shared_ptr<VxSktBase>& sktBasw, VxGUID& onlineId, std::vector<std::pair<int64_t,ConnectId>>& unconfirmedIdList )
{
    bool wasAdded{ false };
    int64_t timeNow = GetGmtTimeMs();
    for( auto iter = unconfirmedIdList.begin(); iter != unconfirmedIdList.end();  )
    {
        if( iter->first < timeNow )
        {
            iter = unconfirmedIdList.erase( iter );
        }
        else
        {
            if( iter->second.getSocketId() == sktBasw->getSocketId() &&
                iter->second.getUserOnlineId() == onlineId )
            {
                if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s %s found user %s relayed %d", __func__,
                    iter->second.describeConnectId().c_str(), m_Engine.describeUser( iter->second.getUserOnlineId() ).c_str(), iter->second.isRelayed() );
                addConnection( sktBasw, iter->second.getGroupieId(), iter->second.isRelayed());
                iter = unconfirmedIdList.erase( iter );
                wasAdded = true;
            }
            else
            {
                ++iter;
            }
        }
    }

    return wasAdded;
}

//============================================================================
bool ConnectIdListMgr::updateUserJoinedFriendships( GroupieId& groupieId, VxNetIdent* netIdent )
{
    bool friendshipOk{ true };
    EFriendState prevMyFriendship = netIdent->getMyFriendshipToHim();
    EFriendState prevHisFriendship = netIdent->getHisFriendshipToMe();
    EFriendState curMyFriendship = prevMyFriendship;
    EFriendState curHisFriendship = prevHisFriendship;
    if( eFriendStateIgnore == curHisFriendship )
    {
        if( curMyFriendship == eFriendStateIgnore )
        {
            LogMsg( LOG_ERROR, "HostServerMgr::updateUserToJoinedToMyHostGuest got ignored user %s %s ",
                   netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str() );
            friendshipOk = false;
        }
        else
        {
            // not sure how this can happen but I guess is allowed so just downgrade to anonymous
            curMyFriendship = eFriendStateAnonymous;
        }
    }
    else if( eFriendStateIgnore == curMyFriendship )
    {
        curHisFriendship = eFriendStateIgnore; // so will appear in the blocked list
        friendshipOk = false;
    }
    else
    {
        if( eFriendStateAnonymous == curMyFriendship )
        {
            curMyFriendship = eFriendStateGuest;
        }

        if( eFriendStateAnonymous == curHisFriendship )
        {
            curHisFriendship = eFriendStateGuest;
        }
    }

    if( curMyFriendship != prevMyFriendship || curHisFriendship != prevHisFriendship )
    {
        netIdent->setMyFriendshipToHim( curMyFriendship );
        netIdent->setHisFriendshipToMe( curHisFriendship );

        if( curMyFriendship != prevMyFriendship )
        {
            m_Engine.getBigListMgr().onMyFriendshipChanged( prevMyFriendship, netIdent );
        }

        m_Engine.toGuiContactAnythingChange( netIdent );
    }

    return friendshipOk;
}

//============================================================================
void ConnectIdListMgr::disconnectFromHost( HostedId& hostId )
{
    std::set<ConnectId> hostIds;
    std::set<ConnectId> otherHostedIds;
    std::set<VxGUID> toDisconnectIds;

    lockConnectIdList();
    for( auto& connectIdInList : m_ConnectIdList )
    {
        ConnectId& connectId = const_cast<ConnectId&>( connectIdInList );
        if( connectId.getHostedId() == hostId )
        {
            hostIds.insert( connectId );
        }
        else if( connectId.getHostOnlineId() == hostId.getHostOnlineId() )
        {
            otherHostedIds.insert( connectId );
        }
    }

    unlockConnectIdList();
    for( auto& connected : hostIds )
    {
        toDisconnectIds.insert( connected.getSocketId() );
    }

    if( !toDisconnectIds.empty() && !otherHostedIds.empty() )
    {
        // if same socket is connection is used for another host then cannot be disconnected
        for( auto& connectIdInList : otherHostedIds )
        {
            auto connectIter = toDisconnectIds.find( connectIdInList.getSocketId() );
            if( connectIter != toDisconnectIds.end() )
            {
                toDisconnectIds.erase( connectIter );
                if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "ConnectIdListMgr::%s connot disconnect because in use by %s", __func__,
                              m_Engine.describeConnectId( const_cast<ConnectId&>( connectIdInList ) ).c_str() );
            }
        }
    }

    for( auto& connectIdInList : hostIds )
    {
        ConnectId& connectId = const_cast<ConnectId&>( connectIdInList );

        removeConnection( connectId );
    }

    for( auto sktConnectId : toDisconnectIds )
    {
        m_Engine.getPeerMgr().closeConnection( sktConnectId, eSktCloseHostLeave );
    }
}
