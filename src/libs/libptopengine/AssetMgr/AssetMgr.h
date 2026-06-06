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

#include <AssetBase/AssetBaseMgr.h>
#include <AssetMgr/AssetXferMgr.h>

class AssetMgr : public AssetBaseMgr
{
public:
	AssetMgr( P2PEngine& engine, const char* dbName, const char* dbStateName );
	virtual ~AssetMgr() = default;

protected:
	AssetBaseInfo*				createAssetInfo( AssetBaseInfo& assetInfo ) override;
	AssetBaseInfo*				createAssetInfo( FileInfo& fileInfo ) override;

	AssetBaseInfo*				createAssetInfo( enum EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen ) override;
    AssetBaseInfo*				createAssetInfo( enum EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId ) override;

};

