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

#include "ConnectedListBase.h"
#include <GuiInterface/IDefs.h>

#include <memory>
#include <vector>

class BigListInfo;
class IConnectRequestCallback;
class P2PEngine;

class ConnectedListAll : public ConnectedListBase
{
public:
    ConnectedListAll() = delete;
    ConnectedListAll( P2PEngine& engine );
    virtual ~ConnectedListAll() override = default;

    ConnectedInfo*              getOrAddConnectedInfo( const VxGUID& socketId, BigListInfo* bigListInfo );
    ConnectedInfo*              getConnectedInfo( const VxGUID& socketId, VxGUID onlineId );
    ConnectedInfo*              getAnyConnectedInfo( VxGUID onlineId );
    void                        removeConnectedInfo( const VxGUID& socketId, VxGUID onlineId, bool isLockedList = false );

    bool                        removeConnectedReason( VxGUID& sessionId, VxGUID& onlineId, IConnectRequestCallback* callback, EConnectReason connectReason, std::vector<std::shared_ptr<VxSktBase>>& retUnusedSkts );

    void                        onSktDisconnected( const VxGUID& socketId );

    //=== vars ===//
    P2PEngine&                  m_Engine;
};
