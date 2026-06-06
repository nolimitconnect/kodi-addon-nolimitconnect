//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "VxBaseSktInfo.h"

//============================================================================
VxBaseSktInfo::VxBaseSktInfo()
{
}

//============================================================================
VxBaseSktInfo::~VxBaseSktInfo()
{
}

//============================================================================
//! return true if file is allready qued to be sent or has been sent
bool VxBaseSktInfo::isFileQuedOrSent( const char* pFileName )
{
	return false;
}

//============================================================================
//! que a file to send
void VxBaseSktInfo::queFileToSend( const char* pLclFileName, const char* pRmtFileName )
{
}

