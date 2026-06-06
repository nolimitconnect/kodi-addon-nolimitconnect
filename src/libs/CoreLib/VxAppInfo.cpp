
#include "VxAppInfo.h"

#include "VxGlobals.h"
#include "AppVersion.h"

#include <cstddef>

//=============================================================================
const char* VxAppInfo::getAppName( void )
{
	return VxGetApplicationTitle();
}

//=============================================================================
const char* VxAppInfo::getAppNameNoSpaces( void )
{
	return VxGetApplicationNameNoSpaces();
}

//=============================================================================
const char* VxAppInfo::getAppNameNoSpacesLowerCase( void )
{
    return VxGetApplicationNameNoSpacesLowerCase();
}

//=============================================================================
const char* VxAppInfo::getCompanyWebsite( void )
{
    return VxGetCompanyWebsite();
}

//=============================================================================
const char* VxAppInfo::getCompanyDomain( void )
{
    return VxGetCompanyDomain();
}

//=============================================================================
int VxAppInfo::getVersionMajor( void )
{
	return APP_MAJOR_VERSION;
}

//=============================================================================
int VxAppInfo::getVersionMinor( void )
{
	return APP_MINOR_VERSION;
}

//=============================================================================
const char* VxAppInfo::getVersionSuffix( void )
{
	return "";
}

//=============================================================================
std::string VxAppInfo::getVersionString( void )
{
    return APP_VERSION;
}

//=============================================================================
const char* VxAppInfo::getSCMID( void )
{
	return "@APP_SCMID@";
}
