//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================


#include "HostJoinInfo.h"

#include <PktLib/VxCommon.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxPtopUrl.h>

//============================================================================
HostJoinInfo::HostJoinInfo()
    : BaseJoinInfo()
{ 
}

//============================================================================
HostJoinInfo::HostJoinInfo( const HostJoinInfo& rhs )
    : BaseJoinInfo( rhs )
    , m_NetIdent( rhs.m_NetIdent )
    , m_FriendState( rhs.m_FriendState )
    , m_HostFlags( rhs.m_HostFlags )
    , m_GroupieId( rhs.m_GroupieId )
    , m_UserUrl( rhs.m_UserUrl )
    , m_ConnectionId( rhs.m_ConnectionId )
    , m_SessionId( rhs.m_SessionId )    
{
}

//============================================================================
HostJoinInfo& HostJoinInfo::operator=( const HostJoinInfo& rhs ) 
{	
	if( this != &rhs )
	{
        BaseJoinInfo::operator = ( rhs );
        m_NetIdent = rhs.m_NetIdent;
        m_FriendState = rhs.m_FriendState;
        m_HostFlags = rhs.m_HostFlags;
        m_GroupieId = rhs.m_GroupieId;
        m_UserUrl = rhs.m_UserUrl;
        m_ConnectionId = rhs.m_ConnectionId;
        m_SessionId = rhs.m_SessionId;
    }

	return *this;
}

//============================================================================
bool HostJoinInfo::isUrlValid( void )
{
    VxPtopUrl ptopUrl( m_UserUrl );
    return ptopUrl.isValid() && ptopUrl.getOnlineId() == getOnlineId();
}

//============================================================================
std::string HostJoinInfo::describeHostJoin( void )
{
    std::string desc = m_GroupieId.describeGroupieId();
    desc += " ";
    if( m_NetIdent )
    {
        desc += m_NetIdent->getOnlineName();
        desc += " state ";
    }
    else
    {
        desc += "null NetIdent state ";
    }
    
    desc += DescribeJoinState( getJoinState() );
    return desc;
}