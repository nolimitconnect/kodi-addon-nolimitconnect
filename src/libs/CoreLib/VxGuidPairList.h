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

#include "VxGUID.h"
#include <vector>
#include <utility>

class VxGuidPairList
{
public:
	VxGuidPairList();
	~VxGuidPairList() = default;

    //! copy constructor
    VxGuidPairList( const VxGuidPairList & rhs );
    //! copy operator
    VxGuidPairList& operator =( const VxGuidPairList & rhs );

	int							size( void )							{ return (int)m_GuidPairList.size(); }

	void						addGuid( const VxGUID& guid1, const VxGUID& guid2 );
	// returns false if guid already exists
	bool						addGuidIfDoesntExist( const VxGUID& guid1, const VxGUID& guid2 );
	// return true if guid is in list
	bool						doesGuidExist( const VxGUID& guid1, const VxGUID& guid2 );
	// returns false if guid did not exists
	bool						removeGuid( const VxGUID& guid1, const VxGUID& guid2 );

	void						clearList( void );

	std::vector<std::pair<VxGUID,VxGUID>>&	getGuidList( void )						{ return m_GuidPairList; }

protected:
	std::vector<std::pair<VxGUID,VxGUID>>	m_GuidPairList;
};
