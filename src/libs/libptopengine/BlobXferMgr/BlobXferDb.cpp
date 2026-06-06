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
#include "BlobXferDb.h"
#include "BlobInfo.h"

#include <CoreLib/VxDebug.h>

namespace
{
	std::string 		TABLE_ASSET_XFER	 			= "asset_xfer";
	std::string 		CREATE_COLUMNS_ASSET_XFER		= " (unique_id TEXT PRIMARY KEY ) ";
}

//============================================================================
BlobXferDb::BlobXferDb( const char* stateDbName )
: DbBase( stateDbName )
{
}


//============================================================================
//! create tables in database 
int32_t BlobXferDb::onCreateTables( int iDbVersion )
{
	lockBlobXferDb();
    std::string strCmd = "CREATE TABLE " + TABLE_ASSET_XFER + CREATE_COLUMNS_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockBlobXferDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t BlobXferDb::onDeleteTables( int iOldVersion ) 
{
	lockBlobXferDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockBlobXferDb();
	return rc;
}

//============================================================================
void BlobXferDb::purgeAllBlobXfer( void ) 
{
	lockBlobXferDb();
    std::string strCmd = "DELETE FROM " + TABLE_ASSET_XFER;
    int32_t rc = sqlExec( strCmd );
	unlockBlobXferDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "BlobXferDb::purgeAllBlobXfer error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "BlobXferDb::purgeAllBlobXfer success" );
	}
}

//============================================================================
void BlobXferDb::removeBlob( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );
}

//============================================================================
void BlobXferDb::addBlob( VxGUID& assetUniqueId )
{
	std::string strId;
	assetUniqueId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM asset_xfer WHERE unique_id=?", bindList );

	int32_t rc  = sqlExec( "INSERT INTO asset_xfer (unique_id) values(?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "BlobXferDb::addBlob error %d", rc );
	}
}

//============================================================================
void BlobXferDb::getAllBlobs( std::vector<VxGUID>& assetList )
{
	assetList.clear();
	std::string strId;
	VxGUID uniqueId;
	lockBlobXferDb();
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

	unlockBlobXferDb();
}


