//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetMgr.h"
#include "AssetInfo.h"

//============================================================================
AssetMgr::AssetMgr( P2PEngine& engine, const char* dbName, const char* dbStateName )
: AssetBaseMgr( engine, dbName, dbStateName, eAssetMgrTypeAssets )
{
}

//============================================================================
AssetBaseInfo* AssetMgr::createAssetInfo( EAssetType assetType, const char* assetName, const char* fileNameAndPath, uint64_t assetLen )
{
    AssetInfo* assetInfo = new AssetInfo( assetType, assetName, fileNameAndPath, assetLen );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* AssetMgr::createAssetInfo( EAssetType assetType, const char* assetName, const char* fileNameAndPath, uint64_t assetLen, VxGUID& assetId )
{
    AssetInfo* assetInfo = new AssetInfo( assetType, assetName, fileNameAndPath, assetLen, assetId );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* AssetMgr::createAssetInfo( AssetBaseInfo& assetInfo )
{
    AssetInfo* assetInfoNew = new AssetInfo( assetInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}

//============================================================================
AssetBaseInfo* AssetMgr::createAssetInfo( FileInfo& fileInfo )
{
    AssetInfo* assetInfoNew = new AssetInfo( fileInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}