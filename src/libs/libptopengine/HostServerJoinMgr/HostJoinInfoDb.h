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
class HostJoinInfo;
class HostServerJoinMgr;
class VxGUID;

class HostJoinInfoDb : public DbBase
{
public:
    HostJoinInfoDb( P2PEngine& engine, HostServerJoinMgr& mgr, const char*dbName );
    virtual ~HostJoinInfoDb() = default;

    void						lockHostJoinInfoDb( void ) { m_HostJoinInfoDbMutex.lock(); }
    void						unlockHostJoinInfoDb( void ) { m_HostJoinInfoDbMutex.unlock(); }

    bool						addHostJoin( HostJoinInfo * hostInfo );
    void						removeHostJoin( GroupieId& groupieId );

    void						getAllHostJoins( std::map<GroupieId, HostJoinInfo*>& userHostList );
    void						purgeAllHostJoins( void ); 

protected:

    bool						addHostJoin( GroupieId& groupieId,
                                                VxGUID&			thumbId,
                                                uint64_t		infoModTime,                             
                                                enum EJoinState joinState,
                                                uint64_t		lastConnectMs,
                                                uint64_t		lastJoinMs,
                                                enum EFriendState    friendState,
                                                uint32_t        hostFlags,
                                                std::string     hostUrl
                                            );

    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );
    void						insertHostJoinInTimeOrder( HostJoinInfo * hostInfo, std::map<GroupieId, HostJoinInfo*>& assetList );

    P2PEngine&					m_Engine;
    HostServerJoinMgr&				m_HostJoinMgr;
    VxMutex						m_HostJoinInfoDbMutex;
};

