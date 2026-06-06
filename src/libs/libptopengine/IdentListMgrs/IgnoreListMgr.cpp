//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IgnoreListMgr.h"
#include "IgnoredHostInfo.h"
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>

//============================================================================
IgnoreListMgr::IgnoreListMgr( P2PEngine& engine )
    : IdentListMgrBase( engine )
    , m_IgnoredHostsDb( engine, *this, "IngnoredHosts.db3" )
{
    setIdentListType( eUserViewTypeIgnored );
}

//============================================================================
void IgnoreListMgr::ignoredHostsListMgrStartup( std::string& dbFileName )
{
    m_IgnoredHostsDb.dbShutdown();
    m_IgnoredHostsDb.dbStartup( IGNORED_HOSTS_LIST_DB_VERSION, dbFileName );
}

//============================================================================
bool IgnoreListMgr::isIgnored( VxGUID& onlineId )
{
    bool isIgnored = false;
    lockIdentList();
    for( auto iter = m_IgnoreIdentList.begin(); iter != m_IgnoreIdentList.end(); ++iter )
    {
        if( iter->first == onlineId )
        {
            isIgnored = true;
            break;
        }
    }

    unlockIdentList();
    return isIgnored;
}

//============================================================================
void IgnoreListMgr::updateIdent( VxGUID& onlineId, int64_t timestamp )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "IgnoreListMgr::updateIgnoreIdent invalid id" );
        return;
    }

    if( onlineId == m_Engine.getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, "IgnoreListMgr::updateIgnoreIdent cannot ignore myself" );
        return;
    }

    bool wasInserted = false;
    bool wasErased = false;
    bool timestampUpdated = false;
    lockIdentList();
    for( auto iter = m_IgnoreIdentList.begin(); iter != m_IgnoreIdentList.end(); )
    {
        if( iter->first == onlineId )
        {
            iter = m_IgnoreIdentList.erase( iter );
            wasErased = true;
        }
        else if( !wasInserted )
        {
            if( timestamp > iter->second )
            {
                iter = m_IgnoreIdentList.insert( iter, std::make_pair( onlineId, timestamp ) );
                timestampUpdated = true;
                wasInserted = true;
            }
            else
            {
                ++iter;
            }
        }
        else
        {
            ++iter;
        }

        if( wasErased && wasInserted )
        {
            break;
        }
    }

    if( !wasInserted )
    {
        m_IgnoreIdentList.push_back( std::make_pair( onlineId, timestamp ) );
    }

    unlockIdentList();

    if( timestampUpdated || ( wasInserted && !wasErased ) )
    {
        onUpdateIdent( onlineId, timestamp );
    }
}

//============================================================================
void IgnoreListMgr::removeIdent( VxGUID& onlineId )
{
    bool wasRemoved = false;
    lockIdentList();
    for( auto iter = m_IgnoreIdentList.begin(); iter != m_IgnoreIdentList.end(); ++iter )
    {
        if( iter->first == onlineId )
        {
            m_IgnoreIdentList.erase( iter );
            wasRemoved = true;
            break;
        }
    }

    unlockIdentList();

    if( wasRemoved )
    {
        onRemoveIdent( onlineId );
    }
}

//============================================================================
bool IgnoreListMgr::isHostIgnored( VxGUID& onlineId )
{
    m_IgnoredHostsMutex.lock();
    initializeIgnoredHostsIfNeeded();
    bool isIgnrored = !onlineId.isValid() || m_IgnoredHostList.find( onlineId ) != m_IgnoredHostList.end();
    m_IgnoredHostsMutex.unlock();
    return isIgnrored;
}

//============================================================================
bool IgnoreListMgr::addHostIgnore( VxGUID& onlineId, std::string hostUrl, std::string hostTitle, VxGUID& thumbId, std::string hostDescription )
{
    bool wasAdded{ false };
    if( onlineId.isValid() )
    {
        IgnoredHostInfo hostInfo( onlineId, thumbId, hostUrl, hostTitle, hostDescription, GetGmtTimeMs() );

        m_IgnoredHostsMutex.lock();
        m_IgnoredHostList[ onlineId ] = hostInfo;

        initializeIgnoredHostsIfNeeded();
        wasAdded = m_IgnoredHostsDb.saveToDatabase( hostInfo );
        m_IgnoredHostsMutex.unlock();
    }

    return wasAdded;
}

//============================================================================
bool IgnoreListMgr::removeHostIgnore( VxGUID& onlineId )
{
    bool removedIgnore{ false };
    if( onlineId.isValid() )
    {
        m_IgnoredHostsMutex.lock();
        m_IgnoredHostList.erase( onlineId );
        initializeIgnoredHostsIfNeeded();
        removedIgnore = m_IgnoredHostsDb.removeFromDatabase( onlineId );
        m_IgnoredHostsMutex.unlock();
    }

    return removedIgnore;
}

//============================================================================
void IgnoreListMgr::initializeIgnoredHostsIfNeeded( void )
{
    if( !m_IgnoredHostsDbInitialized )
    {
        m_IgnoredHostsDbInitialized = true;
        m_IgnoredHostsDb.restoreFromDatabase( m_IgnoredHostList );
    }
}

//============================================================================
void IgnoreListMgr::getIgnoredHostsList( std::map<VxGUID, IgnoredHostInfo>& ignoredHostList )
{
    m_IgnoredHostsMutex.lock();
    initializeIgnoredHostsIfNeeded();
    ignoredHostList = m_IgnoredHostList;
    m_IgnoredHostsMutex.unlock();
}