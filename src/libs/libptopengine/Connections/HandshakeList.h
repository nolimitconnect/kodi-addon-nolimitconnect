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

#include <GuiInterface/IDefs.h>

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxMutex.h>

#include <map>
#include <memory>
#include <vector>

class HandshakeInfo;
class VxSktBase;
class IConnectRequestCallback;

class HandshakeList
{
public:
    HandshakeList() = default;
    virtual ~HandshakeList() = default;

    void                        addHandshake( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sessionId, VxGUID onlineId, 
                                              IConnectRequestCallback* callback, EConnectReason connectReason, EHostType hostType );
    void                        removeHandshake( std::shared_ptr<VxSktBase>& sktBase ); // for removal before even used

    void                        getAndRemoveHandshakeInfo( const VxGUID& socketId, VxGUID onlineId, std::vector<HandshakeInfo>& shakeList, std::vector<HandshakeInfo>& timedOutList );
    void                        removeHandshakeInfo( const VxGUID& socketId, const VxGUID& sessionId );
    void                        removeHandshakeSession( const VxGUID& sessionId );
    void                        onSktDisconnected( const VxGUID& socketId );

protected:
    void                        getAndRemoveHandshakeInfo( std::shared_ptr<VxSktBase>& sktBase, std::vector<HandshakeInfo>& disconnectedList );

    //=== vars ===//
    std::map<std::pair<VxGUID,VxGUID>, HandshakeInfo> m_ShakeList; // pair is sockedId and sessionId.. NOT onlineId
};
