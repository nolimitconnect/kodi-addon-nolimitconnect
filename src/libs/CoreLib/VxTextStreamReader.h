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

class VxTextStreamReader
{

public:
	VxTextStreamReader();
	virtual ~VxTextStreamReader() {};

	void				setStreamData( const char* pData );
	const char*		getStreamData( void );
	bool 				readStreamLine( std::string& retStrValue );
	int					readStreamData( std::string& retStrValue, int lenToRead );
	int					skipStreamData( int iSkipLen );
	void				resetStreamReadPosition( void );
	void				unwindStreamReadPosition( int iLen );
	std::string&		getRawData( void ) { return m_strData; }

private:
	//=== vars ===//
	std::string			m_strData;
	int					m_iReadIdx;

};
