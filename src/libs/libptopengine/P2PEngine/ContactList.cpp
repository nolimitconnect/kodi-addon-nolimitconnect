//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ContactList.h"

#include <BigListLib/BigListInfo.h>

//============================================================================
ContactList::ContactList()
{
}

//============================================================================
ContactList::~ContactList()
{

}

//============================================================================
void ContactList::addContactInfo( VxConnectInfo& connectInfo )
{
	bool bFound = false;
	std::vector<ConnectRequest>::iterator iter;
	m_ContactListMutex.lock();
	for( iter = m_ContactList.begin(); iter != m_ContactList.end(); ++iter )
	{
		if( (*iter).getMyOnlineId() == connectInfo.getMyOnlineId() )
		{
			(*iter) = connectInfo;
			bFound = true;
			break;
		}
	}

	if( false == bFound )
	{
		m_ContactList.push_back( ConnectRequest(connectInfo) );
	}

	m_ContactListMutex.unlock();
}

//============================================================================
void ContactList::removeContactInfo( VxConnectInfo& connectInfo )
{
	std::vector<ConnectRequest>::iterator iter;
	m_ContactListMutex.lock();
	for( iter = m_ContactList.begin(); iter != m_ContactList.end(); ++iter )
	{
		if( (*iter).getMyOnlineId() == connectInfo.getMyOnlineId() )
		{
			m_ContactList.erase(iter);
			break;
		}
	}

	m_ContactListMutex.unlock();
}

//============================================================================
bool ContactList::contactIsInList( VxGUID& onlineId )
{
	bool isInList = false;
	std::vector<ConnectRequest>::iterator iter;
	m_ContactListMutex.lock();
	for( iter = m_ContactList.begin(); iter != m_ContactList.end(); ++iter )
	{
		if( (*iter).getMyOnlineId() == onlineId )
		{
			isInList = true;
			break;
		}
	}

	m_ContactListMutex.unlock();

	return isInList;
}
