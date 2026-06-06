//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostUrlInfo.h"

//============================================================================
HostUrlInfo::HostUrlInfo( EHostType hostType, VxGUID& onlineId, std::string& hostUrl, int64_t timestamp )
    : m_HostType( hostType )
    , m_OnlineId( onlineId )
    , m_HostUrl( hostUrl )
    , m_TimestampMs( timestamp )
{
}

//============================================================================
HostUrlInfo::HostUrlInfo( const HostUrlInfo& rhs )
    : m_HostType( rhs.m_HostType )
    , m_OnlineId( rhs.m_OnlineId )
    , m_HostUrl( rhs.m_HostUrl )
    , m_TimestampMs( rhs.m_TimestampMs )
{
}

//============================================================================
HostUrlInfo& HostUrlInfo::operator=( const HostUrlInfo& rhs ) 
{	
	if( this != &rhs )
	{
        m_HostType = rhs.m_HostType;
        m_OnlineId = rhs.m_OnlineId;
        m_HostUrl = rhs.m_HostUrl;
        m_TimestampMs = rhs.m_TimestampMs;
    }

	return *this;
}
