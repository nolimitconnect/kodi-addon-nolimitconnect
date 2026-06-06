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

#include <CoreLib/VxTextStreamReader.h>
#include <CoreLib/VxBuffer.h>

class VxHttpParse : public VxTextStreamReader 
{
public:
	VxHttpParse();
	virtual ~VxHttpParse();

	VxBuffer&					getContentsBuffer( void );

	virtual bool				parseHttpHeaderFromContents();

	virtual const char*		getHttpValue( const char*name, std::string& strRetValue );

	virtual const char*		getHost( std::string& strRetValue );
	virtual const char*		getServer( std::string& strRetValue );
	virtual const char*		getLocation( std::string& strRetValue );
	virtual const char*		getCacheControl( std::string& strRetValue );
	virtual int					getHttpContentLen();
	virtual int					getHttpCodeNumber( void );
	std::string					getMethod( void );
	bool						isHttp( void );
	std::string					readUntil( char const*& str, char delim, char const* end );

	//=== vars ===//
	VxBuffer					m_ContentsBuffer;
};

