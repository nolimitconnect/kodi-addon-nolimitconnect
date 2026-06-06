//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxGuidPairList.h"

#include <CoreLib/VxDebug.h>

//============================================================================
VxGuidPairList::VxGuidPairList()
: m_GuidPairList()
{
}

//============================================================================
VxGuidPairList::VxGuidPairList( const VxGuidPairList& rhs )
    : m_GuidPairList( rhs.m_GuidPairList )
{
}

//============================================================================
VxGuidPairList& VxGuidPairList::operator =( const VxGuidPairList& rhs )
{
    if( this != &rhs )
    {
        m_GuidPairList          = rhs.m_GuidPairList;
    }

    return *this;
}

//============================================================================
void VxGuidPairList::addGuid( const VxGUID& guid1, const VxGUID& guid2 )
{
	m_GuidPairList.push_back( std::make_pair(guid1, guid2));
}

//============================================================================
bool VxGuidPairList::addGuidIfDoesntExist( const VxGUID& guid1, const VxGUID& guid2 )
{
	if( doesGuidExist( guid1, guid2 ) )
	{
		return false;
	}
	
	addGuid( guid1, guid2 );
	return true;
}

//============================================================================
bool VxGuidPairList::doesGuidExist( const VxGUID& guid1, const VxGUID& guid2 )
{
	std::pair<VxGUID, VxGUID> guidPair( std::make_pair( guid1, guid2 ) );
    for( auto iter = m_GuidPairList.begin(); iter != m_GuidPairList.end(); ++iter )
	{
		if( (*iter).first == guid1 && ( *iter ).second == guid2 )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool VxGuidPairList::removeGuid( const VxGUID& guid1, const VxGUID& guid2 )
{
	bool guidExisted = false;
    for( auto iter = m_GuidPairList.begin(); iter != m_GuidPairList.end(); ++iter )
	{
		if( ( *iter ).first == guid1 && ( *iter ).second == guid2 )
		{
			guidExisted = true;
			m_GuidPairList.erase( iter );
			break;
		}
	}

	return guidExisted;
}

//============================================================================
void VxGuidPairList::clearList( void )
{
	m_GuidPairList.clear();
}
