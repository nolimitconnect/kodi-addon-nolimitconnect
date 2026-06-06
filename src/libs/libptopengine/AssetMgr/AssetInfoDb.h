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

#include <AssetBase/AssetBaseInfoDb.h>

class AssetBaseMgr;

class AssetInfoDb : public AssetBaseInfoDb
{
public:
	AssetInfoDb( AssetBaseMgr& mgr, const char* dbName );
	virtual ~AssetInfoDb() = default;

protected:
    virtual AssetBaseInfo*     createAssetInfo( enum EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen ) override;
    virtual AssetBaseInfo*     createAssetInfo( AssetBaseInfo& assetInfo ) override;

};

