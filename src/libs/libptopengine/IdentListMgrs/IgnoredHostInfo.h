#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxGUID.h>

class IgnoredHostInfo
{
public:
    IgnoredHostInfo() = default;
    IgnoredHostInfo( VxGUID& onlineId, VxGUID& thumbId, std::string hostUrl, std::string hostTitle, std::string hostDescription, int64_t timestampMs );

    IgnoredHostInfo( const IgnoredHostInfo& rhs );

    virtual ~IgnoredHostInfo() = default;

    IgnoredHostInfo& operator=( const IgnoredHostInfo& rhs );

    void                        setOnlineId( VxGUID& onlineId )                 { m_OnlineId = onlineId; }
    VxGUID&                     getOnlineId()                                   { return m_OnlineId; }
    void                        setThumbId( VxGUID& thumbId )                   { m_ThumbId = thumbId; }
    VxGUID&                     getThumbId( void )                              { return m_ThumbId; }
    void                        setHostUrl( std::string& hostUrl )              { m_HostUrl = hostUrl; }
    std::string&                getHostUrl( void )                              { return m_HostUrl; }
    void                        setHostTitle( std::string& hosTitle )           { m_HostTitle = hosTitle; }
    std::string&                getHostTitle( void )                            { return m_HostTitle; }
    void                        setHostDescription( std::string& hosDescription ) { m_HostDescription = hosDescription; }
    std::string&                getHostDescription( void )                      { return m_HostDescription; }
    void                        setTimestampMs( int64_t timestampMs )           { m_TimestampMs = timestampMs; }
    int64_t                     getTimestampMs( void )                          { return m_TimestampMs; }

protected:
    VxGUID                      m_OnlineId;
    VxGUID                      m_ThumbId;
    std::string                 m_HostUrl;
    std::string                 m_HostTitle;
    std::string                 m_HostDescription;
    int64_t                     m_TimestampMs{ 0 };
};