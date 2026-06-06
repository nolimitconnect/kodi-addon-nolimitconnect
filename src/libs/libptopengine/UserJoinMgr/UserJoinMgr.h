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

#include "LastJoined.h"

#include "UserJoinInfoDb.h"
#include "UserJoinedLastDb.h"

#include <CoreLib/VxMutex.h>

#include <map>
#include <memory>

/**
* client side manager of user join to host service states
*/
class BaseSessionInfo;
class UserJoinInfo;
class UserJoinCallbackInterface;
class P2PEngine;
class VxSktBase;
class VxNetIdent;

class UserJoinMgr : public LastJoined
{
    const int USER_JOIN_DB_VERSION = 1;
    const int USER_JOINED_LAST_DB_VERSION = 1;
public:
	UserJoinMgr( P2PEngine& engine, const char* dbName, const char* dbJoinedLastName );
	virtual ~UserJoinMgr() = default;

    void                        fromGuiUserLoggedOn( void );
    
    bool                        fromGuiLeaveHost( HostedId& adminId, bool unjoinAlso = false );
    bool                        fromGuiUnJoinHost( HostedId& adminId );

    void                        callbackOnlineStatusChange( VxGUID& onlineId, bool isOnline );

    bool                        getLastJoinedHostUrlFromDb( EHostType hostType, std::string& retHostUrl );
    bool                        getLastJoinedGroupieIdFromDb( EHostType hostType, GroupieId& retGroupieId );

    void                        addUserJoinMgrClient( UserJoinCallbackInterface * client, bool enable );

    void                        onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onUserLeftHost( GroupieId& groupieId );
    void                        onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );

    virtual void                onConnectionLost( std::shared_ptr<VxSktBase>& sktBase, VxGUID& connectionId, VxGUID& peerOnlineId );

    virtual void				announceUserJoinRequested( UserJoinInfo* userHostInfo );
    virtual void				announceUserJoinUpdated( UserJoinInfo * userJoinInfo );
    virtual void				announceUserUnJoin( UserJoinInfo* userJoinInfo );
    virtual void				announceUserJoinRemoved( GroupieId& groupieId );

    virtual void				announceUserJoinAHostStatus( EHostType hostType, VxGUID& sessionId, EConnectStatus connectStatus );

    VxMutex&					getResourceMutex( void )					{ return m_ResourceMutex; }
    void						lockResources( void )						{ m_ResourceMutex.lock(); }
    void						unlockResources( void )						{ m_ResourceMutex.unlock(); }

    UserJoinInfo*               findUserJoinInfo( GroupieId& groupieId );

    void                        changeJoinState( GroupieId& groupieId, EJoinState joinState );
    EJoinState                  getUserJoinState( GroupieId& groupieId );
    void                        queryUserListFromHost( GroupieId& groupieId );

    bool                        deleteDatabase( void );

protected:
    void						clearUserJoinInfoList( void );

    void						lockClientList( void )						{ m_UserJoinClientMutex.lock(); }
    void						unlockClientList( void )					{ m_UserJoinClientMutex.unlock(); }

    bool                        saveToDatabase( UserJoinInfo* joinInfo, bool isLocked = false );
    void                        removeFromDatabase( GroupieId& groupieId, bool isLocked );

    bool                        saveToJoinedLastDatabase( UserJoinInfo* joinInfo, bool isLocked = false );
    void                        removeFromJoinedLastDatabase( GroupieId& groupieId, bool isLocked );

    void                        updateUserIsJoined( UserJoinInfo * userJoinInfo );

    P2PEngine&					m_Engine;
    UserJoinInfoDb              m_UserJoinInfoDb;
    UserJoinedLastDb            m_UserJoinedLastDb;
    VxMutex						m_ResourceMutex;
    bool						m_Initialized{ false };
 
    std::map<GroupieId, UserJoinInfo*>	m_UserJoinInfoList;
    VxMutex						m_UserJoinInfoMutex;
    bool                        m_UserJoinListInitialized{ false };

    std::vector<UserJoinCallbackInterface *> m_UserJoinClients;
    VxMutex						m_UserJoinClientMutex;
};
