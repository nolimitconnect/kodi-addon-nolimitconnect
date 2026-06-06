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

#include <ConnectIdListMgr/ConnectIdListCallback.h>

#include <CoreLib/GroupieId.h>

#include <CoreLib/VxMutex.h>

#include <vector>

class MemberActiveCallback;
class P2PEngine;

class MemberActiveMgr : public ConnectIdListCallback
{
public:
    MemberActiveMgr() = default;
    virtual ~MemberActiveMgr() = default;

    void                        memberActiveStartup( void );

    void                        wantMemberActiveCallbacks( MemberActiveCallback* client, bool enable );

    bool                        updateMemberActive( GroupieId& groupieId, bool memberActive ); // return true if new active member

    bool                        isMemberActive( GroupieId& groupieId );
    bool                        isActiveMemberOfAny( VxGUID& onlineId ); // return true if active member of any host we have joined

    std::vector<GroupieId>&     getMemberActiveList( void )         { return m_MemberList; };

    void				        callbackConnectionStatusChange( ConnectId& connectId, bool isOnline ) override;

    bool						isUserJoinedToRelayHost( VxGUID& memberOnlineId, VxGUID& hostOnlineId );

protected:

    void                        lockMemberList( void )              { m_MemberListMutex.lock(); }
    void                        unlockMemberList( void )            { m_MemberListMutex.unlock(); }

    void                        lockClientList( void )              { m_MemberClientsMutex.lock(); }
    void                        unlockClientList( void )            { m_MemberClientsMutex.unlock(); }

    virtual void                announceMemberActive( GroupieId& groupieId, bool memberActive );
    
    VxMutex                     m_MemberListMutex;
    std::vector<GroupieId>      m_MemberList;

    VxMutex                     m_MemberClientsMutex;
    std::vector<MemberActiveCallback*> m_MemberClients;
};

