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
#include <CoreLib/VxGUID.h>

class P2PEngine;
class ConnectInfo;
class ConnectMgr;
class VxGUID;

class ConnectInfoDb : public DbBase
{
public:
   ConnectInfoDb( P2PEngine& engine,ConnectMgr& mgr, const char*dbName );
    virtual ~ConnectInfoDb() = default;

    void						lockConnectInfoDb( void ) { m_ConnectInfoDbMutex.lock(); }
    void						unlockConnectInfoDb( void ) { m_ConnectInfoDbMutex.unlock(); }

    void						addConnect(     enum EHostType  hostType,
                                                VxGUID&			hostOnlineId,  
                                                uint64_t		hostModTime,
                                                uint32_t        hostFlags,
                                                VxGUID&			thumbId,
                                                uint64_t		thumbModTime,
                                                VxGUID&			offerId,
                                                enum EOfferState offerState,
                                                uint64_t		offerModTime,
                                                std::string     hostUrl
                                                );

    void						addConnect(ConnectInfo * hostInfo );
    void						removeConnect( VxGUID& hostOnlineId );

    void						getAllConnects( std::vector<ConnectInfo*>& userHostList );
    void						purgeAllConnects( void ); 

protected:

    virtual int32_t				onCreateTables( int iDbVersion );
    virtual int32_t				onDeleteTables( int iOldVersion );
    void						insertConnectInTimeOrder(ConnectInfo * hostInfo, std::vector<ConnectInfo*>& assetList );

    P2PEngine&					m_Engine;
    ConnectMgr&				    m_ConnectMgr;
    VxMutex						m_ConnectInfoDbMutex;
};

