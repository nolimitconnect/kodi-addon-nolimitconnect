//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "BlobMgr.h"
#include "BlobInfo.h"
#include "BlobInfoDb.h"

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsFileList.h>

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>

#include <time.h>

//============================================================================
BlobMgr::BlobMgr( P2PEngine& engine, const char* dbName, const char* stateDbName )
: AssetBaseMgr( engine, dbName, stateDbName, eAssetMgrTypeBlob )
{
}

//============================================================================
AssetBaseInfo* BlobMgr::createAssetInfo( EAssetType assetType, const char* assetName, const char* fileNameAndPath, uint64_t assetLen )
{
    BlobInfo* assetInfo = new BlobInfo( assetType, assetName, fileNameAndPath, assetLen );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* BlobMgr::createAssetInfo( EAssetType assetType, const char* assetName, const char* fileNameAndPath, uint64_t assetLen, VxGUID& assetId )
{
    BlobInfo* assetInfo = new BlobInfo( assetType, assetName, fileNameAndPath, assetLen, assetId );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* BlobMgr::createAssetInfo( AssetBaseInfo& assetInfo )
{
    BlobInfo* assetInfoNew = new BlobInfo( assetInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}

//============================================================================
AssetBaseInfo* BlobMgr::createAssetInfo( FileInfo& fileInfo )
{
    BlobInfo* assetInfoNew = new BlobInfo( fileInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}