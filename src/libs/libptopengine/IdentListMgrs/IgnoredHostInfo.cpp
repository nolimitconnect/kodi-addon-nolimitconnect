//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IgnoredHostInfo.h"

IgnoredHostInfo::IgnoredHostInfo( VxGUID& onlineId, VxGUID& thumbId, std::string hostUrl, std::string hostTitle, std::string hostDescription, int64_t timestampMs )
    : m_OnlineId( onlineId )
    , m_ThumbId( thumbId )
    , m_HostUrl( hostUrl )
    , m_HostTitle( hostTitle )
    , m_HostDescription( hostDescription )
    , m_TimestampMs( timestampMs )
{
}

IgnoredHostInfo::IgnoredHostInfo( const IgnoredHostInfo& rhs )
    : m_OnlineId( rhs.m_OnlineId )
    , m_ThumbId( rhs.m_ThumbId )
    , m_HostUrl( rhs.m_HostUrl )
    , m_HostTitle( rhs.m_HostTitle )
    , m_HostDescription( rhs.m_HostDescription )
    , m_TimestampMs( rhs.m_TimestampMs )
{
}

IgnoredHostInfo& IgnoredHostInfo::operator=( const IgnoredHostInfo & rhs )
{
    m_OnlineId = rhs.m_OnlineId;
    m_ThumbId = rhs.m_ThumbId;
    m_HostUrl = rhs.m_HostUrl;
    m_HostTitle = rhs.m_HostTitle;
    m_HostDescription = rhs.m_HostDescription;
    m_TimestampMs = rhs.m_TimestampMs;
    return *this;
}

