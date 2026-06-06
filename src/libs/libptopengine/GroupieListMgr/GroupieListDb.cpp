//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "GroupieListDb.h"
#include "GroupieInfo.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/sqlite3.h>

#include <memory.h>

namespace
{
	const int			COLUMN_IDX_GROUPIE_ONLINE_ID	= 0;
	const int			COLUMN_IDX_HOST_ONLINE_ID		= 1;
	const int			COLUMN_IDX_HOST_TYPE			= 2;
	const int			COLUMN_IDX_HOST_URL				= 3;
	const int			COLUMN_IDX_HOST_TITLE			= 4;
	const int			COLUMN_IDX_HOST_DESC			= 5;
	const int			COLUMN_IDX_IS_FAVORITE			= 6;
	const int			COLUMN_IDX_CONNECT_TIME			= 7;
	const int			COLUMN_IDX_JOIN_TIME			= 8;
	const int			COLUMN_IDX_HOST_INFO_TIME		= 9;
}

//============================================================================
GroupieListDb::GroupieListDb()
: DbBase( "GroupieListDb" )
{
}

//============================================================================
GroupieListDb::~GroupieListDb()
{
}

//============================================================================
int32_t GroupieListDb::groupieListDbStartup( int dbVersion, const char* dbFileName )
{
	dbShutdown();
	return dbStartup( dbVersion, dbFileName );
}

//============================================================================
int32_t GroupieListDb::groupieListDbShutdown( void )
{
	return dbShutdown();
}

//============================================================================
int32_t GroupieListDb::onCreateTables( int iDbVersion )
{
	int32_t rc = sqlExec( "CREATE TABLE tblGroupie (groupieOnlineId TEXT, hostOnlineId TEXT, hostType INTEGER, groupieUrl TEXT, groupieTitle TEXT, groupieDesc TEXT, favorite INTEGER, connectTime BIGINT, joinTime BIGINT, infoTime BIGINT)" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
int32_t GroupieListDb::onDeleteTables( int iOldVersion )
{
	int32_t rc = sqlExec( (char *)"DROP TABLE tblGroupie" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
void GroupieListDb::getAllGroupies( std::vector<GroupieInfo>& groupieList )
{
	lockDb();
	DbCursor* cursor = startQuery( "SELECT * FROM tblGroupie" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			GroupieInfo hostInfo;

			hostInfo.getUserOnlineId().fromVxGUIDHexString( cursor->getString( COLUMN_IDX_GROUPIE_ONLINE_ID ) );
			hostInfo.getHostOnlineId().fromVxGUIDHexString( cursor->getString( COLUMN_IDX_HOST_ONLINE_ID ) );
			hostInfo.setHostType( (EHostType)cursor->getS32( COLUMN_IDX_HOST_TYPE ) );
			hostInfo.setGroupieUrl( cursor->getString( COLUMN_IDX_HOST_URL ) );
			hostInfo.setGroupieTitle( cursor->getString( COLUMN_IDX_HOST_TITLE ) );
			hostInfo.setGroupieDescription( cursor->getString( COLUMN_IDX_HOST_DESC ) );
			hostInfo.setIsFavorite( cursor->getS32( COLUMN_IDX_IS_FAVORITE ) );
			hostInfo.setConnectedTimestamp( ( int64_t )cursor->getS64( COLUMN_IDX_CONNECT_TIME ) );
			hostInfo.setJoinedTimestamp( ( int64_t )cursor->getS64( COLUMN_IDX_JOIN_TIME ) );
			hostInfo.setGroupieInfoTimestamp( ( int64_t )cursor->getS64( COLUMN_IDX_HOST_INFO_TIME ) );

			groupieList.push_back( hostInfo );
		}

		cursor->close();
	}

	unlockDb();
}

//============================================================================
void GroupieListDb::removeGroupieInfo( GroupieId& groupieId )
{
	removeGroupieInfo( groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType() );
}

//============================================================================
void GroupieListDb::removeGroupieInfo( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType )
{
	std::string groupieOnlineIdHex = groupieOnlineId.toHexString();
	std::string hostOnlineIdHex = hostOnlineId.toHexString();

	DbBindList bindList( groupieOnlineIdHex.c_str() );
	bindList.add( hostOnlineIdHex.c_str() );
	bindList.add( ( int )hostType );
	m_DbMutex.lock();

	sqlExec( "DELETE FROM tblGroupie WHERE groupieOnlineId=? AND hostOnlineId=? AND hostType=?", bindList );
	m_DbMutex.unlock();
}

//============================================================================
bool GroupieListDb::doesGroupieInfoExist( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, std::string& retGroupieOnlineHexStr, std::string& retHostOnlineHexStr )
{
	if( !groupieOnlineId.isValid() || !groupieOnlineId.toHexString( retGroupieOnlineHexStr ) )
	{
		LogMsg( LOG_ERROR, "ERROR: GroupieListDb::doesGroupieInfoExist INVALID GROUPIE ID" );
		return false;
	}

	if( !hostOnlineId.isValid() || !hostOnlineId.toHexString( retHostOnlineHexStr ) )
	{
		LogMsg( LOG_ERROR, "ERROR: GroupieListDb::doesGroupieInfoExist INVALID GROUPIE ID" );
		return false;
	}

	m_DbMutex.lock(); // make thread safe

	bool doesExists = false;
	DbCursor* cursor = startQuery( "SELECT * FROM tblGroupie WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", retGroupieOnlineHexStr.c_str(), retHostOnlineHexStr.c_str(), ( int )hostType );
	if( cursor )
	{
		if( cursor->getNextRow() )
		{
			doesExists = true;
		}

		cursor->close();
	}

	m_DbMutex.unlock();
	return doesExists;
}

//============================================================================
bool GroupieListDb::updateIsFavorite( GroupieId& groupieId, bool isFavorite )
{
	return updateIsFavorite( groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType(), isFavorite );
}

//============================================================================
bool GroupieListDb::updateIsFavorite( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, bool isFavorite )
{
	std::string groupieOnlineHexStr;
	std::string hostOnlineHexStr;
	bool result = doesGroupieInfoExist( groupieOnlineId, hostOnlineId, hostType, groupieOnlineHexStr, hostOnlineHexStr );
	if( result )
	{
		DbBindList bindList( (int)isFavorite );
		bindList.add( groupieOnlineHexStr.c_str() );
		bindList.add( hostOnlineHexStr.c_str() );
		bindList.add( ( int )hostType );

		m_DbMutex.lock();
		int32_t rc = sqlExec( "UPDATE tblGroupie SET favorite=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		m_DbMutex.unlock();
		result = 0 == rc;
	}

	return result;
}

//============================================================================
bool GroupieListDb::updateLastConnected( GroupieId& groupieId, int64_t lastConnectedTime )
{
	return updateLastConnected( groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType(), lastConnectedTime );
}

//============================================================================
bool GroupieListDb::updateLastConnected( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, int64_t lastConnectedTime )
{
	std::string groupieOnlineHexStr;
	std::string hostOnlineHexStr;
	bool result = doesGroupieInfoExist( groupieOnlineId, hostOnlineId, hostType, groupieOnlineHexStr, hostOnlineHexStr );
	if( result )
	{
		DbBindList bindList( lastConnectedTime );
		bindList.add( groupieOnlineHexStr.c_str() );
		bindList.add( hostOnlineHexStr.c_str() );
		bindList.add( ( int )hostType );

		m_DbMutex.lock();
		int32_t rc = sqlExec( "UPDATE tblGroupie SET connectTime=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		m_DbMutex.unlock();
		result = 0 == rc;
	}

	return result;
}

//============================================================================
bool GroupieListDb::updateLastJoined( GroupieId& groupieId, int64_t lastJoinedTime )
{
	return updateLastJoined( groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType(), lastJoinedTime );
}

//============================================================================
bool GroupieListDb::updateLastJoined( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, int64_t lastJoinedTime )
{
	std::string groupieOnlineHexStr;
	std::string hostOnlineHexStr;
	bool result = doesGroupieInfoExist( groupieOnlineId, hostOnlineId, hostType, groupieOnlineHexStr, hostOnlineHexStr );
	if( result )
	{
		DbBindList bindList( lastJoinedTime );
		bindList.add( groupieOnlineHexStr.c_str() );
		bindList.add( hostOnlineHexStr.c_str() );
		bindList.add( ( int )hostType );

		m_DbMutex.lock();
		int32_t rc = sqlExec( "UPDATE tblGroupie SET joinTime=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		m_DbMutex.unlock();
		result = 0 == rc;
	}

	return result;
}

//============================================================================
bool GroupieListDb::updateGroupieUrl( bool ipv6, GroupieId& groupieId, std::string& hostUrl )
{
	return updateGroupieUrl( ipv6, groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType(), hostUrl );
}

//============================================================================
bool GroupieListDb::updateGroupieUrl( bool ipv6, VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, std::string& hostUrl )
{
	std::string groupieOnlineHexStr;
	std::string hostOnlineHexStr;
	bool result = doesGroupieInfoExist( groupieOnlineId, hostOnlineId, hostType, groupieOnlineHexStr, hostOnlineHexStr );
	if( result )
	{
		DbBindList bindList( hostUrl.c_str() );
		bindList.add( groupieOnlineHexStr.c_str() );
		bindList.add( hostOnlineHexStr.c_str() );
		bindList.add( ( int )hostType );

		int32_t rc{ 0 };
		m_DbMutex.lock();
		if( ipv6 )
		{
			rc = sqlExec( "UPDATE tblGroupie SET groupieUrlIpv6=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		}
		else
		{
			rc = sqlExec( "UPDATE tblGroupie SET groupieUrlIpv4=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		}

		m_DbMutex.unlock();
		result = 0 == rc;
	}

	return result;
}

//============================================================================
bool GroupieListDb::updateGroupieTitleAndDescription( GroupieId& groupieId, std::string& title, std::string& description, int64_t lastDescUpdateTime )
{
	return updateGroupieTitleAndDescription( groupieId.getUserOnlineId(), groupieId.getHostOnlineId(), groupieId.getHostType(), title, description, lastDescUpdateTime );
}

//============================================================================
bool GroupieListDb::updateGroupieTitleAndDescription( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, std::string& title, std::string& description, int64_t lastDescUpdateTime )
{
	std::string groupieOnlineHexStr;
	std::string hostOnlineHexStr;
	bool result = doesGroupieInfoExist( groupieOnlineId, hostOnlineId, hostType, groupieOnlineHexStr, hostOnlineHexStr );
	if( result )
	{
		DbBindList bindList( title.c_str() );
		bindList.add( description.c_str() );
		bindList.add( lastDescUpdateTime );
		bindList.add( groupieOnlineHexStr.c_str() );
		bindList.add( hostOnlineHexStr.c_str() );
		bindList.add( ( int )hostType );

		m_DbMutex.lock();
		int32_t rc = sqlExec( "UPDATE tblGroupie SET groupieTitle=?, groupieDesc=?, infoTime=? WHERE groupieOnlineId=? hostOnlineId=? AND hostType=?", bindList );
		m_DbMutex.unlock();
		result = 0 == rc;
	}

	return result;
}


//============================================================================
bool GroupieListDb::saveGroupie( GroupieInfo& hostedInfo )
{
	std::string groupieOnlineId;
	if( !hostedInfo.getGroupieId().isValid() || !hostedInfo.getUserOnlineId().toHexString( groupieOnlineId ) )
	{
		LogMsg( LOG_ERROR, "ERROR: GroupieListDb::saveGroupie INVALID ONLINE ID" );
		return false;
	}

	std::string hostOnlineId;
	if( !hostedInfo.getHostOnlineId().toHexString( hostOnlineId ) )
	{
		LogMsg( LOG_ERROR, "ERROR: GroupieListDb::saveGroupie INVALID HOSTED ID" );
		return false;
	}

	removeGroupieInfo( hostedInfo.getGroupieId() );

	m_DbMutex.lock(); // make thread safe

	DbBindList bindList( groupieOnlineId.c_str() );
	bindList.add( hostOnlineId.c_str() );
	bindList.add( ( int )hostedInfo.getHostType() );
	bindList.add( hostedInfo.getGroupieUrl().c_str() );
	bindList.add( hostedInfo.getGroupieTitle().c_str() );
	bindList.add( hostedInfo.getGroupieDescription().c_str() );

	bindList.add( ( int )hostedInfo.getIsFavorite() );

	bindList.add( hostedInfo.getConnectedTimestamp() );
	bindList.add( hostedInfo.getJoinedTimestamp() );
	bindList.add( hostedInfo.getGroupieInfoTimestamp() );

	// insert new record
	int32_t rc = sqlExec( "INSERT INTO tblGroupie (groupieOnlineId, hostOnlineId, hostType, groupieUrl, groupieTitle, groupieDesc, favorite, connectTime, joinTime, infoTime) VALUES(?,?,?,?,?,?,?,?,?,?)", bindList );

	if( rc )
	{
		LogMsg( LOG_ERROR, "GroupieListDb::saveGroupie: ERROR %d updating host url", rc );
	}

	m_DbMutex.unlock();
	return 0 == rc;
}
