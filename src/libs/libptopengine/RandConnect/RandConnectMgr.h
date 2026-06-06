#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <ConnectIdListMgr/ConnectIdListCallback.h>
#include <Membership/MemberActiveCallback.h>

#include <CoreLib/VxMutex.h>

#include <CoreLib/GroupieId.h>
#include <PktLib/PktsRandConnectDefs.h>

#include <vector>

class RandConnectCallback;
class P2PEngine;

class RandConnectMgr : public ConnectIdListCallback, public MemberActiveCallback
{
public:
    struct RandConnectOfferEntry
    {
        GroupieId                m_GroupieId;
        VxGUID                   m_ToUserOnlineId;
        VxGUID                   m_SessionId;
        ERandAction              m_RandAction{ eRandActionNone };
        uint64_t                 m_TimeRequestedMs{ 0 };
        EOfferType               m_OfferType{ eOfferTypeUnknown };
    };

    RandConnectMgr() = default;
    virtual ~RandConnectMgr() = default;

    void                        onEngineStartup( void );
    void                        onEngineShutdown( void );

    void                        wantRandConnectCallbacks( RandConnectCallback* client, bool enable );

    void				        callbackConnectionStatusChange( ConnectId& connectId, bool isOnline ) override;
    void				        callbackMemberActive( GroupieId& onlineId, bool isActive ) override;

    void                        updateRandConnectStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction );
    void                        updateRandConnectStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction, VxGUID& sessionId, uint64_t timeRequestedMs, EOfferType offerType );

protected:

    void                        lockMemberList( void )              { m_MemberListMutex.lock(); }
    void                        unlockMemberList( void )            { m_MemberListMutex.unlock(); }

    void                        lockClientList( void )              { m_MemberClientsMutex.lock(); }
    void                        unlockClientList( void )            { m_MemberClientsMutex.unlock(); }

    void                        lockOfferList( void )               { m_OfferListMutex.lock(); }
    void                        unlockOfferList( void )             { m_OfferListMutex.unlock(); }

    void                        updateRandConnectStatus( GroupieId& groupieId, enum ERandAction randAction );
    void                        updateRandConnectOfferStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction, VxGUID& sessionId, uint64_t timeRequestedMs, EOfferType offerType );
    bool                        isSelectionAction( enum ERandAction randAction );
    bool                        isTerminalOfferAction( enum ERandAction randAction );

    virtual void                announceRandConnect( GroupieId& groupieId, enum ERandAction randAction );
    virtual void                announceRandConnectOffer( GroupieId& groupieId, VxGUID& toUserOnlineId, VxGUID& sessionId, enum ERandAction randAction, uint64_t timeRequestedMs, EOfferType offerType );
    
    VxMutex                     m_MemberListMutex;
    std::vector<std::pair<GroupieId, ERandAction>>      m_MemberList;

    VxMutex                     m_MemberClientsMutex;
    std::vector<RandConnectCallback*> m_MemberClients;

    VxMutex                     m_OfferListMutex;
    std::vector<RandConnectOfferEntry> m_OfferList;
};

