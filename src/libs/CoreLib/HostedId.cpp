//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostedId.h"
#include "PktBlobEntry.h"

//============================================================================
HostedId::HostedId( VxGUID& onlineId, EHostType hostType )
    : m_HostOnlineId( onlineId )
    , m_HostType( hostType )
{
}

//============================================================================
HostedId::HostedId( const HostedId& rhs )
    : m_HostOnlineId( rhs.m_HostOnlineId )
    , m_HostType( rhs.m_HostType )
{
}

//============================================================================
HostedId& HostedId::operator =( const HostedId& rhs )
{
	if( this != &rhs )
	{
        m_HostOnlineId          = rhs.m_HostOnlineId;
        m_HostType              = rhs.m_HostType;
	}

	return *this;
}

//============================================================================
bool HostedId::operator == ( const HostedId& rhs ) const
{
    return (m_HostOnlineId == rhs.m_HostOnlineId) && ( m_HostType == rhs.m_HostType );
}

//============================================================================
bool HostedId::operator != ( const HostedId& rhs ) const
{
    return !(*this == rhs);
}

//============================================================================
bool HostedId::operator < ( const HostedId& rhs ) const
{
    return m_HostOnlineId < rhs.m_HostOnlineId || (m_HostOnlineId == rhs.m_HostOnlineId && m_HostType < rhs.m_HostType );
}

//============================================================================
bool HostedId::operator <= ( const HostedId& rhs ) const
{
    if( *this == rhs )
    {
        return true;
    }

    if( *this < rhs )
    {
        return true;
    }

    return false;
}

//============================================================================
bool HostedId::operator > ( const HostedId& rhs ) const
{
    return m_HostOnlineId > rhs.m_HostOnlineId || (m_HostOnlineId == rhs.m_HostOnlineId && m_HostType > rhs.m_HostType );
}

//============================================================================
bool HostedId::operator >= ( const HostedId& rhs ) const
{
    if( *this == rhs )
    {
        return true;
    }

    if( *this > rhs )
    {
        return true;
    }

    return false;
}

//============================================================================
bool HostedId::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_HostOnlineId );
    result &= blob.setValue( m_HostType );
    return result;
}

//============================================================================
bool HostedId::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_HostOnlineId );
    result &= blob.getValue( m_HostType );
    return result;
}

// returns 0 if equal else -1 if less or 1 if greater
//============================================================================
int HostedId::compareTo( HostedId& pluginId )
{
    int result = 0;
    if( m_HostType > pluginId.m_HostType )
    {
        result = 1;
    }
    else if( m_HostType < pluginId.m_HostType )
    {
        result = -1;
    }

    if( 0 == result )
    {
        result = m_HostOnlineId.compareTo( pluginId.getHostOnlineId() );
    }

    return result;
}

// returns true if guids are same value
//============================================================================
bool HostedId::isEqualTo( const HostedId& pluginId )
{
    return *this == pluginId;
}

// get a description of the plugin id
//============================================================================
std::string HostedId::describeHostedId( void ) const
{
    std::string desc = DescribeHostType( getHostType() );
    desc += m_HostOnlineId.toOnlineIdString();
    return desc;
}

//============================================================================
EPluginType HostedId::getHostPluginType( void )
{
    return HostTypeToHostPlugin( getHostType() );
}

//============================================================================
EPluginType HostedId::getClientPluginType( void )
{
    return HostTypeToClientPlugin( getHostType() );
}
