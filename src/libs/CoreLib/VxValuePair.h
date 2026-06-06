#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/config_corelib.h>

#include <string>

class VxValuePair
{
public:
	VxValuePair( const char* lineStr);
	VxValuePair( const char* name, const char* value );
	VxValuePair( VxValuePair * header );

	~VxValuePair();

	void						setName( const char* name );
	const char*				getName( void );
	bool						hasName( void );
	void						setValue( const char*value );
	const char*				getValue( void );

private:
	//=== vars ===//
	std::string					m_strName;
	std::string					m_strValue;
};



