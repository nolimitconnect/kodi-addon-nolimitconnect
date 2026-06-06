#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <Network/ConnectRequest.h>

#include <PktLib/VxCommon.h>
#include <CoreLib/VxMutex.h>

class ContactList
{
public:
	ContactList();
	virtual ~ContactList();

	virtual void				addContactInfo( VxConnectInfo& connectInfo );
	virtual void				removeContactInfo( VxConnectInfo& connectInfo );
	virtual bool				contactIsInList( VxGUID& onlineId );

	//=== vars ===//
	VxMutex						m_ContactListMutex;
	std::vector<ConnectRequest>	m_ContactList;
};

