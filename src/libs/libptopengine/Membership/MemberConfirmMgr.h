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

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxMutex.h>

#include <memory>
#include <vector>

// this class handles the confirm member announce by exchanging PktAnnounce through relay so that
// the correct friendship is shown and queued items will be sent 

class VxSktBase;

class MemberConfirmMgr
{
public:
    MemberConfirmMgr() = default;
    virtual ~MemberConfirmMgr() = default;

    void                        addMemberConfirm( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

    void                        onOncePerSecond( void );

    void                        pktAnnRecieved( VxGUID onlineId );

    void                        onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );

protected:

    void                        lockMemberList( void )              { m_MemberListMutex.lock(); }
    void                        unlockMemberList( void )            { m_MemberListMutex.unlock(); }
    
    VxMutex                     m_MemberListMutex;
    std::vector<std::pair<std::shared_ptr<VxSktBase>, VxGUID>>      m_MemberList;
};

extern MemberConfirmMgr& GetMemberConfirmMgr();