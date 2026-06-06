//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/ConnectId.h>
#include <CoreLib/PktBlobEntry.h>

//============================================================================
ConnectId::ConnectId( VxGUID& socketId )
    : m_SocketId( socketId )
    , m_GroupieId()
{
}

//============================================================================
ConnectId::ConnectId( VxGUID& socketId, GroupieId& groupieId )
    : m_SocketId( socketId )
    , m_GroupieId( groupieId )
{
}

//============================================================================
ConnectId::ConnectId( VxGUID& socketId, VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType )
    : m_SocketId( socketId )
    , m_GroupieId( groupieOnlineId, hostOnlineId, hostType )
{
}

//============================================================================
ConnectId::ConnectId( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType )
    : m_SocketId()
    , m_GroupieId( groupieOnlineId, hostOnlineId, hostType )
{
}

//============================================================================
ConnectId::ConnectId( VxGUID& groupieOnlineId, HostedId& hostedId )
    : m_SocketId()
    , m_GroupieId( groupieOnlineId, hostedId )
{
}

//============================================================================
ConnectId::ConnectId( const ConnectId& rhs )
    : m_SocketId( rhs.m_SocketId )
    , m_GroupieId( rhs.m_GroupieId )
    , m_IsRelayed( rhs.m_IsRelayed )
{
}

//============================================================================
ConnectId& ConnectId::operator =( const ConnectId& rhs )
{
	if( this != &rhs )
	{
        m_SocketId                  = rhs.m_SocketId;
        m_GroupieId                 = rhs.m_GroupieId;
        m_IsRelayed                 = rhs.m_IsRelayed;
	}

	return *this;
}

//============================================================================
bool ConnectId::operator == ( const ConnectId& rhs ) const
{
    return m_SocketId == rhs.m_SocketId && m_GroupieId == rhs.m_GroupieId && m_IsRelayed == rhs.m_IsRelayed;
}

//============================================================================
bool ConnectId::operator != ( const ConnectId& rhs ) const
{
    return !(*this == rhs);
}

//============================================================================
bool ConnectId::operator < ( const ConnectId& rhs ) const
{
    if( *this == rhs )
    {
        return false;
    }

    if( m_SocketId < rhs.m_SocketId )
    {
        return true;
    }

    if( m_SocketId == rhs.m_SocketId  && m_GroupieId < rhs.m_GroupieId )
    {
        return true;
    }

    if( m_SocketId == rhs.m_SocketId && m_GroupieId == rhs.m_GroupieId && m_IsRelayed < rhs.m_IsRelayed )
    {
        return true;
    }

    return false;
}

//============================================================================
bool ConnectId::operator <= ( const ConnectId& rhs ) const
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
bool ConnectId::operator > ( const ConnectId& rhs ) const
{
    if( *this == rhs )
    {
        return false;
    }

    if( *this < rhs )
    {
        return false;
    }

    return true;
}

//============================================================================
bool ConnectId::operator >= ( const ConnectId& rhs ) const
{
    if( *this == rhs )
    {
        return true;
    }

    if( *this < rhs )
    {
        return false;
    }

    return true;
}

//============================================================================
bool ConnectId::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_SocketId );
    result &= m_GroupieId.addToBlob( blob );
    return result;
}

//============================================================================
bool ConnectId::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_SocketId );
    result &= m_GroupieId.extractFromBlob( blob );
    return result;
}

// returns 0 if equal else -1 if less or 1 if greater
//============================================================================
int ConnectId::compareTo( ConnectId& connectId )
{
    int result = 0;
    if( *this == connectId )
    {
        return 0;
    }

    if( *this < connectId )
    {
        return -1;
    }

    if( *this > connectId )
    {
        return 1;
    }

    return 0;
}

// returns true if guids are same value
//============================================================================
bool ConnectId::isEqualTo( const ConnectId& connectId )
{
    return *this == connectId;
}

// get a description of the plugin id
//============================================================================
std::string ConnectId::describeConnectId( void ) const
{
    std::string desc = "skt id ";
    desc += m_SocketId.toHexString();
    desc += " ";
    desc += m_GroupieId.describeGroupieId();
    desc += m_IsRelayed ? " relayed" : " direct";
    return desc;
}

//============================================================================
EPluginType ConnectId::getHostPluginType( void )
{
    return HostTypeToHostPlugin( getHostType() );
}

//============================================================================
EPluginType ConnectId::getClientPluginType( void )
{
    return HostTypeToClientPlugin( getHostType() );
}
