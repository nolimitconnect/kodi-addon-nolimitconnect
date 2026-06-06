//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "UserJoinInfo.h"

#include <PktLib/VxSearchDefs.h>

#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <sys/types.h>
#include <sys/stat.h>

//============================================================================
UserJoinInfo::UserJoinInfo()
    : BaseJoinInfo()
{ 
}

//============================================================================
UserJoinInfo::UserJoinInfo( const UserJoinInfo& rhs )
    : BaseJoinInfo( rhs )
    , m_NetIdent( rhs.m_NetIdent )
    , m_HostFlags( rhs.m_HostFlags )
    , m_GroupieId( rhs.m_GroupieId )
    , m_HostUrl( rhs.m_HostUrl )
    , m_ConnectionId( rhs.m_ConnectionId )
    , m_SessionId( rhs.m_SessionId )
{
}

//============================================================================
UserJoinInfo& UserJoinInfo::operator=( const UserJoinInfo& rhs ) 
{	
	if( this != &rhs )
	{
        BaseJoinInfo::operator = ( rhs );
        m_NetIdent = rhs.m_NetIdent;
        m_HostFlags = rhs.m_HostFlags;
        m_GroupieId = rhs.m_GroupieId;
        m_HostUrl = rhs.m_HostUrl;
        m_ConnectionId = rhs.m_ConnectionId;
        m_SessionId = rhs.m_SessionId;
    }

	return *this;
}
