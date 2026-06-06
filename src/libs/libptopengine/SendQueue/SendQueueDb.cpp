//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SendQueueDb.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>

namespace
{
	const int			SEND_QUEUE_DB_VERSION			= 1;
	std::string 		TABLE_SEND_QUEUE	 			= "send_queue";
	std::string 		CREATE_COLUMNS_SEND_QUEUE		= " (onlineId TEXT PRIMARY KEY, assetId TEXT, modTime BIGINT, sendState INTEGER) ";

	const int			SEND_COLUMN_ONLINE_ID			= 0;
	const int			SEND_COLUMN_ASSET_ID			= 1;
	const int			SEND_COLUMN_MOD_TIME			= 2;
	const int			SEND_COLUMN_SEND_STATE			= 3;
}

//============================================================================
SendQueueDb::SendQueueDb()
: DbBase( "SendQueueDb" )
{
}

//============================================================================
void SendQueueDb::sendQueueDbStartup( std::string& dbName )
{
	dbStartup( SEND_QUEUE_DB_VERSION, dbName );
}

//============================================================================
//! create tables in database 
int32_t SendQueueDb::onCreateTables( int iDbVersion )
{
	lockSendQueueDb();
    std::string strCmd = "CREATE TABLE " + TABLE_SEND_QUEUE + CREATE_COLUMNS_SEND_QUEUE;
    int32_t rc = sqlExec(strCmd);
	unlockSendQueueDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t SendQueueDb::onDeleteTables( int iOldVersion ) 
{
	lockSendQueueDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_SEND_QUEUE;
    int32_t rc = sqlExec(strCmd);
	unlockSendQueueDb();
	return rc;
}

//============================================================================
void SendQueueDb::purgeAllSendQueue( void ) 
{
	lockSendQueueDb();
    std::string strCmd = "DELETE FROM " + TABLE_SEND_QUEUE;
    int32_t rc = sqlExec( strCmd );
	unlockSendQueueDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "SendQueueDb::purgeAllSendQueue error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "SendQueueDb::purgeAllSendQueue success" );
	}
}

//============================================================================
int32_t SendQueueDb::updateSendQueueInfo( SendQueInfo& sendQueInfo )
{
	removeSendQueueInfo( sendQueInfo.getGroupieId() );

	std::string onlineId;
	std::string assetId;
	sendQueInfo.getUserOnlineId().toHexString( onlineId );
	sendQueInfo.getHostOnlineId().toHexString( assetId );

	DbBindList bindList( onlineId.c_str() );
	bindList.add( assetId.c_str() );
	bindList.add( sendQueInfo.getModTime() ? sendQueInfo.getModTime() : GetGmtTimeMs() );
	bindList.add( (int)sendQueInfo.getSendQueState() );

	int32_t rc  = sqlExec( "INSERT INTO send_queue (onlineId,assetId,modTime,sendState) values(?,?,?,?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "SendQueueDb::addAsset error %d", rc );
	}

	return 0;
}

//============================================================================
void SendQueueDb::removeSendQueueInfo( GroupieId sendQueId )
{
	std::string onlineId;
	std::string assetId;
	sendQueId.getUserOnlineId().toHexString( onlineId );
	sendQueId.getHostOnlineId().toHexString( assetId );

	DbBindList bindList( onlineId.c_str() );
	bindList.add( assetId.c_str() );
	sqlExec( "DELETE FROM send_queue WHERE onlineId=? AND assetId=?", bindList );
}

//============================================================================
void SendQueueDb::getAllQueInfo( std::vector<SendQueInfo>& sendQueList )
{
	sendQueList.clear();
	std::string strId;
	VxGUID onlineId;
	VxGUID uniqueId;
	int64_t modTime;
	enum ESendQueState sendState;

	lockSendQueueDb();
	DbCursor * cursor = startQuery( "SELECT * FROM send_queue" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			strId = cursor->getString( SEND_COLUMN_ONLINE_ID );
			onlineId.fromVxGUIDHexString( strId.c_str() );
			strId = cursor->getString( SEND_COLUMN_ASSET_ID );
			uniqueId.fromVxGUIDHexString( strId.c_str() );
			modTime = cursor->getS64( SEND_COLUMN_MOD_TIME );
			sendState = (enum ESendQueState)cursor->getS32( SEND_COLUMN_SEND_STATE );

			SendQueInfo sendQueInfo( onlineId, uniqueId, sendState, modTime );
			
			sendQueList.emplace_back( sendQueInfo );
		}

		cursor->close();
	}

	unlockSendQueueDb();
}


