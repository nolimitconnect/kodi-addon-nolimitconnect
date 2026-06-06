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

#include <GuiInterface/IDefs.h>

#include <CoreLib/DbBase.h>
#include <CoreLib/GroupieId.h>
#include <map>

class P2PEngine;
class UserJoinInfo;
class UserJoinMgr;

class UserJoinInfoDb : public DbBase
{
public:
    UserJoinInfoDb( P2PEngine& engine, UserJoinMgr& mgr, const char*dbName );
    virtual ~UserJoinInfoDb() = default;

    bool						userJoinInfoDbStartup( int dbVersion, const char* dbFileName );
    bool						userJoinInfoDbShutdown( void );

    bool						addUserJoin( UserJoinInfo * hostInfo );
    void						removeUserJoin( GroupieId& groupieId );

    void						getAllUserJoins( std::map<GroupieId, UserJoinInfo*>& userHostList );
    void						purgeAllUserJoins( void ); 

protected:

    bool						addUserJoin(    GroupieId&      groupieId,
                                                VxGUID&			thumbId,
                                                uint64_t		infoModTime,                                    
                                                EJoinState      joinState,
                                                uint64_t		lastConnectMs,
                                                uint64_t		lastJoinMs,
                                                EFriendState    friendState,
                                                uint32_t        hostFlags,
                                                std::string     hostUrl
                                            );

    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );
    void						insertUserJoinInTimeOrder( UserJoinInfo * hostInfo, std::map<GroupieId, UserJoinInfo*>& assetList );

    P2PEngine&					m_Engine;
    UserJoinMgr&				m_UserJoinMgr;
};

