#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/GroupieId.h>

class LastJoined
{
public:
	LastJoined() = default;
	~LastJoined() = default;

	void						setLastJoined( GroupieId& lastJoinedGroupie );
	GroupieId					getLastJoined( EHostType hostType );
	GroupieId					getLastJoined( void );
	void						setLastJoinedHostType( EHostType hostType ) { m_LastHostType = hostType; }

protected:
	GroupieId					m_LastJoinedChatRoom;
	GroupieId					m_LastJoinedGroup;
	GroupieId					m_LastJoinedRandomConnect;

	EHostType					m_LastHostType{ eHostTypeUnknown };
};
