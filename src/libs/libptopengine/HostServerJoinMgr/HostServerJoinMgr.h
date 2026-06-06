#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostJoinInfoDb.h"
#include "HostJoinedLastDb.h"

#include <CoreLib/VxMutex.h>
#include <map>
#include <memory>

class BaseSessionInfo;
class HostJoinInfo;
class HostJoinCallbackInterface;
class P2PEngine;
class VxSktBase;
class VxNetIdent;
class GroupieId;

class HostServerJoinMgr 
{
    const int USER_HOST_JOIN_DB_VERSION = 1;
    const int JOINED_LAST_DB_VERSION = 1;
public:
	HostServerJoinMgr( P2PEngine& engine, const char* dbName, const char* dbJoinedLastName );
	virtual ~HostServerJoinMgr() = default;

    void                        fromGuiUserLoggedOn( void );
    virtual int					fromGuiGetJoinedListCount( enum EPluginType pluginType );
    virtual EJoinState	        fromGuiQueryJoinState( enum EHostType hostType, VxNetIdent& netIdent );
    EMembershipState            fromGuiQueryMembership( enum EHostType hostType, VxNetIdent& netIdent );
    void                        fromGuiGetJoinedStateList(enum  EPluginType pluginType, enum EJoinState joinState, std::vector<HostJoinInfo*>& hostJoinList );
    void                        fromGuiListAction( EListAction listAction );

    void                        wantHostJoinMgrCallbacks( HostJoinCallbackInterface * client, bool enable );

    virtual void				announceHostJoinRequested( HostJoinInfo* userHostInfo );
    virtual void				announceHostJoinUpdated( HostJoinInfo* userHostInfo );
    virtual void				announceHostUnJoin( GroupieId& groupieId );
    virtual void				announceHostJoinRemoved( GroupieId& groupieId );

    VxMutex&					getHostJoinInfoListMutex( void )                    { return m_HostJoinInfoListMutex; }
    void						lockHostJoinInfoList( void )						{ m_HostJoinInfoListMutex.lock(); }
    void						unlockHostJoinInfoList( void )						{ m_HostJoinInfoListMutex.unlock(); }

    void                        onHostJoinRequestedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onHostUnJoinRequestedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onHostJoinedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onHostLeftByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onHostUnJoinedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    virtual void                onConnectionLost( std::shared_ptr<VxSktBase>& sktBase, VxGUID& connectionId, VxGUID& peerOnlineId );

    HostJoinInfo*               findUserJoinInfo( GroupieId& groupieId );

    void                        updateJoinState( GroupieId& groupieId, enum EJoinState joinState );
    EJoinState                  getHostJoinState( GroupieId& groupieId );
    bool                        isUserJoinedToRelayHost( VxGUID& onlineId );

    virtual void                onUserOffline( VxGUID& onlineId ) {}; // TODO implement

    bool                        deleteDatabase( void );

protected:
    void						clearHostJoinInfoList( void );

    void						lockClientList( void )						{ m_HostJoinClientMutex.lock(); }
    void						unlockClientList( void )					{ m_HostJoinClientMutex.unlock(); }

    bool                        saveToDatabase( HostJoinInfo* joinInfo, bool resourcesLocked = false );
    void                        removeFromDatabase( GroupieId& groupieId, bool resourcesLocked );

    P2PEngine&					m_Engine;
    HostJoinInfoDb              m_HostJoinInfoDb;
    HostJoinedLastDb            m_HostJoinedLastDb;
    bool						m_Initialized{ false };
 
    std::map<GroupieId, HostJoinInfo*>	m_HostJoinInfoList;
    VxMutex						m_HostJoinInfoListMutex;
    bool                        m_HostJoinListInitialized{ false };

    std::vector<HostJoinCallbackInterface *> m_HostJoinClients;
    VxMutex						m_HostJoinClientMutex;
};

