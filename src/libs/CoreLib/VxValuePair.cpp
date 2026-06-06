//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxValuePair.h"
#include "VxParse.h"

//============================================================================
VxValuePair::VxValuePair( const char* pStr )
{
	if( NULL == pStr )
	{
		return;
	}

	int iColonIdx = -1;
	int iSearchIdx = 0;
	while( 0 != pStr[iSearchIdx] )
	{
		if( ':' == pStr[iSearchIdx] )
		{
			iColonIdx = iSearchIdx;
			break;
		}
		++iSearchIdx;
	}
	if( 0 > iColonIdx )
	{
		return;
	}

	std::string strLine = pStr;
	m_strName = strLine.substr(0, iColonIdx).c_str();
	StdStringTrim( m_strName );
	m_strValue = strLine.substr( iColonIdx + 1, strLine.length() - iColonIdx - 1 ).c_str();
	StdStringTrim( m_strValue );
}

//============================================================================
VxValuePair::VxValuePair( const char* name, const char* value )
{
	setName(name);
	setValue(value);
}

//============================================================================
VxValuePair::VxValuePair( VxValuePair * header )
{
	setName( header->getName() );
	setValue( header->getValue() );
}

//============================================================================
VxValuePair::~VxValuePair()
{

}

//============================================================================
void VxValuePair::setName(const char*name)
{
	m_strName = name;
}

//============================================================================
const char* VxValuePair::getName( void )
{
	return m_strName.c_str();
}

//============================================================================
bool VxValuePair::hasName( void )
{
	return( 0 < m_strName.length() );
}

//============================================================================
void VxValuePair::setValue(const char*value)
{
	m_strValue = value;
}

//============================================================================
const char* VxValuePair::getValue( void )
{
	return m_strValue.c_str();
}
