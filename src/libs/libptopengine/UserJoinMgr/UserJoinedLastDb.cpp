//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "UserJoinedLastDb.h"
#include "UserJoinInfo.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

namespace
{
    std::string 		TABLE_USER_HOST = "tblUserJoinedLast";

    std::string 		CREATE_COLUMNS_JOINED_LAST = " (onlineId TEXT, hostType INTEGER, lastJoinMs BIGINT, hostUrl TEXT ) ";

    const int			COLUMN_ONLINE_ID        = 0;
    const int			COLUMN_HOST_TYPE        = 1;
    const int			COLUMN_LAST_JOIN_MS     = 2;
    const int			COLUMN_HOST_URL         = 3;

    std::string 		TABLE_LAST_HOST_TYPE = "tbLastHostType";
    std::string 		CREATE_COLUMNS_LAST_HOST_TYPE = " (hostType INTEGER) ";
    const int			COLUMN_LAST_HOST_TYPE = 0;
};

//============================================================================
UserJoinedLastDb::UserJoinedLastDb( P2PEngine& engine, UserJoinMgr& hostListMgr, const char*dbName )
    : DbBase( dbName )
    , m_Engine( engine )
    , m_UserJoinMgr( hostListMgr )
{
}

//============================================================================
//! create tables in database 
int32_t UserJoinedLastDb::onCreateTables( int iDbVersion )
{
    lockDb();
    std::string strCmd = "CREATE TABLE " + TABLE_USER_HOST + CREATE_COLUMNS_JOINED_LAST;
    int32_t rc = sqlExec(strCmd);
    std::string strCmd2 = "CREATE TABLE " + TABLE_LAST_HOST_TYPE + CREATE_COLUMNS_LAST_HOST_TYPE;
    rc |= sqlExec( strCmd2 );
    unlockDb();
    return rc;
}

//============================================================================
// delete tables in database
int32_t UserJoinedLastDb::onDeleteTables( int iOldVersion ) 
{
    lockDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_USER_HOST;
    int32_t rc = sqlExec(strCmd);
    std::string strCmd2 = "DROP TABLE IF EXISTS " + TABLE_LAST_HOST_TYPE;
    rc |= sqlExec( strCmd2 );
    unlockDb();
    return rc;
}

//============================================================================
bool UserJoinedLastDb::setJoinedLastHostType( EHostType hostType )
{
    lockDb();
    std::string strCmd = "DELETE FROM " + TABLE_LAST_HOST_TYPE;
    int32_t rc = sqlExec( strCmd );

    DbBindList bindList( ( int )hostType );
    rc |= sqlExec( "INSERT INTO tbLastHostType (hostType) values(?)",
        bindList );
    vx_assert( 0 == rc );
    if( rc )
    {
        LogMsg( LOG_ERROR, "UserJoinedLastDb::setJoinedLastHostType error %d", rc );
    }

    unlockDb();
    return 0 == rc;
}

//============================================================================
bool UserJoinedLastDb::getJoinedLastHostType( EHostType& hostType )
{
    bool result{ false };

    lockDb();
    char queryStatement[512];
    sprintf( queryStatement, "SELECT * FROM tbLastHostType" );

    DbCursor* cursor = startQuery( queryStatement );
    if( cursor )
    {
        if( cursor->getNextRow() )
        {
            hostType = ( EHostType )cursor->getS32( COLUMN_LAST_HOST_TYPE );
            if( hostType != eHostTypeUnknown )
            {
                result = true;
            }
        }

        cursor->close();
    }

    unlockDb();
    return result;
}

//============================================================================
void UserJoinedLastDb::purgeAllJoinedLast( void )
{
    lockDb();
    std::string strCmd = "DELETE FROM " + TABLE_USER_HOST;
    int32_t rc = sqlExec( strCmd );
    unlockDb();
    if( rc )
    {
        LogMsg( LOG_ERROR, "UserJoinedLastDb::purgeAllJoinedLast error %d", rc );
    }
    else
    {
        LogMsg( LOG_INFO, "UserJoinedLastDb::purgeAllJoinedLast success" );
    }
}

//============================================================================
void UserJoinedLastDb::removeJoinedLast( EHostType hostType )
{
    DbBindList bindList( ( int )hostType );
    sqlExec( "DELETE FROM tblUserJoinedLast WHERE hostType=?", bindList );
}

//============================================================================
bool UserJoinedLastDb::setJoinedLast( EHostType hostType, VxGUID& onlineId, int64_t lastJoinMs, std::string hostUrl )
{
    if( onlineId == m_Engine.getMyOnlineId() )
    {
        // do not add ourself to database. If we joined then we are the admin
        // and we may join another host at the same time
        return true;
    }

    lockDb();
    removeJoinedLast( hostType );

    std::string onlineIdStr = onlineId.toHexString();

    DbBindList bindList( onlineIdStr.c_str() );
    bindList.add( (int)hostType );
    bindList.add( lastJoinMs ); 
    bindList.add( hostUrl.c_str() );

    int32_t rc = sqlExec( "INSERT INTO tblUserJoinedLast (onlineId, hostType, lastJoinMs, hostUrl ) values(?,?,?,?)",
        bindList );
    vx_assert( 0 == rc );
    if( rc )
    {
        LogMsg( LOG_ERROR, "UserJoinedLastDb::addUserJoin error %d", rc );
    }

    unlockDb();
    return ( 0 == rc );
}

//============================================================================
bool UserJoinedLastDb::getJoinedLast( EHostType hostType, VxGUID& onlineId, int64_t& lastJoinMs, std::string& hostUrl )
{
    bool result{ false };

    lockDb();
    char queryStatement[512];
    sprintf( queryStatement, "SELECT * FROM tblUserJoinedLast WHERE hostType=%d", hostType );

    DbCursor * cursor = startQuery( queryStatement );
    if( cursor )
    {
        if( cursor->getNextRow() )
        {
            onlineId.fromVxGUIDHexString( cursor->getString( COLUMN_ONLINE_ID ) );
            EHostType dbHostType = (EHostType)cursor->getS32( COLUMN_HOST_TYPE );
            if( hostType != dbHostType )
            {
                LogMsg( LOG_VERBOSE, "UserJoinedLastDb::getJoinedLast not the right host type %s", DescribeHostType( dbHostType ) );
            }
            else
            {
                lastJoinMs = cursor->getS64( COLUMN_LAST_JOIN_MS );
                hostUrl = cursor->getString( COLUMN_HOST_URL );
                result = onlineId.isValid() && lastJoinMs && !hostUrl.empty();
                if( !result )
                {
                    LogMsg( LOG_ERROR, "UserJoinedLastDb::getJoinedLast invalid data" );
                }
            }
        }

        cursor->close();
    }

    unlockDb();
    return result;
} 
