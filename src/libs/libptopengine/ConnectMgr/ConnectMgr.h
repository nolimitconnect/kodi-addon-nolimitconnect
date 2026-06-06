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

#include "ConnectInfoDb.h"

#include <CoreLib/VxMutex.h>

#include <memory>

class ConnectInfo;
class ConnectCallbackInterface;
class P2PEngine;
class VxPtopUrl;
class VxSktBase;

class ConnectMgr 
{
    const int CONNECT_MGR_DB_VERSION = 1;
public:
	ConnectMgr( P2PEngine& engine, const char* dbName, const char* stateDbName );
	virtual ~ConnectMgr() = default;

    void                        fromGuiUserLoggedOn( void );
    bool                        isConnectedToHost( enum EHostType hostType, VxPtopUrl& hostUrl, std::shared_ptr<VxSktBase>& sktBase );

    void                        addConnectMgrClient( ConnectCallbackInterface * client, bool enable );

    virtual void				announceConnectAdded( ConnectInfo * userHostInfo );
    virtual void				announceConnectUpdated( ConnectInfo * userHostInfo );
    virtual void				announceConnectRemoved( VxGUID& hostOnlineId );
    virtual void				announceConnectOfferState( VxGUID& hostOnlineId, enum EOfferState userHostOfferState );
    virtual void				announceConnectOnlineState( VxGUID& hostOnlineId, enum  EOnlineState onlineState, VxGUID& connectionId );

    VxMutex&					getResourceMutex( void )					{ return m_ResourceMutex; }
    void						lockResources( void )						{ m_ResourceMutex.lock(); }
    void						unlockResources( void )						{ m_ResourceMutex.unlock(); }

    ConnectInfoDb&              getConnectInfoDb( void )                    { return m_ConnectInfoDb; }

protected:
    void						clearConnectInfoList( void );

    void						lockClientList( void )						{ m_ConnectClientMutex.lock(); }
    void						unlockClientList( void )					{ m_ConnectClientMutex.unlock(); }

    P2PEngine&					m_Engine;
    ConnectInfoDb               m_ConnectInfoDb;
    VxMutex						m_ResourceMutex;
    bool						m_Initialized{ false };
 
    std::vector<ConnectInfo*>	m_ConnectInfoList;
    VxMutex						m_ConnectInfoMutex;
    bool                        m_ConnectListInitialized{ false };

    std::vector<ConnectCallbackInterface *> m_ConnectClients;
    VxMutex						m_ConnectClientMutex;
};

