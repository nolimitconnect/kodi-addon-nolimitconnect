//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "BaseSessionInfo.h"

#include <CoreLib/VxDebug.h>
#include <PktLib/VxCommon.h>

//============================================================================
BaseSessionInfo::BaseSessionInfo()
{ 
}

//============================================================================
BaseSessionInfo::BaseSessionInfo( HostUserSessionId& hostUserSessionId )
    : HostUserSessionId( hostUserSessionId )
{ 
}

//============================================================================
BaseSessionInfo::BaseSessionInfo( const BaseSessionInfo& rhs )
    : HostUserSessionId( rhs )
    , m_OnlineState( rhs.m_OnlineState )
    , m_JoinState( rhs.m_JoinState )
{
}

//============================================================================
BaseSessionInfo& BaseSessionInfo::operator=( const BaseSessionInfo& rhs )
{
    if( this != &rhs )
    {
        getHostUserSessionId() = rhs;
        m_OnlineState = rhs.m_OnlineState;
        m_JoinState = rhs.m_JoinState;
    }

    return *this;
}

//============================================================================
bool BaseSessionInfo::operator==( const BaseSessionInfo& rhs )
{
    return getHostUserSessionId() == rhs;
}
