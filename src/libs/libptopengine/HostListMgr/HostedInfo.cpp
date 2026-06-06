//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostedInfo.h"

#include <PktLib/PktsHostInvite.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

//============================================================================
HostedInfo::HostedInfo( EHostType hostType, VxGUID& onlineId, std::string& hostUrl, VxGUID& thumbId )
    : m_AdminId( onlineId, hostType )
    , m_HostInviteUrl( hostUrl )
    , m_ThumbId( thumbId )
{
}

//============================================================================
HostedInfo::HostedInfo( HostedId& adminId, std::string& hostUrl )
: m_AdminId( adminId )
{
    m_HostInviteUrl = hostUrl;
}

//============================================================================
HostedInfo::HostedInfo( const HostedInfo& rhs )
    : m_AdminId( rhs.m_AdminId )
    , m_ConnectedTimestampMs( rhs.m_ConnectedTimestampMs )
    , m_JoinedTimestampMs( rhs.m_JoinedTimestampMs )
    , m_HostInfoTimestampMs( rhs.m_HostInfoTimestampMs )
    , m_IsFavorite( rhs.m_IsFavorite )
    , m_HostInviteUrl( rhs.m_HostInviteUrl )
    , m_HostTitle( rhs.m_HostTitle )
    , m_HostDesc( rhs.m_HostDesc )
    , m_ThumbId( rhs.m_ThumbId )
{
}

//============================================================================
HostedInfo& HostedInfo::operator=( const HostedInfo& rhs ) 
{	
	if( this != &rhs )
	{
        m_AdminId = rhs.m_AdminId;
        m_ConnectedTimestampMs = rhs.m_ConnectedTimestampMs;
        m_JoinedTimestampMs = rhs.m_JoinedTimestampMs;
        m_HostInfoTimestampMs = rhs.m_HostInfoTimestampMs;
        m_IsFavorite = rhs.m_IsFavorite;
        m_HostInviteUrl = rhs.m_HostInviteUrl;
        m_HostTitle = rhs.m_HostTitle;
        m_HostDesc = rhs.m_HostDesc;
        m_ThumbId = rhs.m_ThumbId;
    }

	return *this;
}

//============================================================================
bool HostedInfo::shouldSaveToDb( void )
{
    return m_IsFavorite || m_JoinedTimestampMs;
}

//============================================================================
bool HostedInfo::isValidForGui( void )
{
    return !m_HostInviteUrl.empty() && !m_HostTitle.empty() && !m_HostDesc.empty();
}

//============================================================================
int HostedInfo::getSearchBlobSpaceRequirement( void )
{
    // the +3 is for string \0 terminators
    return sizeof( int64_t ) + m_HostInviteUrl.length() + m_HostTitle.length() + m_HostDesc.length() + 3 * 5; // each string requires null terminator and 4 byte length of data in the blob
}

//============================================================================
bool HostedInfo::fillSearchBlob( PktBlobEntry& blobEntry )
{
    bool result{ true };
    if( getSearchBlobSpaceRequirement() <= blobEntry.getRemainingStorageLen() )
    {
        result &= blobEntry.setValue( m_HostInfoTimestampMs );
        result &= blobEntry.setValue( m_HostInviteUrl );
        result &= blobEntry.setValue( m_HostTitle );
        result &= blobEntry.setValue( m_HostDesc );  
        result &= blobEntry.setValue( m_ThumbId );
    }
    else
    {
        result = false;
    }

    return result;
}

//============================================================================
bool HostedInfo::extractFromSearchBlob( PktBlobEntry& blobEntry )
{
    bool result = blobEntry.getValue( m_HostInfoTimestampMs );
    result &= blobEntry.getValue( m_HostInviteUrl );
    result &= blobEntry.getValue( m_HostTitle );
    result &= blobEntry.getValue( m_HostDesc );
    result &= blobEntry.getValue( m_ThumbId );
    if( result )
    {
        VxPtopUrl hostUrl( m_HostInviteUrl );

        result = hostUrl.isValid() && m_HostInfoTimestampMs && !m_HostTitle.empty() && !m_HostDesc.empty();
        if( result )
        {
            setAdminOnlineId( hostUrl.getOnlineId() );
            if( eHostTypeUnknown != hostUrl.getHostType() )
            {
                setHostType( hostUrl.getHostType() );
            }
        }
    }

    return result;
}

//============================================================================
bool HostedInfo::fillFromHostInvite( PktHostInviteAnnounceReq* hostAnn )
{
    std::string inviteUrl;
    std::string hostTitle;
    std::string hostDesc;
    int64_t hostTimestampMs{ 0 };
    VxGUID thumbId;

    if( hostAnn->getHostInviteInfo( inviteUrl, hostTitle, hostDesc, hostTimestampMs, thumbId ) )
    {
        VxPtopUrl hostUrl( inviteUrl );
        if( hostUrl.isValid() && hostTimestampMs && !hostTitle.empty() && !hostDesc.empty() )
        {
            setAdminOnlineId( hostUrl.getOnlineId() );
            setHostType( hostAnn->getHostType() );
            setHostInviteUrl( inviteUrl );
            setHostTitle( hostTitle );
            setHostDescription( hostDesc );
            setHostInfoTimestamp( hostTimestampMs );
            setThumbId( thumbId );
            return true;
        }
        else
        {
            LogMsg( LOG_ERROR, "HostedInfo::%s invalid url", __func__ );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "HostedInfo::%s get info failed", __func__ );
    }

    return false;
}

//============================================================================
bool HostedInfo::isHostInviteValid( void )
{
    return m_AdminId.isValid() && m_HostInfoTimestampMs &&!m_HostInviteUrl.empty() && !m_HostTitle.empty() && !m_HostDesc.empty();
}

//============================================================================
EPluginType HostedInfo::getHostPluginType( void )
{
    return HostTypeToHostPlugin( m_AdminId.getHostType() );
}

//============================================================================
EPluginType HostedInfo::getClientPluginType( void )
{
    return HostTypeToClientPlugin( m_AdminId.getHostType() );
}