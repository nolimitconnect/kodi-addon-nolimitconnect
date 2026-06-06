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

class FileInfo;

class AssetInfo : public AssetBaseInfo
{
public:
	AssetInfo();
    AssetInfo( enum EAssetType assetType );
	AssetInfo( const AssetInfo& rhs );
    AssetInfo( const AssetBaseInfo& rhs );
    AssetInfo( const VxFileInfoBase& rhs );
    AssetInfo( FileInfo& rhs );
    AssetInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath );
    AssetInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, VxGUID& assetId );
    AssetInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen );
    AssetInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen, VxGUID& assetId );

	AssetInfo&					operator=( const AssetInfo& rhs ); 

};
