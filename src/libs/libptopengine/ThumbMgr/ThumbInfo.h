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

#include <AssetBase/AssetBaseInfo.h>

class ThumbInfo : public AssetBaseInfo
{
public:
	ThumbInfo();
	ThumbInfo( const ThumbInfo& rhs );
    ThumbInfo( const AssetBaseInfo& rhs );
    ThumbInfo( VxGUID& onlineId, int64_t modifiedTime = 0 );
    ThumbInfo( VxGUID& onlineId, VxGUID& assetId, int64_t modifiedTime = 0 );
	ThumbInfo( const std::string& fileName, const std::string& fileNameAndPath );
    ThumbInfo( const std::string& fileName, const std::string& fileNameAndPath, VxGUID& assetId );
    ThumbInfo( const char* fileName, const char* fileNameAndPath, uint64_t fileLen );
    ThumbInfo( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId );
    virtual ~ThumbInfo();

	ThumbInfo&				    operator=( const ThumbInfo& rhs ); 

    /// thumb objects use the asset id and ther is no other assciated thumb to this thumb fil
    virtual VxGUID&				getThumbId( void ) override                             { return m_UniqueId; }

public:
	//=== vars ===//

};
