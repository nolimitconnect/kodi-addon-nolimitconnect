#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <string>

class OsDetect
{
public:
    static std::string				getOsName( void ); // returns os platform name.. ie Windows or Linux etc
    static std::string				getCpuName( void );
	
    static bool						isAndroid( void );
    static bool						isLinux( void );
    static bool						isApple( void );
    static bool						isRasberryPi( void );
    static bool						isWindowsPlatform( void );

    static double					getWindowsVersionNumber( void );
    static bool						isWindows7( void );
    static bool						isVista( void );
    static bool						isWindows2003( void );
    static bool						isXP( void );
};
