//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginId.h"
#include "PktBlobEntry.h"

//============================================================================
PluginId::PluginId( VxGUID& onlineId, EPluginType pluginType )
    : m_OnlineId(onlineId)
    , m_PluginType(pluginType)
{
}

//============================================================================
PluginId::PluginId( const PluginId& rhs )
    : m_OnlineId(rhs.m_OnlineId)
    , m_PluginType(rhs.m_PluginType)
{
}

//============================================================================
PluginId& PluginId::operator =( const PluginId& rhs )
{
	if( this != &rhs )
	{
        m_OnlineId              = rhs.m_OnlineId;
        m_PluginType            = rhs.m_PluginType;
	}

	return *this;
}

//============================================================================
bool PluginId::operator == ( const PluginId& rhs ) const
{
    return ( m_OnlineId == rhs.m_OnlineId ) && ( m_PluginType == rhs.m_PluginType );
}

//============================================================================
bool PluginId::operator != ( const PluginId& rhs ) const
{
    return !(*this == rhs);
}

//============================================================================
bool PluginId::operator < ( const PluginId& rhs ) const
{
    if( m_PluginType < rhs.m_PluginType )
        return true;
    if( m_PluginType > rhs.m_PluginType )
        return false;

    if( m_OnlineId < rhs.m_OnlineId )
        return true;
    return false;
}

//============================================================================
bool PluginId::operator <= ( const PluginId& rhs ) const
{
    if( *this == rhs )
    {
        return true;
    }

    if( m_PluginType < rhs.m_PluginType )
        return true;
    if( m_PluginType > rhs.m_PluginType )
        return false;

    if( m_OnlineId < rhs.m_OnlineId )
        return true;

    return false;
}

//============================================================================
bool PluginId::operator > ( const PluginId& rhs ) const
{
    if( m_PluginType > rhs.m_PluginType )
        return true;
    if( m_PluginType < rhs.m_PluginType )
        return false;

    if( m_OnlineId > rhs.m_OnlineId )
        return true;

    return false;
}

//============================================================================
bool PluginId::operator >= ( const PluginId& rhs ) const
{
    if( *this == rhs )
    {
        return true;
    }

    if( m_PluginType > rhs.m_PluginType )
        return true;
    if( m_PluginType < rhs.m_PluginType )
        return false;

    if( m_OnlineId > rhs.m_OnlineId )
        return true;

    return false;
}

//============================================================================
bool PluginId::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_OnlineId );
    result &= blob.setValue( m_PluginType );
    return result;
}

//============================================================================
bool PluginId::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_OnlineId );
    result &= blob.getValue( m_PluginType );
    return result;
}

// returns 0 if equal else -1 if less or 1 if greater
//============================================================================
int PluginId::compareTo( PluginId& pluginId )
{
    int result = 0;
    if( m_PluginType > pluginId.m_PluginType )
    {
        result = 1;
    }
    else if( m_PluginType < pluginId.m_PluginType )
    {
        result = -1;
    }

    if( 0 == result )
    {
        result = m_OnlineId.compareTo( pluginId.getOnlineId() );
    }

    return result;
}

// returns true if guids are same value
//============================================================================
bool PluginId::isEqualTo( const PluginId& pluginId )
{
    return *this == pluginId;
}

// get a description of the plugin id
//============================================================================
std::string PluginId::describePluginId( void ) const
{
    std::string desc = DescribePluginType( getPluginType() );
    desc += m_OnlineId.toOnlineIdString();
    return desc;
}
