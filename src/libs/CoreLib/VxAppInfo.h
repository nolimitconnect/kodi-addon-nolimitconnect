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

#include <CoreLib/config_corelib.h>
#include <string>


class VxAppInfo
{
public:
	const char*			getAppName( void );
	const char*			getAppNameNoSpaces( void );
    const char*			getAppNameNoSpacesLowerCase( void );
    const char*            getCompanyWebsite( void );
    const char*            getCompanyDomain( void );

	int						getVersionMajor( void );
	int						getVersionMinor( void );
	const char*			getVersionSuffix( void );
    std::string			    getVersionString( void );

	const char*			getSCMID( void );
};

