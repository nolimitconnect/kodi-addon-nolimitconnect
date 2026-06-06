//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostUrlListDb.h"
#include "HostUrlInfo.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/sqlite3.h>

#include <memory.h>

namespace
{
	const int			COLUMN_IDX_ONLINE_ID			= 0;
	const int			COLUMN_IDX_HOST_TYPE			= 1;
	const int			COLUMN_IDX_HOST_URL				= 2;
	const int			COLUMN_IDX_TIMESTAMP			= 3;
}

//============================================================================
HostUrlListDb::HostUrlListDb()
: DbBase( "HostUrlListDb" )
{
}

//============================================================================
int32_t HostUrlListDb::hostUrlListDbStartup( int dbVersion, const char* dbFileName )
{
	dbShutdown();
	return dbStartup( dbVersion, dbFileName );
}

//============================================================================
int32_t HostUrlListDb::hostUrlListDbShutdown( void )
{
	return dbShutdown();
}

//============================================================================
int32_t HostUrlListDb::onCreateTables( int iDbVersion )
{
	int32_t rc = sqlExec( "CREATE TABLE tblHostUrl (online_id TEXT, host_type INTEGER, hostUrl TEXT, timestamp BIGINT)" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
int32_t HostUrlListDb::onDeleteTables( int iOldVersion )
{
	int32_t rc = sqlExec( (char *)"DROP TABLE tblHostUrl" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
void HostUrlListDb::getAllHostUrls( std::vector<HostUrlInfo>& hostUrlList )
{
	lockDb();
	DbCursor* cursor = startQuery( "SELECT * FROM tblHostUrl" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			HostUrlInfo hostInfo;

			hostInfo.getOnlineId().fromVxGUIDHexString( cursor->getString( COLUMN_IDX_ONLINE_ID ) );
			hostInfo.setHostType( (EHostType)cursor->getS32( COLUMN_IDX_HOST_TYPE ) );
			hostInfo.setHostUrl( cursor->getString( COLUMN_IDX_HOST_URL ) );
			hostInfo.setTimestamp( ( int64_t )cursor->getS64( COLUMN_IDX_TIMESTAMP ) );			

			hostUrlList.push_back( hostInfo );
		}

		cursor->close();
	}

	unlockDb();
}

//============================================================================
bool HostUrlListDb::saveHostUrl( HostUrlInfo& hostUrlInfo )
{
	int32_t rc = 0;
	std::string onlineId;
	if( !hostUrlInfo.getOnlineId().toHexString( onlineId ) )
	{
		LogMsg( LOG_ERROR, "ERROR: HostUrlListDb::saveHostUrl INVALID ONLINE ID" );
	}

	m_DbMutex.lock(); // make thread safe

	bool bExists = false;
    DbCursor * cursor = startQuery(  "SELECT * FROM tblHostUrl WHERE online_id=? AND host_type=?", onlineId.c_str(), (int)hostUrlInfo.getHostType() );
	if( cursor )
    {
		if( cursor->getNextRow() )
		{
			bExists = true;
		}

		cursor->close();
	}

	DbBindList bindList( hostUrlInfo.getHostUrl().c_str() );
	bindList.add( hostUrlInfo.getTimestamp() );
	bindList.add( onlineId.c_str() );
	bindList.add( (int)hostUrlInfo.getHostType() );

	if( bExists )
	{
		rc = sqlExec( "UPDATE tblHostUrl SET hostUrl=?,timestamp=? WHERE online_id=? AND host_type=?", bindList );
	}
	else
	{
		// insert new record
		rc = sqlExec( "INSERT INTO tblHostUrl (hostUrl, timestamp, online_id, host_type) VALUES(?,?,?,?)", bindList );
	}

	if( rc )
	{
		LogMsg( LOG_ERROR, "HostUrlListDb::saveHostUrl: ERROR %d updating host url", rc );
	}

	m_DbMutex.unlock();
	return 0 == rc;
}

//============================================================================
void HostUrlListDb::removeClosedPortIdent( VxGUID& onlineId )
{
	m_DbMutex.lock();
	std::string onlineIdHex = onlineId.toHexString();
	DbBindList bindList( onlineIdHex.c_str() );
	sqlExec( "DELETE FROM tblHostUrl WHERE online_id=?", bindList );
	m_DbMutex.unlock();
}
