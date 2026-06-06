#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <CoreLib/DbBase.h>
#include <map>

class IgnoredHostInfo;
class IgnoreListMgr;
class P2PEngine;
class VxGUID;

class IgnoredHostsDb : public DbBase
{
public:
    IgnoredHostsDb() = delete;
    IgnoredHostsDb( P2PEngine& engine, IgnoreListMgr& mgr, const char* dbname );
    virtual ~IgnoredHostsDb() = default;

    bool                        saveToDatabase( IgnoredHostInfo& hostInfo );
    bool                        removeFromDatabase( VxGUID& onlineId );

    bool                        restoreFromDatabase( std::map<VxGUID, IgnoredHostInfo>& ignoredHostList );

protected:
    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );

    P2PEngine&					m_Engine;
    IgnoreListMgr&				m_IgnoreListMgr;
};

