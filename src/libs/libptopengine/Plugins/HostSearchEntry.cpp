//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostServerSearchMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginBase.h>
//#include <HostMgr/HostedEntry.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostJoin.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxParse.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
void HostSearchEntry::updateLastRxTime( void )
{
    m_LastRxTime = GetTimeStampMs();
}

//============================================================================
bool HostSearchEntry::announceTimeExpired( int64_t timeNow )
{
    return timeNow - m_LastRxTime > MIN_HOST_RX_UPDATE_TIME_MS;
}

//============================================================================
bool HostSearchEntry::updateHostedInfo( PktHostInviteAnnounceReq* hostAnn )
{
    return m_HostedInfo.fillFromHostInvite( hostAnn );
}

//============================================================================
HostSearchEntry::HostSearchEntry( const HostSearchEntry& rhs )
: m_LastRxTime( rhs.m_LastRxTime )
, m_HostedInfo( rhs.m_HostedInfo )
{
}

//============================================================================
HostSearchEntry& HostSearchEntry::operator=( const HostSearchEntry& rhs )
{
    if( this != &rhs )
    {
        m_LastRxTime = rhs.m_LastRxTime;
        m_HostedInfo = rhs.m_HostedInfo;
    }

    return *this;
}

//============================================================================
bool HostSearchEntry::searchHostedMatch( SearchParams& searchParams, std::string& searchStr )
{
    if( searchParams.getSearchListAll() )
    {
        return true;
    }

    if( !searchStr.empty() )
    {
        if( -1 != CaseInsensitiveFindSubstr( m_HostedInfo.getHostTitle(), searchStr ) )
        {
            return true;
        }

        if( -1 != CaseInsensitiveFindSubstr( m_HostedInfo.getHostDescription(), searchStr ) )
        {
            return true;
        }
    }

    return false;
}
