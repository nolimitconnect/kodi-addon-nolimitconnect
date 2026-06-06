//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "ConnectInfo.h"

#include <PktLib/VxSearchDefs.h>

#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <sys/types.h>
#include <sys/stat.h>

//============================================================================
ConnectInfo::ConnectInfo()
    : BaseHostInfo()
{ 
}

//============================================================================
ConnectInfo::ConnectInfo( const ConnectInfo& rhs )
    : BaseHostInfo( rhs )
{
}

//============================================================================
ConnectInfo& ConnectInfo::operator=( const ConnectInfo& rhs ) 
{	
	if( this != &rhs )
	{
        BaseHostInfo::operator=( rhs );
	}

	return *this;
}
