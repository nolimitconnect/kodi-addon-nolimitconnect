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

#include "HostBaseMgr.h"
#include "HostSearchEntry.h"

#include <HostListMgr/HostedInfo.h>

#include <PktLib/PktsHostInvite.h>

#include <string>
#include <vector>

class HostedEntry;

class HostSearchEntry
{
public:
    HostSearchEntry() = default;
    ~HostSearchEntry() = default;
    HostSearchEntry( const HostSearchEntry& rhs );

    HostSearchEntry&			operator=( const HostSearchEntry& rhs );

    void                        updateLastRxTime( void );
    bool                        announceTimeExpired( int64_t timeNow );
    bool                        updateHostedInfo( PktHostInviteAnnounceReq* hostAnn );

    bool                        searchHostedMatch( SearchParams& searchParams, std::string& searchStr );

    VxGUID&                     getAdminOnlineId( void )         { return m_HostedInfo.getAdminOnlineId(); }

    uint64_t                    m_LastRxTime{ 0 }; // time last recieved announce
    HostedInfo                  m_HostedInfo;
};
