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
#include "IgnoredHostInfo.h"
#include "IgnoredHostsDb.h"

#include <vector>

// maintains a list of ignored sorted by timestamp for faster lookup

class IgnoreListMgr : public IdentListMgrBase
{
    const int IGNORED_HOSTS_LIST_DB_VERSION = 1;

public:
    IgnoreListMgr() = delete;
    IgnoreListMgr( P2PEngine& engine );
    virtual ~IgnoreListMgr() = default;

    void                        ignoredHostsListMgrStartup( std::string& dbFileName );

    bool                        isIgnored( VxGUID& onlineId );
    virtual void                updateIdent( VxGUID& onlineId, int64_t timestamp ) override;
    virtual void                removeIdent( VxGUID& onlineId ) override;

    bool                        hasIgnoredHosts( void )                 { return !m_IgnoredHostList.empty(); }
    bool                        isHostIgnored( VxGUID& onlineId );
    bool                        addHostIgnore( VxGUID& onlineId, std::string hostUrl, std::string hostTitle, VxGUID& thumbId, std::string hostDescription );
    bool                        removeHostIgnore( VxGUID& onlineId );

    std::vector<std::pair<VxGUID, int64_t>>& getIdentList()             { return m_IgnoreIdentList; };
    void                        getIgnoredHostsList( std::map<VxGUID, IgnoredHostInfo>& ignoredHostList );

protected:
    void                        initializeIgnoredHostsIfNeeded( void );

    IgnoredHostsDb              m_IgnoredHostsDb;
    VxMutex                     m_IgnoredHostsMutex;
    bool                        m_IgnoredHostsDbInitialized{ false };

    std::vector<std::pair<VxGUID, int64_t>> m_IgnoreIdentList;
    std::map<VxGUID, IgnoredHostInfo> m_IgnoredHostList;
};

