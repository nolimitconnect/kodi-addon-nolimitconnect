//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FriendRequestInfo.h"

#include <CoreLib/VxDebug.h>

//============================================================================
FriendRequestInfo::FriendRequestInfo( VxGUID& onlineId, VxGUID& requestId, std::string& requestText, EFriendRequestState requestState, std::shared_ptr<VxNetIdent> netIdent, int64_t timeModified )
    : m_OnlineId( onlineId )
    , m_RequestId( requestId )
    , m_RequestText( requestText )
    , m_RequestState( requestState )
    , m_RequestTimestampMs( timeModified )
    , m_NetIdent( netIdent )
{
}

//============================================================================
FriendRequestInfo::FriendRequestInfo( const FriendRequestInfo& rhs )
    : m_OnlineId( rhs.m_OnlineId )
    , m_RequestId( rhs.m_RequestId )
    , m_RequestText( rhs.m_RequestText )
    , m_RequestState( rhs.m_RequestState )
    , m_RequestTimestampMs( rhs.m_RequestTimestampMs )
    , m_NetIdent( rhs.m_NetIdent )
{
}

//============================================================================
FriendRequestInfo& FriendRequestInfo::operator=( const FriendRequestInfo& rhs ) 
{	
	if( this != &rhs )
	{
        m_OnlineId = rhs.m_OnlineId;
        m_RequestId = rhs.m_RequestId;
        m_RequestText = rhs.m_RequestText;
        m_RequestState = rhs.m_RequestState;
        m_RequestTimestampMs = rhs.m_RequestTimestampMs;
        m_NetIdent = rhs.m_NetIdent;
    }

	return *this;
}
