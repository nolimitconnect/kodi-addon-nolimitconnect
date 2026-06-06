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
#include "AssetXferDb.h"
#include "AssetInfo.h"

#include <CoreLib/VxDebug.h>

namespace
{
	std::string 		TABLE_ASSET_XFER	 			= "asset_xfer";
	std::string 		CREATE_COLUMNS_ASSET_XFER		= " (unique_id TEXT PRIMARY KEY ) ";
}

//============================================================================
AssetXferDb::AssetXferDb()
: DbBase( "AssetXferDb" )
{
}

//============================================================================
AssetXferDb::~AssetXferDb()
{
}

//============================================================================
//! create tables in database 
int32_t AssetXferDb::onCreateTables( int iDbVersion )
{
	lockAssetXferDb();
    std::string strCmd = "CREATE TABLE " + TABLE_ASSET_XFER + CREATE_COLUMNS_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockAssetXferDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t AssetXferDb::onDeleteTables( int iOldVersion ) 
{
	lockAssetXferDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockAssetXferDb();
	return rc;
}

//============================================================================
void AssetXferDb::purgeAllAssetXfer( void ) 
{
	lockAssetXferDb();
    std::string strCmd = "DELETE FROM " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec( strCmd );
	unlockAssetXferDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "AssetXferDb::purgeAllAssetXfer error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "AssetXferDb::purgeAllAssetXfer success" );
	}
}

//============================================================================
void AssetXferDb::removeAsset( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );
}

//============================================================================
void AssetXferDb::addAsset( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );

	int32_t rc  = sqlExec( "INSERT INTO asset_xfer (unique_id) values(?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "AssetXferDb::addAsset error %d", rc );
	}
}

//============================================================================
void AssetXferDb::getAllAssets( std::vector<VxGUID>& assetList )
{
	assetList.clear();
	std::string strId;
	VxGUID uniqueId;
	lockAssetXferDb();
	DbCursor * cursor = startQuery( "SELECT * FROM asset_xfer" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			strId = cursor->getString( 0 );
			uniqueId.fromVxGUIDHexString( strId.c_str() );
			assetList.push_back( uniqueId );
		}

		cursor->close();
	}

	unlockAssetXferDb();
}


