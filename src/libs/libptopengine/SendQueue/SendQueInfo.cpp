//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SendQueInfo.h"

//============================================================================
SendQueInfo::SendQueInfo( const SendQueInfo& rhs )
    : GroupieId( rhs )
    , m_SendState( rhs.getSendQueState() )
    , m_ModTime( rhs.getModTime() )
{
}

//============================================================================
SendQueInfo::SendQueInfo( VxGUID& userOnlineId, VxGUID& hostOnlineId )
    : GroupieId( userOnlineId, hostOnlineId, eHostTypePeerUser )
{
}

//============================================================================
SendQueInfo::SendQueInfo( VxGUID& userOnlineId, VxGUID& hostOnlineId, enum ESendQueState sendState, int64_t modTime )
    : GroupieId( userOnlineId, hostOnlineId, eHostTypePeerUser )
    , m_SendState( sendState )
    , m_ModTime( modTime )
{
}

//============================================================================
SendQueInfo::SendQueInfo( GroupieId& groupieId )
    : GroupieId( groupieId )
{
}

//============================================================================
SendQueInfo::SendQueInfo( GroupieId& groupieId, enum ESendQueState sendState, int64_t modTime )
    : GroupieId( groupieId )
    , m_SendState( sendState )
    , m_ModTime( modTime )
{
}

//============================================================================
SendQueInfo& SendQueInfo::operator =( const SendQueInfo& rhs )
{
	if( this != &rhs )
	{
        *((GroupieId*)this)     = *((GroupieId*)&rhs);
        m_SendState             = rhs.m_SendState;
        m_ModTime               = rhs.m_ModTime;
	}

	return *this;
}

//============================================================================
bool SendQueInfo::operator == ( const SendQueInfo& rhs ) const
{
    return m_UserOnlineId == rhs.m_UserOnlineId &&  m_HostedId == rhs.m_HostedId;
}

//============================================================================
bool SendQueInfo::operator != ( const SendQueInfo& rhs ) const
{
    return !(*this == rhs);
}

//============================================================================
bool SendQueInfo::operator < ( const SendQueInfo& rhs ) const
{
    return m_UserOnlineId < rhs.m_UserOnlineId || (m_UserOnlineId == rhs.m_UserOnlineId && m_HostedId < rhs.m_HostedId );
}

//============================================================================
bool SendQueInfo::operator <= ( const SendQueInfo& rhs ) const
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
bool SendQueInfo::operator > ( const SendQueInfo& rhs ) const
{
    return m_UserOnlineId > rhs.m_UserOnlineId || (m_UserOnlineId == rhs.m_UserOnlineId && m_HostedId > rhs.m_HostedId );
}

//============================================================================
bool SendQueInfo::operator >= ( const SendQueInfo& rhs ) const
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
std::string SendQueInfo::describeSendQueInfo( void ) const
{
    std::string desc = m_HostedId.describeHostedId();
    desc += " user ";
    desc += m_UserOnlineId.toOnlineIdString();

    return desc;
}
