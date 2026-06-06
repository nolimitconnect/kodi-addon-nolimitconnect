//==========================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "BigListDb.h"
#include "BigListInfo.h"
#include "BigListMgr.h"
#include <P2PEngine/P2PEngine.h>
#include <Network/NetworkMgr.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/sqlite3.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxMacros.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

namespace
{
	const int COL_BIGLIST_ONLINE_ID		= 1;
	const int COL_BIGLIST_NETWORK_KEY	= 2;
	const int COL_BIGLIST_CONNECT_TIME	= 3;
	const int COL_BIGLIST_BLOB			= 4;

} // annonymous namespace

//============================================================================
//! thread function to load all nodes in big list
static void * BigListLoadThreadFunction( void * pvParam )
{
    int32_t rc = 0;
    VxThread* poThread = (VxThread*)pvParam;
    poThread->setIsThreadRunning( true );

    BigListMgr* poMgr = (BigListMgr *)poThread->getThreadUserParam();
    if( poMgr )
    {
		poMgr->threadedRestoreAll();
    }

    poThread->threadAboutToExit();
    return nullptr;
}


//============================================================================
BigListDb::BigListDb( P2PEngine& engine, BigListMgr& bigListMgr )
: DbBase( "BigListDb" )
, BigList()
, m_Engine( engine )
, m_BigListMgr( bigListMgr )
{
}

//============================================================================
void BigListDb::threadedRestoreAll( void )
{
	// wait for network key to be set
    for( int i = 0; i < 200; i++ )
	{
		if( VxIsAppShuttingDown() )
		{
			return ;
		}

		if( !getNetworkKey().empty() )
		{
			break;
		}

		VxSleep( 1000 );
	}

	if( getNetworkKey().empty() )
	{
		LogMsg( LOG_ERROR, "BigListLoadThreadFunction: Restore No Network Key" );
		vx_assert( false );

		return;
	}

    // load all lists urls from database
    int32_t rc = dbRestoreAll();
    if( rc )
    {
        LogMsg( LOG_ERROR, "BigListLoadThreadFunction: Restore Error %d", rc );
    }

    m_Engine.onBigListLoadComplete( rc );
}

//============================================================================
std::string BigListDb::getNetworkKey( void )
{
	return m_Engine.getNetworkMgr().getNetworkKey();
}

//============================================================================
//! create tables in the database
int32_t BigListDb::onCreateTables( int iDbVersion )
{
	int32_t rc = sqlExec( "CREATE TABLE BigList ( online_id TEXT PRIMARY KEY, NetworkName TEXT, ConnectTime BIGINT, Object BLOB)" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
//! create tables in the database
int32_t BigListDb::onDeleteTables( int oldVersion )
{
	int32_t rc = sqlExec( (char *)"DROP TABLE BigList" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
//! startup
int32_t BigListDb::bigListDbStartup(  const char* pDbFileName )
{
	if( m_BigListDbInitialized )
	{
		bigListDbShutdown();
	}

	int32_t rc = dbStartup(BIG_LIST_INFO_VERSION, pDbFileName );
#ifdef DEBUG_BIGLIST_DB
    if( 0 == rc )
	{
		LogMsg( LOG_INFO, "bigListDbStartup  before Restore All");
	}

	LogMsg( LOG_INFO, "bigListDbStartup  result %d", rc );
#endif // DEBUG_BIGLIST_DB
    m_BigListLoadThread.startThread( (VX_THREAD_FUNCTION_T)BigListLoadThreadFunction, &m_BigListMgr );

	m_BigListDbInitialized = true;
	return rc;
}

//============================================================================
//! shutdown
int32_t BigListDb::bigListDbShutdown( void )
{
	int32_t rc = 0;
	if( m_BigListDbInitialized )
	{
		m_BigListDbInitialized = false;
		m_BigListLoadThread.killThread();

		removeAllInfos();
		rc = dbShutdown();
	}

	return rc;
}

//============================================================================
//! restore all of given network to lists from database
int32_t BigListDb::dbRestoreAll( void )
{
	int iRestoredCount = 0;
	m_LastNetworkKey = getNetworkKey(); 
	std::string strNetworkName = m_LastNetworkKey;

	std::vector<VxGUID> toRemoveIds;

	lockDb();

	removeAllInfos();
	DbCursor * cursor = startQuery( "SELECT Object, ConnectTime FROM BigList WHERE NetworkName=?", strNetworkName.c_str() );
	if( cursor )
	{
		while( cursor->getNextRow() )
		{
			int blobLen = 0;
			void * pTempBlob = cursor->getBlob( 0, &blobLen );
			int64_t sessionTimeMs = cursor->getS64( 1 );
			if (pTempBlob)
			{
				BigListInfo * poInfo = new BigListInfo();
				VxGUID onlineId;
				if( 0 == restoreBigListInfoFromBlob( (uint8_t *)pTempBlob, blobLen, poInfo, sessionTimeMs, onlineId ) )
				{
					// clear temporary flags
					poInfo->m_u32BigListTempFlags = 0;
					poInfo->setIsInDatabase( true );
					poInfo->setLastSessionTimeMs( sessionTimeMs );

					bigInsertInfo( poInfo->m_DirectConnectId, poInfo );
					iRestoredCount++;
					m_Engine.onBigListInfoRestored( poInfo );
				}
				else
				{
					LogModule( eLogStartup, LOG_DEBUG, "restoreBigListInfoFromBlob: failed" );
					delete poInfo;
				}
			}
		}

		cursor->close();
		LogModule( eLogStartup, LOG_DEBUG, "restoreBigListInfoFromBlob: restored %d", iRestoredCount );
	}

	unlockDb();
	return 0;
}

//============================================================================
int32_t BigListDb::dbUpdateSessionTime( VxGUID& onlineId, int64_t lastSessionTime )
{
	if( lastSessionTime )
	{
		lockDb();
		std::string strHexOnlineId = onlineId.toHexString();
		DbBindList bindList( lastSessionTime );
		bindList.add( strHexOnlineId.c_str() );
		int32_t rc = sqlExec( "UPDATE BigList SET ConnectTime=? WHERE online_id=?", bindList );
		unlockDb();
        return rc;
	}
	else
	{
		LogMsg( LOG_ERROR, "BigListDb::%s bad param 0 lastSessionTime", __func__ );
		vx_assert( false );
		return false;
	}
}

//============================================================================
//! if not in db insert BigListInfo else update database
int32_t BigListDb::dbUpdateBigListInfo( BigListInfo * poInfo, const char* networkName )
{
	if( !networkName || 0 == strlen( networkName ) )
	{
		LogMsg( LOG_ERROR, "BigListDb::%s bad param", __func__ );
		return -1;
	}

	int32_t rc = 0;
	if( poInfo->isInDatabase() )
	{
		rc = dbUpdateBigListInfoInDb( poInfo, networkName );
	}
	else 
	{
		rc = dbInsertBigListInfoIntoDb( poInfo, networkName );
	}

	return rc;
}

//============================================================================
//! remove friend by id
int32_t BigListDb::dbRemoveBigListInfo( VxGUID& onlineId )
{
	std::string strHexOnlineId = onlineId.toHexString();

	char SQL_Statement[1024];
	// make statement
	sprintf(SQL_Statement, "DELETE FROM BigList WHERE online_id='%s'", strHexOnlineId.c_str() );

	int retval;
	sqlite3_stmt* pStatement{ nullptr };

	lockDb();
	int32_t rc = dbOpen();
	if( rc )
	{
		vx_assert( false );
		dbClose();
		unlockDb();
		return rc;
	}

	retval = sqlite3_prepare( m_Db, SQL_Statement, (int)strlen(SQL_Statement), &pStatement, nullptr );
	if (!(SQLITE_OK == retval))
	{
		LogMsg( LOG_ERROR, "BigListDb::removeFriendFromDb:sqlite3_prepare:%s", sqlite3_errmsg(m_Db) );
		dbClose();
		unlockDb();
		vx_assert( false );
		return -1;
	}

	retval = sqlite3_step(pStatement);
	if (SQLITE_ERROR == retval)
	{
		LogMsg( LOG_ERROR, "BigListDb::removeFriendFromDb:sqlite3_step:%s", sqlite3_errmsg(m_Db) );
		sqlite3_finalize(pStatement);
		dbClose();
		unlockDb();
		vx_assert( false );
		return -1;
	}

	sqlite3_finalize(pStatement);
	dbClose();
	unlockDb();
	return 0;
}

//============================================================================
//! insert big list info node into database
int32_t BigListDb::dbInsertBigListInfoIntoDb( BigListInfo * poInfo, const char* networkName )
{
	// there is a possibility the isInDatabase flag did not get set so remove first
	dbRemoveBigListInfo( poInfo->getMyOnlineId() );

	int				    retval;
	sqlite3_stmt*		pStatement{ nullptr };
	uint8_t*			pu8Blob{ 0 };
	int				    iBlobLen{ 0 };
	int64_t				s64LastContactMs = poInfo->getLastSessionTimeMs();

	// make big list info into blob
	int32_t rc = saveBigListInfoIntoBlob( poInfo, &pu8Blob, &iBlobLen );
	if( rc )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:Make Blob Error:%d", rc );
		vx_assert( false );
		poInfo->setIsInDatabase( false );
		return -1;
	}

	vx_assert( pu8Blob );
	vx_assert( iBlobLen );
	std::string strOnlineIdHex = poInfo->getMyOnlineId().toHexString();
    const char* SQL_Statement = "INSERT INTO BigList (online_id,NetworkName,ConnectTime,Object) VALUES (?,?,?,?)";

	lockDb();
	rc = dbOpen();
	if( rc )
	{
		unlockDb();
		vx_assert( false );
		poInfo->setIsInDatabase( false );
		return rc;
	}

	// make statement
    retval = sqlite3_prepare( m_Db, SQL_Statement, -1, &pStatement, NULL);
    if( !(SQLITE_OK == retval) )
    {
        LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_prepare:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
    }

	if( SQLITE_OK != sqlite3_bind_text( pStatement, COL_BIGLIST_ONLINE_ID, strOnlineIdHex.c_str(), -1, SQLITE_TRANSIENT ) )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_OK != sqlite3_bind_text( pStatement, COL_BIGLIST_NETWORK_KEY, networkName, -1, SQLITE_TRANSIENT ) )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_OK != sqlite3_bind_int64( pStatement, COL_BIGLIST_CONNECT_TIME, s64LastContactMs ) ) 
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_OK != sqlite3_bind_blob( pStatement, COL_BIGLIST_BLOB, pu8Blob, iBlobLen, SQLITE_TRANSIENT ) )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_DONE != sqlite3_step( pStatement ) )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbInsertBigListInfoIntoDb:sqlite3_step:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	sqlite3_finalize( pStatement );
	dbClose();
	poInfo->setIsInDatabase( true );
	unlockDb();
	delete pu8Blob;
	return 0;

error_exit:
	dbClose();
	poInfo->setIsInDatabase( false );
	unlockDb();
	delete pu8Blob;
	return -1;
}

//============================================================================
//! update big list info node in database
int32_t BigListDb::dbUpdateBigListInfoInDb( BigListInfo * poInfo, const char* networkName )
{
	int64_t s64LastContact = poInfo->getLastSessionTimeMs();
	uint8_t * pu8Blob = 0;
	int iBlobLen = 0;

	// make big list info into blob
	int32_t rc = saveBigListInfoIntoBlob( poInfo, &pu8Blob, &iBlobLen );
	if( rc )
	{
		LogMsg( LOG_ERROR,"BigListDb::InsertBlob:Make Blob Error:%d", rc );
		vx_assert( false );
		return -1;
	}
	
	vx_assert( pu8Blob );
	vx_assert( iBlobLen );

	char SQL_Statement[4096];
	int retval;
	sqlite3_stmt* pStatement{ nullptr };

	std::string onlineIdHex = poInfo->getMyOnlineId().toHexString();
	sprintf(SQL_Statement, "UPDATE BigList SET ConnectTime=?,Object=? WHERE online_id='%s' AND NetworkName='%s'", 
			onlineIdHex.c_str(), 
			networkName );

	lockDb();
	rc = dbOpen();
	if( rc )
	{
		unlockDb();
		vx_assert( false );
		return rc;
	}


	// make statement
	retval = sqlite3_prepare( m_Db, SQL_Statement,(int)strlen(SQL_Statement),&pStatement, NULL);
	if (!(SQLITE_OK == retval))
	{
		LogMsg( LOG_ERROR,"BigListDb::dbUpdateBigListInfoInDb:sqlite3_prepare:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_OK != sqlite3_bind_int64( pStatement, 1, s64LastContact ) ) 
	{
		LogMsg( LOG_ERROR,"BigListDb::dbUpdateBigListInfoInDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if( SQLITE_OK != sqlite3_bind_blob( pStatement, 2, pu8Blob, iBlobLen, SQLITE_TRANSIENT) )
	{
		LogMsg( LOG_ERROR,"BigListDb::dbUpdateBigListInfoInDb:sqlite3_bind:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}

	if (SQLITE_DONE != sqlite3_step( pStatement ))
	{
		LogMsg( LOG_ERROR,"BigListDb::dbUpdateBigListInfoInDb:sqlite3_step:%s", sqlite3_errmsg( m_Db ) );
		goto error_exit;
	}
	
	sqlite3_finalize( pStatement );

	poInfo->setIsInDatabase( true );
	dbClose();
	unlockDb();
	delete pu8Blob;
	return 0;

error_exit:
	poInfo->setIsInDatabase( false );
	dbClose();
	unlockDb();
	delete pu8Blob;
	return -1;
}
//============================================================================
//! restore big list info from blob
int32_t BigListDb::restoreBigListInfoFromBlob( uint8_t * pu8Temp, int iDataLen, BigListInfo * poInfo, uint64_t lastSessionTime, VxGUID& onlineId )
{
	VxPktHdr* pktHdr{ nullptr };
	int32_t rc{ 0 };
	uint16_t u16PktLen{ 0 };

	// NOTE: don't bother with encryption. maybe later will use user login key for encryption and separate Info databases
	//// encrypt the blob
	////NOTE: the data length must be a multiple of the key length
	//rc = VxSymDecrypt(	&g_oGlobals.m_DefaultAdminKey, // Sym key
	//					(char *)pu8Temp,		// buffer to encrypt
	//					iDataLen,		// data length ( must be multiple of key length )
	//					NULL );			// if null then encrypted data put in pInBuf

    if( iDataLen < (int)sizeof( BigListInfoBase ) )
	{
		LogMsg( LOG_ERROR, "restoreBigListInfoFromBlob: invalid BigListInfoBase length" );
		rc = -1;
	}
	else
	{
		// read all we can with memcpy
		memcpy( ((BigListInfoBase *) poInfo), pu8Temp, sizeof( BigListInfoBase ));
		pu8Temp += sizeof( BigListInfoBase );
        iDataLen -= (int)sizeof( BigListInfoBase );
		onlineId = poInfo->getMyOnlineId();

		// restore incoming packet que
		// next 4 bytes is number of input que packets
		uint32_t inQueCnt = *((uint32_t *)pu8Temp );
		if( (inQueCnt > 1000) || ((inQueCnt * 16) > iDataLen ) )
		{
			LogMsg( LOG_ERROR, "restoreBigListInfoFromBlob: invalid incoming packet que length" );
			rc = -2;
		}
		else
		{
			poInfo->m_aoInQue.resize( inQueCnt );
			pu8Temp += 4;
			// restore input que
			for( uint32_t i = 0; i < inQueCnt; i++ )
			{
				u16PktLen = *((uint16_t *)pu8Temp);
				pktHdr = (VxPktHdr*)new char[ u16PktLen ];
				memcpy( pktHdr, pu8Temp, u16PktLen );
				poInfo->m_aoInQue[i] =  pktHdr ;
				pu8Temp += u16PktLen;
				iDataLen -= iDataLen;
				if( iDataLen < 0 )
				{
					LogMsg( LOG_ERROR, "restoreBigListInfoFromBlob: invalid incoming packet length" );
					rc = -3;
					break;
				}
			}

			if( 0 == rc )
			{
				// restore outgoing packet que
				// next 4 bytes is number of output que packets
				uint32_t outQueCnt = *((uint32_t *)pu8Temp );
				if( (outQueCnt > 1000) || ((outQueCnt * 16) > iDataLen ) )
				{
					LogMsg( LOG_ERROR, "restoreBigListInfoFromBlob: invalid outgoing packet que length" );
					rc = -4;
				}
				else
				{
					poInfo->m_aoOutQue.resize( outQueCnt );
					pu8Temp += 4;
					// restore Output que
					for( uint32_t i = 0; i < outQueCnt; i++ )
					{
						u16PktLen = *((uint16_t *)pu8Temp);
						pktHdr = (VxPktHdr*)new char[ u16PktLen ];
						memcpy( pktHdr, pu8Temp, u16PktLen );
						poInfo->m_aoOutQue[i] =  pktHdr ;
						pu8Temp += u16PktLen;
						iDataLen -= iDataLen;
						if( iDataLen < 0 )
						{
							LogMsg( LOG_ERROR, "restoreBigListInfoFromBlob: invalid outgoing packet length" );
							rc = -5;
							break;
						}
					}
				}
			}
		}
	}

	return rc;
}

//============================================================================
//! make big list info into blob
int32_t BigListDb::saveBigListInfoIntoBlob( BigListInfo * poInfo, uint8_t * * ppu8RetBlob, int * piRetBlobLen )
{
	VxPktHdr* pktHdr{ nullptr };

	int iLen = poInfo->CalcStoredLen();

	// NOTE: header size was already added to total in CalcStoredLen

	// round total length to 16 byte boundary for crypto
	uint32_t u32BlobLen = ROUND_TO_16BYTE_BOUNDRY( iLen );
	uint8_t* pu8Data = new unsigned char[ u32BlobLen ];
	uint8_t* pu8Temp = (unsigned char *)pu8Data;

	memcpy( pu8Temp, poInfo, sizeof( BigListInfoBase ) );
	pu8Temp += sizeof( BigListInfoBase );

	// store incoming packet que
	uint32_t inQueCnt = (uint32_t)poInfo->m_aoInQue.size();
	// next 4 bytes is number of input que packets
	*((uint32_t *)pu8Temp ) = inQueCnt;
	pu8Temp += 4;
	// store input que
	for( uint32_t i = 0; i < inQueCnt; i++ )
	{
		pktHdr = (VxPktHdr*)poInfo->m_aoInQue[ i ];
		memcpy( pu8Temp, pktHdr, pktHdr->getPktLength() );
		pu8Temp += pktHdr->getPktLength();
	}

	// store outgoing packet que
	uint32_t outQueCnt = (uint32_t)poInfo->m_aoOutQue.size();
	// next 4 bytes is number of output que packets
	*((uint32_t *)pu8Temp ) = outQueCnt;
	pu8Temp += 4;
	// store output que
	for( uint32_t i = 0; i < outQueCnt; i++ )
	{
		pktHdr = (VxPktHdr*)poInfo->m_aoOutQue[ i ];
		memcpy( pu8Temp, pktHdr, pktHdr->getPktLength() );
		pu8Temp += pktHdr->getPktLength();
	}
	
	uint32_t u32RealLen = (uint32_t)ROUND_TO_16BYTE_BOUNDRY( (pu8Temp - pu8Data ) );
	vx_assert(  u32RealLen <= u32BlobLen );
	* ppu8RetBlob = pu8Data;
	* piRetBlobLen = u32BlobLen;
	return 0;
}

//============================================================================
//! lock db and remove use
int32_t BigListDb::removeUserFromDatabase( VxGUID& onlineId )
{
	return dbRemoveBigListInfo( onlineId );
}
