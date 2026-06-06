#pragma once
//============================================================================
// Copyright (C) 2017 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <string>

class ObjectCommon
{
public:
	ObjectCommon();
	ObjectCommon( std::string nameText );
	virtual ~ObjectCommon(){};

	void						setObjName( std::string nameText );
	const char*					getObjName( void )					    { return m_ObjectName.c_str(); }


protected:
	std::string					m_ObjectName;
};
