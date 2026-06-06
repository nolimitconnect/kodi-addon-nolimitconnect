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

#include "SktListMgrBase.h"

#include <CoreLib/VxGUID.h>

#include <vector>


class SktListMgr : public SktListMgrBase
{
public:
    SktListMgr() = delete;
    SktListMgr( P2PEngine& engine );
    virtual ~SktListMgr() = default;

    bool                        isSkt( VxGUID& onlineId );
    virtual void                updateSktIdent( VxGUID& sktConnectId, VxGUID& onlineId, int64_t timestamp );
    virtual void                removeConnection( VxGUID& sktConnectId );
    virtual void                removeIdent( VxGUID& onlineId ) override;
    virtual void                removeAll( void ) override;

    std::vector< std::pair<VxGUID, std::pair<VxGUID, int64_t>>>& getIdentList()         { return m_SktIdentList; };

protected:
    virtual void                removeConnection( VxGUID& onlineId, VxGUID& sktConnectId );

    std::vector< std::pair<VxGUID, std::pair<VxGUID, int64_t>>> m_SktIdentList;
};

