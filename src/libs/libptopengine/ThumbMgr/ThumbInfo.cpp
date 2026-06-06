//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "ThumbInfo.h"

#include <PktLib/VxSearchDefs.h>

#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <sys/types.h>
#include <sys/stat.h>

//============================================================================
ThumbInfo::ThumbInfo()
: AssetBaseInfo()
{ 
}

//============================================================================
ThumbInfo::ThumbInfo( const ThumbInfo& rhs )
: AssetBaseInfo( rhs )
{
}

//============================================================================
ThumbInfo::ThumbInfo( const AssetBaseInfo& rhs )
: AssetBaseInfo( rhs )
{
}

//============================================================================
ThumbInfo::ThumbInfo( VxGUID& onlineId, int64_t modifiedTime )
: AssetBaseInfo( eAssetTypeThumbnail, onlineId, modifiedTime )
{
}

//============================================================================
ThumbInfo::ThumbInfo( VxGUID& onlineId, VxGUID& assetId, int64_t modifiedTime )
: AssetBaseInfo( eAssetTypeThumbnail, onlineId, assetId, modifiedTime )
{
}

//============================================================================
ThumbInfo::ThumbInfo( const std::string& fileName, const std::string& fileNameAndPath )
    : AssetBaseInfo( eAssetTypeThumbnail, fileName, fileNameAndPath )
{ 
}

//============================================================================
ThumbInfo::ThumbInfo( const std::string& fileName, const std::string& fileNameAndPath, VxGUID& assetId )
	: AssetBaseInfo( eAssetTypeThumbnail, fileName, fileNameAndPath, assetId )
{
}

//============================================================================
ThumbInfo::ThumbInfo( const char* fileName, const char* fileNameAndPath, uint64_t fileLen )
: AssetBaseInfo( eAssetTypeThumbnail, fileName, fileNameAndPath, fileLen )
{
}

//============================================================================
ThumbInfo::ThumbInfo( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId )
	: AssetBaseInfo( eAssetTypeThumbnail, fileName, fileNameAndPath, fileLen, assetId )
{
}

//============================================================================
ThumbInfo::~ThumbInfo()
{
	// LogMsg( LOG_DEBUG, "~ThumbInfo %p %s", this, m_UniqueId.toHexString().c_str() );
}

//============================================================================
ThumbInfo& ThumbInfo::operator=( const ThumbInfo& rhs ) 
{	
	if( this != &rhs )
	{
        *( (AssetBaseInfo*)this ) = rhs;
	}

	return *this;
}
