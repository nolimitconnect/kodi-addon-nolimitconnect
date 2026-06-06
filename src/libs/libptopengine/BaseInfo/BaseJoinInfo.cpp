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
#include "BaseJoinInfo.h"

//============================================================================
BaseJoinInfo::BaseJoinInfo()
    : BaseInfo()
{ 
}

//============================================================================
BaseJoinInfo::BaseJoinInfo( const BaseJoinInfo& rhs )
    : BaseInfo( rhs )
    , m_HostType( rhs.m_HostType )
    , m_JoinState( rhs.m_JoinState )
    , m_LastConnectMs( rhs.m_LastConnectMs )
    , m_LastJoinMs( rhs.m_LastJoinMs )
{
}

//============================================================================
BaseJoinInfo& BaseJoinInfo::operator=( const BaseJoinInfo& rhs )
{
    if( this != &rhs )
    {
        BaseInfo::operator=( rhs );
        m_HostType = rhs.m_HostType;
        m_JoinState = rhs.m_JoinState;
        m_LastConnectMs = rhs.m_LastConnectMs;
        m_LastJoinMs = rhs.m_LastJoinMs;
    }

    return *this;
}
