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

class BlobInfo : public AssetBaseInfo
{
public:
	BlobInfo();
	BlobInfo( const BlobInfo& rhs );
    BlobInfo( const AssetBaseInfo& rhs );
	BlobInfo( EAssetType assetType, const std::string& fileName, const std::string& fileNameAndPath );
	BlobInfo( EAssetType assetType, const std::string& fileName, const std::string& fileNameAndPath, VxGUID& assetId );
    BlobInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen );
	BlobInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId );

	BlobInfo&				    operator=( const BlobInfo& rhs ); 
};
