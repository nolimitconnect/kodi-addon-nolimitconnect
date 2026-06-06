//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IgnoredHostsDb.h"
#include "IgnoredHostInfo.h"
#include "IgnoreListMgr.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

namespace
{
    std::string 		TABLE_IGNORE_HOST	 			= "tblIgnoreHost";

    std::string 		CREATE_COLUMNS_HOST_IGNORE		= " (onlineId TEXT, thumbId TEXT, hostUrl TEXT, hostTitle TEXT, hostDescription TEXT, timestampdMs BIGINT) ";

    const int			COLUMN_ONLINE_ID			    = 0;
    const int			COLUMN_HOST_THUMB_ID			= 1;
    const int			COLUMN_HOST_URL                 = 2;
    const int			COLUMN_HOST_TITLE               = 3;
    const int			COLUMN_HOST_DESC                = 4;
    const int			COLUMN_TIMESTAMP_MS				= 5;
}

//============================================================================
IgnoredHostsDb::IgnoredHostsDb( P2PEngine& engine, IgnoreListMgr& hostListMgr, const char*dbName  )
    : DbBase( dbName )
    , m_Engine( engine )
    , m_IgnoreListMgr( hostListMgr )
{
}

//============================================================================
//! create tables in database 
int32_t IgnoredHostsDb::onCreateTables( int iDbVersion )
{
    lockDb();
    std::string strCmd = "CREATE TABLE " + TABLE_IGNORE_HOST + CREATE_COLUMNS_HOST_IGNORE;
    int32_t rc = sqlExec(strCmd);
    unlockDb();
    return rc;
}

//============================================================================
// delete tables in database
int32_t IgnoredHostsDb::onDeleteTables( int iOldVersion ) 
{
    lockDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_IGNORE_HOST;
    int32_t rc = sqlExec(strCmd);
    unlockDb();
    return rc;
}

//============================================================================
bool IgnoredHostsDb::removeFromDatabase( VxGUID& onlineId )
{
    std::string onlineIdStr = onlineId.toHexString();
    DbBindList bindList( onlineIdStr.c_str() );
    int32_t rc = sqlExec( "DELETE FROM tblIgnoreHost WHERE onlineId=?", bindList );
    return 0 == rc;
}

//============================================================================
bool IgnoredHostsDb::saveToDatabase( IgnoredHostInfo& hostInfo )
{
    removeFromDatabase( hostInfo.getOnlineId() );

    std::string onlineIdStr = hostInfo.getOnlineId().toHexString();
    std::string thumbIdStr = hostInfo.getThumbId().toHexString();

    DbBindList bindList( onlineIdStr.c_str() );
    bindList.add( thumbIdStr.c_str() );
    bindList.add( hostInfo.getHostUrl().c_str() );

    bindList.add( hostInfo.getHostTitle().c_str() );
    bindList.add( hostInfo.getHostDescription().c_str() );
    bindList.add( hostInfo.getTimestampMs() );  
   
    int32_t rc = sqlExec( "INSERT INTO tblIgnoreHost (onlineId, thumbId, hostUrl, hostTitle, hostDescription, timestampdMs) values(?,?,?,?,?,?)",
        bindList );
    vx_assert( 0 == rc );
    if( rc )
    {
        LogMsg( LOG_ERROR, "IgnoredHostsDb::saveToDatabase error %d", rc );
    }

    return ( 0 == rc );
}

//============================================================================
bool IgnoredHostsDb::restoreFromDatabase( std::map<VxGUID, IgnoredHostInfo>& ignoredHostList )
{
    lockDb();
    DbCursor * cursor = startQuery( "SELECT * FROM tblIgnoreHost" ); 
    if( NULL != cursor )
    {
        while( cursor->getNextRow() )
        {
            VxGUID onlineId;
            onlineId.fromVxGUIDHexString( cursor->getString( COLUMN_ONLINE_ID ) );
            VxGUID thumbId;
            thumbId.fromVxGUIDHexString( cursor->getString( COLUMN_HOST_THUMB_ID ) );

            IgnoredHostInfo ignoredHostInfo( onlineId, thumbId,
                cursor->getString( COLUMN_HOST_URL ),
                cursor->getString( COLUMN_HOST_TITLE ),
                cursor->getString( COLUMN_HOST_DESC ),
                cursor->getS64( COLUMN_TIMESTAMP_MS ) );

            if( onlineId.isValid() )
            {
                ignoredHostList[ onlineId ] = ignoredHostInfo;
            }
            else
            {
                LogMsg( LOG_ERROR, "IgnoredHostsDb::getAllUserJoins invalid id or host url" );
            }         
        }

        cursor->close();
    }

    unlockDb();
    return ignoredHostList.size();
} 
