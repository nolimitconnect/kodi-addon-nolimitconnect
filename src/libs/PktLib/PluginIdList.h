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

#include <CoreLib/PluginId.h>
#include <vector>

class PluginIdList
{
public:
	PluginIdList() = default;
	~PluginIdList() = default;

    //! copy constructor
    PluginIdList( const PluginIdList& rhs );
    //! copy operator
    PluginIdList& operator =( const PluginIdList& rhs );

	int							size( void )						    { return (int)m_PluginIdList.size(); }

	void						addPluginId( const PluginId& guid );
	// returns false if id already exists
	bool						addPluginIdIfDoesntExist( const PluginId& guid );
	// return true if id is in list
	bool						doesPluginIdExist( const PluginId& guid );
	// returns false if id did not exists
	bool						removePluginId( PluginId& guid );
	void						clearList( void );

	std::vector<PluginId>&		getPluginIdList( void )					{ return m_PluginIdList; }
	void						copyTo( PluginIdList& destPluginIdList );

    void                        setLastActiveTime( uint64_t timeMs )    { m_LastActiveTimeMs = timeMs; }
    uint64_t                    getLastActiveTime( void )               { return m_LastActiveTimeMs; }
    void                        updateLastActiveTime( void );

protected:
	std::vector<PluginId>		m_PluginIdList;
    uint64_t                    m_LastActiveTimeMs{ 0 };
};
