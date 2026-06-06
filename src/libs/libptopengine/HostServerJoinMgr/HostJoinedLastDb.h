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
class HostServerJoinMgr;
class VxGUID;

class HostJoinedLastDb : public DbBase
{
public:
    HostJoinedLastDb( P2PEngine& engine, HostServerJoinMgr& mgr, const char*dbName );
    virtual ~HostJoinedLastDb() = default;

    void						lockHostJoinedLastDb( void ) { m_HostJoinedLastDbMutex.lock(); }
    void						unlockHostJoinedLastDb( void ) { m_HostJoinedLastDbMutex.unlock(); }

    bool						setJoinedLast( enum EPluginType pluginType, VxGUID& onlineId, int64_t lastJoinMs, std::string hostUrl );
    bool						getJoinedLast( enum EPluginType pluginType, VxGUID& onlineId, int64_t& lastJoinMs, std::string& hostUrl );

    void						removeJoinedLast( enum EPluginType pluginType );
    void						purgeAllHJoinedLast( void );

protected:

    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );

    P2PEngine&					m_Engine;
    HostServerJoinMgr&				m_HostJoinMgr;
    VxMutex						m_HostJoinedLastDbMutex;
};

