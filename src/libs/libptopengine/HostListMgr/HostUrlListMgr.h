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

#include <Connections/IConnectRequest.h>

#include "HostUrlInfo.h"
#include "HostUrlListDb.h"

#include <CoreLib/VxMutex.h>

class P2PEngine;
class VxGUID;
class VxNetIdent;

class HostUrlListMgr : public IConnectRequestCallback
{
    const int HOST_URL_LIST_DB_VERSION = 1;
public:
    HostUrlListMgr() = delete;
    HostUrlListMgr( P2PEngine& engine );
    ~HostUrlListMgr() = default;

    int32_t                       hostUrlListMgrStartup( std::string& dbFileName );
    int32_t                       hostUrlListMgrShutdown( void );

    void                        lockList( void )    { m_HostUrlsMutex.lock(); }
    void                        unlockList( void )  { m_HostUrlsMutex.unlock(); }

    void                        updateHostUrl( enum EHostType hostType, VxGUID& hostGuid, std::string& hostUrl, int64_t timestampMs = 0 );
    bool                        getHostUrls( enum EHostType hostType, std::vector<HostUrlInfo>& retHostUrls );
    bool                        getResolvedHostUrl( EHostType hostType, VxGUID& hostOnlineId, std::string& retHostUrl );

    void                        requestIdentity( std::string& url );
    void                        updateHostUrls( VxNetIdent* netIdent, int64_t timestampMs = 0 );

protected:
    virtual bool                onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, ERunTestStatus testStatus,
                                        EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

    virtual bool                onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};
    virtual void                onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    virtual bool                onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, EConnectStatus connectStatus,
                                        enum EConnectReason connectReason = eConnectReasonUnknown, enum ECommErr commErr = eCommErrNone ) override {};
    virtual void                onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    void						removeClosedPortIdent( VxGUID& onlineId );

    P2PEngine&                  m_Engine;
    VxMutex                     m_HostUrlsMutex;
    HostUrlListDb               m_HostUrlListDb;

    std::vector<HostUrlInfo>    m_HostUrlsList;
};

