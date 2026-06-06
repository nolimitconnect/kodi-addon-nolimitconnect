//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "UserList.h"

#include <CoreLib/VxDebug.h>
#include <PktLib/VxCommon.h>

//============================================================================
UserList::UserList( const UserList& rhs )
    : m_UserList( rhs.m_UserList)
{
}

/*
//============================================================================
User UserList::findUser( VxGUID& userId )
{
	for( auto user : m_UserList )
	{
		if( user.getNetIdent() && userId == user.getNetIdent()->getMyOnlineId() )
		{
			return user;
		}
	}

	return nullptr;
}*/

//============================================================================
void UserList::addOrUpdateUser( User& userIn )
{
    if( !userIn.getNetIdent() )
    {
        LogMsg( LOG_ERROR, "UserList::addOrUpdateUser invalid param " );
        return;
    }

    bool foundUser = false;
    VxGUID onlineId = userIn.getNetIdent()->getMyOnlineId();
    //VxGUID sessionId = userIn.getSessionId();
    for( User& user : m_UserList )
    {
        if( user.getNetIdent() && user.getNetIdent()->getMyOnlineId() == onlineId ) // && user.getSessionId() == sessionId )
        {
            user = userIn;
            foundUser = true;
            break;
        }
    }

    if( !foundUser )
    {
        m_UserList.push_back( userIn );
    }
}

//============================================================================
void UserList::removeUser( VxGUID& onlineId, VxGUID& sessionId )
{
    for( auto iter = m_UserList.begin(); iter != m_UserList.end(); ++iter )
    {
        if( iter->getNetIdent() && iter->getNetIdent()->getMyOnlineId() == onlineId ) // && iter->getSessionId() == sessionId )
        {
            m_UserList.erase( iter );
            break;
        }
    }
}
