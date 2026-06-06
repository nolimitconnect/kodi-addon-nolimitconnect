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
#include "BaseHostInfo.h"

//============================================================================
BaseHostInfo::BaseHostInfo()
    : BaseInfo()
{ 
}

//============================================================================
BaseHostInfo::BaseHostInfo( const BaseHostInfo& rhs )
    : BaseInfo( rhs )
    , m_HostType( rhs.m_HostType )
    , m_ConnectUrl( rhs.m_ConnectUrl )
{
}

//============================================================================
BaseHostInfo& BaseHostInfo::operator=( const BaseHostInfo& rhs )
{
    if( this != &rhs )
    {
        BaseInfo::operator=( rhs );
        m_HostType = rhs.m_HostType;
        m_ConnectUrl = rhs.m_ConnectUrl;
    }

    return *this;
}

//============================================================================
bool BaseHostInfo::isHostMatch( EHostType hostType, VxGUID& onlineId )
{
    return  hostType == m_HostType && onlineId == m_OnlineId;
}