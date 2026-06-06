//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostUserSessionId.h"
#include <CoreLib/PktBlobEntry.h>

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& socketId )
    : ConnectId( socketId )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& socketId, GroupieId& groupieId )
    : ConnectId( socketId, groupieId )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& socketId, VxGUID& userOnlineId, VxGUID& hostOnlineId, EHostType hostType )
    : ConnectId( socketId, userOnlineId, hostOnlineId, hostType )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& userOnlineId, VxGUID& hostOnlineId, EHostType hostType )
    : ConnectId( userOnlineId, hostOnlineId, hostType )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& socketId, GroupieId& groupieId, VxGUID& sessionId )
    : ConnectId( socketId, groupieId )
    , m_SessionId( sessionId )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( VxGUID& userOnlineId, HostedId& hostedId )
    : ConnectId( userOnlineId, hostedId )
{
}

//============================================================================
HostUserSessionId::HostUserSessionId( const HostUserSessionId& rhs )
    : ConnectId( rhs )
    , m_SessionId( rhs.m_SessionId )
{
}

//============================================================================
HostUserSessionId& HostUserSessionId::operator =( const HostUserSessionId& rhs )
{
	if( this != &rhs )
	{
        *( (ConnectId *)(this) )    = rhs;
        m_SessionId                 = rhs.m_SessionId;
	}

	return *this;
}

//============================================================================
bool HostUserSessionId::operator == ( const HostUserSessionId& rhs ) const
{
    return *( (ConnectId *)(this) ) == rhs && m_SessionId == rhs.m_SessionId;
}

//============================================================================
bool HostUserSessionId::operator != ( const HostUserSessionId& rhs ) const
{
    return !(*this == rhs);
}

//============================================================================
bool HostUserSessionId::operator < ( const HostUserSessionId& rhs ) const
{
    if( *(this) == rhs )
    {
        return false;
    }

    if( *( (ConnectId*)( this ) ) == rhs && m_SessionId < rhs.m_SessionId )
    {
        return true;
    }

    return false;
}

//============================================================================
bool HostUserSessionId::operator <= ( const HostUserSessionId& rhs ) const
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
bool HostUserSessionId::operator > ( const HostUserSessionId& rhs ) const
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
bool HostUserSessionId::operator >= ( const HostUserSessionId& rhs ) const
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
bool HostUserSessionId::addToBlob( PktBlobEntry& blob )
{
    bool result = ConnectId::addToBlob( blob );
    result &= blob.setValue( m_SessionId );
    return result;
}

//============================================================================
bool HostUserSessionId::extractFromBlob( PktBlobEntry& blob )
{
    bool result = ConnectId::extractFromBlob( blob );
    result &= blob.getValue( m_SessionId );
    return result;
}

// returns 0 if equal else -1 if less or 1 if greater
//============================================================================
int HostUserSessionId::compareTo( HostUserSessionId& connectId )
{
    if( *this == connectId )
    {
        return 0;;
    }

    int result = 0;
    if( *this > connectId )
    {
        result = 1;
    }
    else if( *this < connectId )
    {
        result = -1;
    }

    return result;
}

// returns true if guids are same value
//============================================================================
bool HostUserSessionId::isEqualTo( const HostUserSessionId& groupieId )
{
    return *this == groupieId;
}

// get a description of the plugin id
//============================================================================
std::string HostUserSessionId::describeHostUserSessionId( void ) const
{
    std::string desc = "skt id ";
    desc += m_SocketId.toOnlineIdString();
    desc += " ";
    desc += m_GroupieId.describeGroupieId();
    desc += " session ";
    desc += m_SessionId.toHexString().c_str();
    return desc;
}
