#pragma once
//============================================================================
// Copyright (C) 2016 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxGUID.h"
#include <vector>

class VxGUIDList
{
public:
	VxGUIDList();
	~VxGUIDList() = default;

    //! copy constructor
    VxGUIDList( const VxGUIDList & rhs );
    //! copy operator
    VxGUIDList& operator =( const VxGUIDList & rhs );

	int							size( void )							{ return (int)m_GuidList.size(); }
	bool						isEmpty( void )							{ return 0 == m_GuidList.size(); }
	VxGUID						front( void )							{ if( !isEmpty() ) return m_GuidList.front(); VxGUID emptyGuid; return emptyGuid; }

	void						addGuid( const VxGUID& guid );
	// returns false if guid already exists
	bool						addGuidIfDoesntExist( const VxGUID& guid );
	// return true if guid is in list
	bool						doesGuidExist( const VxGUID& guid );
	// returns false if guid did not exists
	bool						removeGuid( VxGUID& guid );
	void						clearList( void );

	std::vector<VxGUID>&		getGuidList( void )						{ return m_GuidList; }
	VxGUID						getAnyGuid( void );
	void						copyTo( VxGUIDList& destGuidList );

    void                        setLastActiveTime( uint64_t timeMs )    { m_LastActiveTimeMs = timeMs; }
    uint64_t                    getLastActiveTime( void )               { return m_LastActiveTimeMs; }
    void                        updateLastActiveTime( void );

protected:
	std::vector<VxGUID>			m_GuidList;
    uint64_t                    m_LastActiveTimeMs{ 0 };
};
