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
#include "AssetBaseXferDb.h"
#include "AssetBaseInfo.h"

#include <CoreLib/VxDebug.h>

namespace
{
	std::string 		TABLE_ASSET_XFER	 			= "asset_xfer";
	std::string 		CREATE_COLUMNS_ASSET_XFER		= " (unique_id TEXT PRIMARY KEY ) ";
}

//============================================================================
AssetBaseXferDb::AssetBaseXferDb( std::string stateDbName )
: DbBase( stateDbName )
{
}

//============================================================================
//! create tables in database 
int32_t AssetBaseXferDb::onCreateTables( int iDbVersion )
{
	lockAssetBaseXferDb();
    std::string strCmd = "CREATE TABLE " + TABLE_ASSET_XFER + CREATE_COLUMNS_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockAssetBaseXferDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t AssetBaseXferDb::onDeleteTables( int iOldVersion ) 
{
	lockAssetBaseXferDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockAssetBaseXferDb();
	return rc;
}

//============================================================================
void AssetBaseXferDb::purgeAllAssetBaseXfer( void ) 
{
	lockAssetBaseXferDb();
    std::string strCmd = "DELETE FROM " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec( strCmd );
	unlockAssetBaseXferDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "AssetBaseXferDb::purgeAllAssetBaseXfer error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "AssetBaseXferDb::purgeAllAssetBaseXfer success" );
	}
}

//============================================================================
void AssetBaseXferDb::removeAsset( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );
}

//============================================================================
void AssetBaseXferDb::addAsset( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );

	int32_t rc  = sqlExec( "INSERT INTO asset_xfer (unique_id) values(?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "AssetBaseXferDb::addAsset error %d", rc );
	}
}

//============================================================================
void AssetBaseXferDb::getAllAssets( std::vector<VxGUID>& assetList )
{
	assetList.clear();
	std::string strId;
	VxGUID uniqueId;
	lockAssetBaseXferDb();
	DbCursor * cursor = startQuery( "SELECT * FROM asset_xfer" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			strId = cursor->getString( 0 );
			uniqueId.fromVxGUIDHexString( strId.c_str() );
			assetList.emplace_back( uniqueId );
		}

		cursor->close();
	}

	unlockAssetBaseXferDb();
}


