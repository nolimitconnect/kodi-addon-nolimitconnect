//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/config_corelib.h>
#include "OsDetect.h"
#include "VxDebug.h"

#ifdef TARGET_OS_WINDOWS
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
# pragma warning( disable : 4996 ) //  'GetVersionExA': was declared deprecated
#endif // TARGET_OS_WINDOWS

//============================================================================
std::string OsDetect::getOsName( void )
{
    std::string osName;
    if( isAndroid() )
    {
        osName = "Android";
    }
    else if( isWindowsPlatform() )
    {
        osName = "Windows";
    }
    else if( isLinux() )
    {
        osName = "Linux";
    }
    else if( isApple() )
    {
        osName = "Apple";
    }
    else if( isRasberryPi() )
    {
        osName = "RasberryPi";
    }
    else
    {
        osName = "UnknownOS";
    }

    return osName;
}

//============================================================================
std::string OsDetect::getCpuName( void )
{
    std::string cpu;
#if defined(TARGET_CPU_ARM64) || defined(TARGET_CPU_ARM32)
	cpu += "ARM ";
# if defined(TARGET_CPU_ARM64) 
    cpu += "64 Bit ";
# else
    cpu += "32 Bit ";
# endif 
#else
    cpu += "X86 ";
# if ARCH_32_BITS
    cpu += "32 Bit ";
# else
    cpu += "64 Bit ";
# endif 

#endif // defined(TARGET_CPU_ARM64) || defined(TARGET_CPU_ARM32)
    return cpu;
}

//============================================================================
bool OsDetect::isAndroid( void )
{
#ifdef TARGET_OS_ANDROID
	return true;
#endif // TARGET_OS_ANDROID

	return false;
}

//============================================================================
bool OsDetect::isLinux( void )
{
    if( !isAndroid()
        && !isWindowsPlatform()
        && !isApple()
        && !isRasberryPi() )
    {
        // assume some flavor of linux
        return true;
    }

    return false;
}

//============================================================================
bool OsDetect::isApple( void )
{
#ifdef TARGET_OS_APPLE
    return true;
#endif

    return false;
}

//============================================================================
bool OsDetect::isRasberryPi( void )
{
#ifdef TARGET_OS_RASPBERRY_PI
    return true;
#endif

    return false;
}

//============================================================================
bool OsDetect::isWindowsPlatform( void )
{
#ifdef TARGET_OS_WINDOWS
	return true;
#else
	return false;
#endif // TARGET_OS_WINDOWS
}


//Version Number    Description
//6.1               Windows 7     / Windows 2008 R2
//6.0               Windows Vista / Windows 2008
//5.2               Windows 2003 
//5.1               Windows XP
//5.0               Windows 2000

//============================================================================
double OsDetect::getWindowsVersionNumber( void )
{
#ifdef TARGET_OS_WINDOWS
	OSVERSIONINFO verInfo;
	memset( &verInfo, 0, sizeof( verInfo ) );
	verInfo.dwOSVersionInfoSize = sizeof(verInfo);
	BOOL result = GetVersionEx( &verInfo );
	if( result )
	{
		char buf[ 32 ];
		sprintf( buf, "%d.%d", verInfo.dwMajorVersion, verInfo.dwMinorVersion );
		return atof( buf );
	}

	int32_t rc = VxGetLastError();
	LogMsg( LOG_ERROR, "OsDetect::getWindowsVersionNumber failed with error %d", rc );
	return 0;
#else
	LogMsg( LOG_ERROR, "OsDetect::getWindowsVersionNumber called from unknown OS" );
	return 0;
#endif // TARGET_OS_WINDOWS
}

//============================================================================
bool OsDetect::isWindows7( void )
{
#ifdef TARGET_OS_WINDOWS
	return ( 6.1 <= getWindowsVersionNumber() );
#else
	return false;
#endif // TARGET_OS_WINDOWS
}

//============================================================================
bool OsDetect::isVista( void )
{
#ifdef TARGET_OS_WINDOWS
	return ( 6.0 == getWindowsVersionNumber() );
#else
	return false;
#endif // TARGET_OS_WINDOWS
}

//============================================================================
bool OsDetect::isWindows2003( void )
{
#ifdef TARGET_OS_WINDOWS
	return ( 5.2 == getWindowsVersionNumber() );
#else
	return false;
#endif // TARGET_OS_WINDOWS
}

//============================================================================
bool OsDetect::isXP( void )
{
#ifdef TARGET_OS_WINDOWS
	return ( 5.1 == getWindowsVersionNumber() );
#else
	return false;
#endif // TARGET_OS_WINDOWS
}
