#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <Connections/IConnectRequest.h>

#include "FriendRequestInfo.h"

#include <CoreLib/VxMutex.h>

class P2PEngine;
class VxGUID;
class VxNetIdent;
class VxPktHdr;
class VxPtopUrl;
class FriendRequestCallbackInterface;
class PktAnnounce;
class PluginBase;

class FriendRequestMgr
{
public:
    FriendRequestMgr() = default;
    virtual ~FriendRequestMgr() = default;

    int32_t                       friendRequestMgrStartup( void );
    int32_t                       friendRequestMgrShutdown( void );

    void                        lockList( void )    { m_FriendRequestInfoMutex.lock(); }
    void                        unlockList( void )  { m_FriendRequestInfoMutex.unlock(); }

    void						lockClientList( void )          { m_FriendRequestClientMutex.lock(); }
    void						unlockClientList( void )        { m_FriendRequestClientMutex.unlock(); }

    void                        addFriendRequestMgrClient( FriendRequestCallbackInterface* client, bool enable );

    void                        removeFriendRequest( VxGUID& requestId );

    void                        onUserOffline( VxGUID& onlineId ) {}; // TODO implement

    bool                        fromGuiQueryFriendRequest( std::vector<std::shared_ptr<FriendRequestInfo>>& friendRequestList, VxGUID& onlineIdIfNullThenAll );
    bool                        fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestTex, EFriendState myFriendshipToHim );

    void                        rxedFriendRequest( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent& netIdent, VxGUID& requestId, std::string& requestMsg );
    void                        friendRequestAcked( VxGUID requestId, bool wasAccepted );

protected:
    void                        announceFriendRequestInfoRemoved( VxGUID& friendOnlineId, VxGUID& requestId );
    void                        announceFriendRequestInfoUpdated( std::shared_ptr<FriendRequestInfo>& friendRequest );

    VxMutex                     m_FriendRequestInfoMutex;

    std::vector<std::shared_ptr<FriendRequestInfo>>    m_FriendRequestList;

    std::vector<FriendRequestCallbackInterface*> m_FriendRequestClients;
    VxMutex						m_FriendRequestClientMutex;
};

