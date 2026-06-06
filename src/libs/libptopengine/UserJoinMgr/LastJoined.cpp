//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "LastJoined.h"

#include <CoreLib/VxDebug.h>

//============================================================================
void LastJoined::setLastJoined( GroupieId& lastJoinedGroupie )
{
	if( lastJoinedGroupie.isValid() )
	{
		switch( lastJoinedGroupie.getHostType() )
		{
		case eHostTypeChatRoom:
			m_LastJoinedChatRoom = lastJoinedGroupie;
			m_LastHostType = eHostTypeChatRoom;
			break;
		case eHostTypeGroup:
			m_LastJoinedGroup = lastJoinedGroupie;
			m_LastHostType = eHostTypeGroup;
			break;
		case eHostTypeRandomConnect:
			m_LastJoinedRandomConnect = lastJoinedGroupie;
			m_LastHostType = eHostTypeRandomConnect;
			break;
		default:
			LogMsg( LOG_ERROR, "LastJoined::getLastJoined invalid host type" );
			break;
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "LastJoined::getLastJoined invalid groupie id" );
	}
}

//============================================================================
GroupieId LastJoined::getLastJoined( EHostType hostType )
{
	switch( m_LastHostType )
	{
	case eHostTypeChatRoom:
		return m_LastJoinedChatRoom;
	case eHostTypeGroup:
		return m_LastJoinedGroup;
	case eHostTypeRandomConnect:
		return m_LastJoinedRandomConnect;
	default:
		break;
	}

	LogMsg( LOG_ERROR, "LastJoined::getLastJoined invalid host type" );
	GroupieId emptyId;
	return emptyId;
}

//============================================================================
GroupieId LastJoined::getLastJoined( void )
{
	return getLastJoined( m_LastHostType );
}