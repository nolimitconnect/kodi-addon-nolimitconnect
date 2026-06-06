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

class P2PEngine;
class UserJoinMgr;
class VxGUID;

class UserJoinedLastDb : public DbBase
{
public:
    UserJoinedLastDb( P2PEngine& engine, UserJoinMgr& mgr, const char*dbName );
    virtual ~UserJoinedLastDb() = default;

    void						lockDb( void )    { m_DbMutex.lock(); }
    void						unlockDb( void )  { m_DbMutex.unlock(); }

    bool						setJoinedLastHostType( EHostType hostType );
    bool						getJoinedLastHostType( EHostType& hostType );

    bool						setJoinedLast( EHostType hostType, VxGUID& onlineId, int64_t lastJoinMs, std::string hostUrl );
    bool						getJoinedLast( EHostType pluginType, VxGUID& onlineId, int64_t& lastJoinMs, std::string& hostUrl );

    void						removeJoinedLast( EHostType hostType );
    void						purgeAllJoinedLast( void );

protected:

    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );

    P2PEngine&					m_Engine;
    UserJoinMgr&				m_UserJoinMgr;
};

