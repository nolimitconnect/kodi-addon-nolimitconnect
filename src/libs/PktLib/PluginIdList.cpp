//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginIdList.h"

#include <CoreLib/VxParse.h>
#include <CoreLib/VxDebug.h>

#include <string.h>
#include <stdio.h>

//============================================================================
PluginIdList::PluginIdList( const PluginIdList& rhs )
    : m_PluginIdList( rhs.m_PluginIdList )
    , m_LastActiveTimeMs( rhs.m_LastActiveTimeMs )
{
}

//============================================================================
PluginIdList& PluginIdList::operator =( const PluginIdList& rhs )
{
    if( this != &rhs )
    {
        m_PluginIdList          = rhs.m_PluginIdList;
        m_LastActiveTimeMs      = rhs.m_LastActiveTimeMs;
    }

    return *this;
}

//============================================================================
void PluginIdList::addPluginId( const PluginId& guid )
{
	m_PluginIdList.push_back( guid );
}

//============================================================================
bool PluginIdList::addPluginIdIfDoesntExist( const PluginId& guid )
{
	if( doesPluginIdExist( guid ) )
	{
		return false;
	}
	
	addPluginId( guid );
	return true;
}

//============================================================================
bool PluginIdList::doesPluginIdExist( const PluginId& guid )
{
	std::vector<PluginId>::iterator iter;
	for( iter = m_PluginIdList.begin(); iter != m_PluginIdList.end(); ++iter )
	{
		if( (*iter).isEqualTo( guid ) )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool PluginIdList::removePluginId( PluginId& guid )
{
	bool guidExisted = false;
	std::vector<PluginId>::iterator iter;
	for( iter = m_PluginIdList.begin(); iter != m_PluginIdList.end(); ++iter )
	{
		if( (*iter).isEqualTo( guid ) )
		{
			guidExisted = true;
			m_PluginIdList.erase( iter );
			break;
		}
	}

	return guidExisted;
}

//============================================================================
void PluginIdList::clearList( void )
{
	m_PluginIdList.clear();
}

//============================================================================
void PluginIdList::copyTo( PluginIdList& destPluginIdList )
{
	std::vector<PluginId>& guidList = destPluginIdList.getPluginIdList();
	std::vector<PluginId>::iterator iter;
	for( iter = m_PluginIdList.begin(); iter != m_PluginIdList.end(); ++iter )
	{
		guidList.push_back( *iter );
	}
}

//============================================================================
void PluginIdList::updateLastActiveTime( void )
{
    setLastActiveTime( GetTimeStampMs() );
}
