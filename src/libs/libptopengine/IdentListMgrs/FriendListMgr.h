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

#include "IdentListMgrBase.h"

#include <CoreLib/VxGUID.h>

#include <vector>

class FriendListMgr : public IdentListMgrBase
{
public:
    FriendListMgr() = delete;
    FriendListMgr( P2PEngine& engine );
    virtual ~FriendListMgr() = default;

    bool                        isFriend( VxGUID& onlineId );
    void                        updateIdent( VxGUID& onlineId, int64_t timestamp ) override;
    void                        removeIdent( VxGUID& onlineId ) override;

    std::vector<std::pair<VxGUID, int64_t>>& getIdentList() { return m_FriendIdentList; };

protected:
    std::vector<std::pair<VxGUID, int64_t>> m_FriendIdentList;
};

