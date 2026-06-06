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

#include <NlcDependLibrariesConfig.h>
#include <string>

class VxHttpValuePair
{
private:
	std::string m_strName;
	std::string m_strValue;

public:
	VxHttpValuePair( const char* lineStr);
	VxHttpValuePair( const char* name, const char* value );
	VxHttpValuePair( VxHttpValuePair * header );

	~VxHttpValuePair();

	void			setName( const char* name );
	const char*	getName( void );
	bool			hasName( void );
	void			setValue( const char*value );
	const char*	getValue( void );
};



