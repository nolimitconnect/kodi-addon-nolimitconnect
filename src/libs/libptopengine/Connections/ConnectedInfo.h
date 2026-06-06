//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include "HandshakeInfo.h"

#include <CoreLib/VxMutex.h>

#include <vector>
#include <map>
#include <utility>
#include <memory>

class P2PEngine;
class IConnectRequestCallback;
class BigListInfo;
class VxSktBase;

class ConnectedInfo
{
public:
    ConnectedInfo() = delete;
    ConnectedInfo( P2PEngine& engine, const VxGUID& socketId, BigListInfo* bigListInfo );
    ConnectedInfo( const ConnectedInfo& rhs );
    virtual ~ConnectedInfo() = default;

    ConnectedInfo&				operator=( const ConnectedInfo& rhs );
    bool                        operator==( const ConnectedInfo& rhs );

    P2PEngine&                  getEngine()             { return m_Engine; }
    std::shared_ptr<VxSktBase>  getSktBase( void );
    VxGUID&                     getSocketId( void )     { return m_SocketId; }

    void                        onHandshakeComplete( HandshakeInfo& shakeInfo );
    void                        addConnectReason( HandshakeInfo& shakeInfo );
    bool                        removeConnectReason( VxGUID& sessionId, IConnectRequestCallback* callback, EConnectReason connectReason, std::shared_ptr<VxSktBase>& retSktBaseIfDisconnected );

    void                        onSktDisconnected( const VxGUID& socketId );
    void                        aboutToDelete( void );

protected:
    P2PEngine&                  m_Engine;
    VxGUID                      m_SocketId;
    BigListInfo*                m_BigListInfo{ nullptr };
    VxGUID                      m_PeerOnlineId;
    VxMutex                     m_CallbackListMutex;
    std::vector<HandshakeInfo>  m_CallbackList;
};



