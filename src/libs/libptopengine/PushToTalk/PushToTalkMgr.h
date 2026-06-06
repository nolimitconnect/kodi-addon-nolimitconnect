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

#include "PushToTalkCallback.h"

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxMutex.h>

#include <map>
#include <vector>

class PushToTalkMgr
{
public:
    PushToTalkMgr() = default;
    virtual ~PushToTalkMgr() = default;

    void                        wantPushToTalkCallbacks( PushToTalkCallback* client, bool enable );

    virtual void                pushToTalkStatusChange( VxGUID& onlineId, enum EPushToTalkStatus pushToTalkStatus );
 
    EPushToTalkStatus           getPushToTalkStatus( VxGUID& onlineId );

protected:
    void                        lockClientList( void )      { m_PushToTalkClientsMutex.lock(); }
    void                        unlockClientList( void )    { m_PushToTalkClientsMutex.unlock(); }

    void                        lockStatusMap( void )      { m_PushToTalkStatusMutex.lock(); }
    void                        unlockStatusMap( void )    { m_PushToTalkStatusMutex.unlock(); }

    void                        setPushToTalkStatus( VxGUID& onlineId, EPushToTalkStatus pushToTalkStatus );

    VxMutex                     m_PushToTalkClientsMutex;
    std::vector<PushToTalkCallback*> m_PushToTalkClients;

    VxMutex                     m_PushToTalkStatusMutex;
    std::map<VxGUID, enum EPushToTalkStatus> m_PushToTalkStatusMap;
};

