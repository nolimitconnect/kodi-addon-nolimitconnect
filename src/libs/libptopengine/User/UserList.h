#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "User.h"

#include <CoreLib/VxMutex.h>

class UserList
{
public:
    UserList() = default;
    UserList( const UserList& rhs );
	virtual ~UserList() = default;

    /*
    User					    findUser( VxGUID& userId );
    */
    void 				        addOrUpdateUser( User& user );
    void 				        removeUser( VxGUID& onlineId, VxGUID& sessionId );

protected:

	//=== vars ===//
    VxMutex                     m_UserListMutex;
	std::vector<User>		    m_UserList;
};