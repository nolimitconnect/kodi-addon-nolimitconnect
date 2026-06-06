//============================================================================
// Copyright (C) 2017 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "config_corelib.h"

#include "ObjectCommon.h"

//============================================================================
ObjectCommon::ObjectCommon()
: m_ObjectName( "UNKNOWN" )
{
}

//============================================================================
ObjectCommon::ObjectCommon( std::string nameText )
: m_ObjectName( nameText )
{
}

//============================================================================
void ObjectCommon::setObjName( std::string nameText )
{
	m_ObjectName = nameText;
}

