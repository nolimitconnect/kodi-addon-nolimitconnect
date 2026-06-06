//============================================================================
// Copyright (C) 2016 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxGUIDList.h"

#include <CoreLib/VxDebug.h>

//============================================================================
VxGUIDList::VxGUIDList()
: m_GuidList()
{
}

//============================================================================
VxGUIDList::VxGUIDList( const VxGUIDList& rhs )
    : m_GuidList( rhs.m_GuidList )
{
}

//============================================================================
VxGUIDList& VxGUIDList::operator =( const VxGUIDList& rhs )
{
    if( this != &rhs )
    {
        m_GuidList          = rhs.m_GuidList;
    }

    return *this;
}

//============================================================================
void VxGUIDList::addGuid( const VxGUID& guid )
{
	m_GuidList.push_back( guid );
}

//============================================================================
bool VxGUIDList::addGuidIfDoesntExist( const VxGUID& guid )
{
	if( doesGuidExist( guid ) )
	{
		return false;
	}
	
	addGuid( guid );
	return true;
}

//============================================================================
bool VxGUIDList::doesGuidExist( const VxGUID& guid )
{
	std::vector<VxGUID>::iterator iter;
	for( iter = m_GuidList.begin(); iter != m_GuidList.end(); ++iter )
	{
		if( (*iter).isEqualTo( guid ) )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool VxGUIDList::removeGuid( VxGUID& guid )
{
	bool guidExisted = false;
	std::vector<VxGUID>::iterator iter;
	for( iter = m_GuidList.begin(); iter != m_GuidList.end(); ++iter )
	{
		if( (*iter).isEqualTo( guid ) )
		{
			guidExisted = true;
			m_GuidList.erase( iter );
			break;
		}
	}

	return guidExisted;
}

//============================================================================
void VxGUIDList::clearList( void )
{
	m_GuidList.clear();
}

//============================================================================
void VxGUIDList::copyTo( VxGUIDList& destGuidList )
{
	std::vector<VxGUID>& guidList = destGuidList.getGuidList();
	for( auto& guid : m_GuidList )
	{
		guidList.emplace_back( guid );
	}
}

//============================================================================
void VxGUIDList::updateLastActiveTime( void )
{
    setLastActiveTime( GetTimeStampMs() );
}

//============================================================================
VxGUID VxGUIDList::getAnyGuid( void )
{
	if( !isEmpty() )
	{
		return m_GuidList.front();
	}
	else
	{
		return VxGUID::nullVxGUID();
	}
}