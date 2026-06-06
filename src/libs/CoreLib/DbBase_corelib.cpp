//============================================================================
// Copyright (C) 2003 Brett R. Jones
//
// Issued to public domain 2013
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "config_corelib.h"

#include "DbBase.h"
#include "VxFileUtil.h"
#include "VxDebug.h"

#include "sqlite3.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <array>

namespace
{
	enum ESqlExecStep
	{
		eSqlExecStepDbOpen				= 0,
		eSqlExecStepPrepStatement		= 1,
		eSqlExecStepBindParams			= 2,
		eSqlExecStepFinalize			= 3,
		eSqlExecStepError				= 4,
		eSqlExecStepDbClose				= 5,
		eMaxExecSqlStep					= 6
	};

	const char* DescribeSqlStep( enum ESqlExecStep sqlStep )
	{
		switch( sqlStep )
		{
		case eSqlExecStepDbOpen:
			return "sqlDbOpen";
		case eSqlExecStepDbClose:
			return "sqlDbClose";
		case eSqlExecStepPrepStatement:
			return "sqlPrepStmt";
		case eSqlExecStepBindParams:
			return "sqlBindParams";
		case eSqlExecStepFinalize:
			return "sqlFinalize";
		case eSqlExecStepError:
			return "sqlStepError";
		case eMaxExecSqlStep:
		default:
			return "unknown step";
		}
	}

	VxMutex			g_DbBaseStartupMutex;
}

//============================================================================
DbBindParam::DbBindParam( void )
: m_EDbBindType( eDbBindTypeNone )
, m_AsText( "" )
, m_AsInt( 0 )
, m_AsS64( 0 )
, m_AsBlob( 0 )
, m_BlobLen( 0 )
{
}

//============================================================================
DbBindParam::DbBindParam( const char* text )
: m_EDbBindType( eDbBindTypeText )
, m_AsInt( 0 )
, m_AsS64( 0 )
, m_AsBlob( 0 )
, m_BlobLen( 0 )
{
	m_AsText = ( 0 == text ) ? "" : text;
}

//============================================================================
DbBindParam::DbBindParam( int asInt )
: m_EDbBindType( eDbBindTypeInt )
, m_AsText( "" )
, m_AsInt( asInt )
, m_AsS64( 0 )
, m_AsBlob( 0 )
, m_BlobLen( 0 )
{
}

//============================================================================
DbBindParam::DbBindParam( int64_t asS64 )
: m_EDbBindType( eDbBindTypeS64 )
, m_AsText( "" )
, m_AsInt( 0 )
, m_AsS64( asS64 )
, m_AsBlob( 0 )
, m_BlobLen( 0 )
{
}

//============================================================================
DbBindParam::DbBindParam( void * blob, int len )
: m_EDbBindType( eDbBindTypeBlob )
, m_AsText( "" )
, m_AsInt( 0 )
, m_AsS64( 0 )
, m_AsBlob( blob )
, m_BlobLen( len )
{
}

//============================================================================
EDbBindType DbBindParam::getType( void )
{
	return( m_EDbBindType );
}

//============================================================================
const char* DbBindParam::getText( void )
{
	vx_assert( eDbBindTypeText == m_EDbBindType );
	return( m_AsText.c_str() );
}

//============================================================================
unsigned short DbBindParam::getTextLen( void )
{
	vx_assert( eDbBindTypeText == m_EDbBindType );
	return (unsigned short)( m_AsText.length() );
}

//============================================================================
int DbBindParam::getInt( void )
{
	vx_assert( eDbBindTypeInt == m_EDbBindType );
	return( m_AsInt );
}

//============================================================================
int64_t DbBindParam::getS64( void )
{
	vx_assert( eDbBindTypeS64 == m_EDbBindType );
	return( m_AsS64 );
}

//============================================================================
void * DbBindParam::getBlob( int& len )
{
	vx_assert( eDbBindTypeBlob == m_EDbBindType );
	len = m_BlobLen;
	return m_AsBlob;
}

//============================================================================
DbBindList::DbBindList( void )
{
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
DbBindList::DbBindList( const char* strVal )
{
	add( strVal );
}

//============================================================================
DbBindList::DbBindList( void * blob, int blobLen )
{
	add( blob, blobLen );
}

//============================================================================
DbBindList::DbBindList( int intVal )
{
	DbBindParam* pBindParam;

	pBindParam = new DbBindParam( intVal );
	vx_assert( ( DbBindParam* )0 != pBindParam );
	m_ParamList.emplace_back( pBindParam );
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
DbBindList::DbBindList( int64_t s64Val )
{
	DbBindParam* pBindParam;

	pBindParam = new DbBindParam( s64Val );
	vx_assert( ( DbBindParam* )0 != pBindParam );
	m_ParamList.emplace_back( pBindParam );
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
DbBindList::~DbBindList( void )
{
	DbBindParam* pBindParam;

	while( ! m_ParamList.empty() )
	{
		pBindParam = m_ParamList.back();
		vx_assert( ( DbBindParam* )0 != pBindParam );
		m_ParamList.pop_back();
		delete pBindParam;
	}

}

//============================================================================
void DbBindList::add( const char* strVal )
{
	DbBindParam* pBindParam;

	pBindParam = new DbBindParam( strVal );
	vx_assert( ( DbBindParam* )0 != pBindParam );
	m_ParamList.emplace_back( pBindParam );
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
void DbBindList::add( int64_t s64Val )
{
	DbBindParam* pBindParam;

	pBindParam = new DbBindParam( s64Val );
	vx_assert( ( DbBindParam* )0 != pBindParam );
	m_ParamList.emplace_back( pBindParam );
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
void DbBindList::add( void * blob, int blobLen )
{
	DbBindParam* pBindParam;

	pBindParam = new DbBindParam( blob, blobLen );
	vx_assert( ( DbBindParam* )0 != pBindParam );
	m_ParamList.emplace_back( pBindParam );
	m_CurrParamIter = m_ParamList.begin();
}

//============================================================================
bool DbBindList::getFirst( DbBindParam** ppBindParam )
{
	bool retVal = false;

	vx_assert( ( DbBindParam** )0 != ppBindParam );
	m_CurrParamIter = m_ParamList.begin();

	if( m_ParamList.end() != m_CurrParamIter )
	{
		*ppBindParam = *m_CurrParamIter;
		vx_assert( ( DbBindParam* )0 != *ppBindParam );
		retVal = true;
	}

	return( retVal );
}

//============================================================================
bool DbBindList::getNext( DbBindParam** ppBindParam )
{
	bool retVal = false;

	vx_assert( ( DbBindParam** )0 != ppBindParam );
	vx_assert( m_ParamList.end() != m_CurrParamIter );
	m_CurrParamIter++;

	if( m_ParamList.end() == m_CurrParamIter )
	{
		m_CurrParamIter = m_ParamList.begin();
	}
	else
	{
		*ppBindParam = *m_CurrParamIter;
		vx_assert( ( DbBindParam* )0 != *ppBindParam );
		retVal = true;
	}

	return( retVal );
}

//============================================================================
bool DbBindList::hasNext( void )
{
	auto nextIter = m_CurrParamIter;
	nextIter++;
	return nextIter != m_ParamList.end();
}

//============================================================================
DbBase::DbBase( std::string databaseName ) 
: m_strDatabaseName( databaseName )
, m_bDbInitialized(0)
, m_iDbVersion(0)
, m_Db(0)
{
}

//============================================================================
//! Initialize the database.. if doesn't exist then call DbCreateDatabase and DbCreateTables
int32_t DbBase::dbStartup( int iDbVersion, std::string pDbName )
{
	// in theory although sqlite is not thread safe you should be able to access multiple separate instances as same time by different threads.
	// Just to make sure we are only allowing one startup of database at a time
	g_DbBaseStartupMutex.lock();
	int32_t rc = 0;

	vx_assert( !pDbName.empty() );
	m_strDbFileName = pDbName;
	m_iDbVersion = iDbVersion;

	rc = doDatabaseStartup();

	g_DbBaseStartupMutex.unlock();
	return rc;
}

//============================================================================
int32_t DbBase::doDatabaseStartup( void )
{
    LogModule( eLogStartup, LOG_INFO, "DbBase::dbStartup %s",  m_strDbFileName.c_str() );

	int32_t rc = -1;

	// create paths and database if necessary
	if( !VxFileUtil::fileExists( m_strDbFileName.c_str(), false ) )
	{
		char tmpDir[ VX_MAX_PATH ];

		char* pTemp{ nullptr };

		strcpy( tmpDir, m_strDbFileName.c_str() );
		pTemp = strrchr( tmpDir, '/' );
		if( NULL != pTemp )
		{
			pTemp[0] = '\0';
			VxFileUtil::makeDirectory( tmpDir );
		}

        if( ! VxFileUtil::directoryExists( tmpDir ) )
        {
            LogMsg( LOG_DEBUG, "ERROR DbBase::dbStartup could not create directory for db %s", tmpDir);
        }

		rc = onCreateDatabase( m_iDbVersion );
		if( 0 != rc )
		{
			handleSqlError( 0, "DbBase:Cannot create database %s", m_strDbFileName.c_str() );
		}
	}
	else
	{
		// check the version .. upgrade if necessary
		int iOldDbVersion = readDatabaseVersion();
		if( iOldDbVersion != m_iDbVersion )
		{
			rc = onUpgradeDatabase( iOldDbVersion, m_iDbVersion );
		}
		else
		{
			rc = 0;
		}
	}

	m_bDbInitialized = true;
	return rc;
}

//============================================================================
int32_t DbBase::dbShutdown( void )
{
	return 0;
}

//============================================================================
//! create initial database
int32_t DbBase::onCreateDatabase( int iDbVersion )
{
	sqlite3 *db;
	int32_t rc = sqlite3_open( m_strDbFileName.c_str(), &db);
	if( SQLITE_OK != rc )
	{
		handleSqlError( LOG_ERROR, "DbCreateDatabase:ERROR %d Unable to create database %s", rc, m_strDbFileName.c_str() );
		sqlite3_close(db);
		return rc;
	}
	// close database.. Create tables will reopen
	sqlite3_close(db);
	rc = writeDatabaseVersion( iDbVersion );
	if( SQLITE_OK != rc )
	{
		handleSqlError( LOG_ERROR, "DbCreateDatabase:ERROR %d Unable to create version table %s", rc, m_strDbFileName.c_str() );
		sqlite3_close(db);
		return rc;
	}

	// make tables in database
	rc = onCreateTables( iDbVersion );
	if( rc )
	{
		handleSqlError( LOG_ERROR, "CreateDataBase:ERROR %d Unable to create Table in db %s", rc, m_strDbFileName.c_str() );
		VxFileUtil::deleteFile( m_strDbFileName.c_str() );
	}
	// create version table
	return rc;
}

//============================================================================
//! upgrade db from old version to new version
int32_t DbBase::onUpgradeDatabase(int iOldDbVersion, int iNewDbVersion)
{
	int32_t rc = onDeleteTables( iOldDbVersion );
	if( 0 == rc )
	{
		rc = onCreateTables( iNewDbVersion );
	}
	if( rc )
	{
		handleSqlError( LOG_ERROR, "onUpgradeDatabase:ERROR %d in db %s", rc, m_strDbFileName.c_str() );
	}
	else
	{
		//default behavior should write new version to database table otherwise will upgrade each time initialized
		writeDatabaseVersion( iNewDbVersion );
	}

	return rc;
}

//============================================================================
//! open the database
int32_t DbBase::dbOpen( void )
{
	if( 0 == m_strDbFileName.size() )
	{
		LogMsg( LOG_ERROR, "ERROR Attempted DbBase::dbOpen %s without a file name", m_strDatabaseName.c_str() );
		vx_assert( m_strDbFileName.size() );
		return -1;
	}

	if( m_Db )
	{
		LogMsg( LOG_ERROR, "ERROR Attempted DbBase::dbOpen file exists %s but database is already open", m_strDbFileName.c_str() );
		vx_assert( false ); // throw assert but do not stop
	}

	m_Db = nullptr;

	int retval = sqlite3_open( m_strDbFileName.c_str(), &m_Db );
	if (!(SQLITE_OK == retval))
	{
		handleSqlError( LOG_ERROR, "DbBase:Unable to open db %s errno %d SQLITE ERROR %d ", m_strDbFileName.c_str(), VxGetLastError(), retval );
		sqlite3_close(m_Db);
		m_Db = nullptr;
		if( SQLITE_CANTOPEN == retval )
		{
			uint64_t dbFileSize = VxFileUtil::fileExists( m_strDbFileName.c_str() );
			LogMsg( LOG_VERBOSE, "Retry of DbBase::dbOpen %s file size %s", m_strDbFileName.c_str(), VxFileUtil::describeFileSize( dbFileSize ).c_str() );

			VxSleep( 500 );
			retval = sqlite3_open( m_strDbFileName.c_str(), &m_Db );
			if (!(SQLITE_OK == retval))
			{
				handleSqlError( LOG_ERROR, "Failed rety DbBase:dbOpen db %s errno %d SQLITE ERROR %d ", m_strDbFileName.c_str(), VxGetLastError(), retval );
				sqlite3_close(m_Db);
				m_Db = nullptr;
				return -4;
			}
			else
			{
				LogMsg( LOG_VERBOSE, "Retry of DbBase::dbOpen %s SUCCESS", m_strDbFileName.c_str() );
				return 0;
			}
		}

		return -3;
	}

	vx_assert( m_Db );
	return 0;
}

//============================================================================
//! close the database
int32_t DbBase::dbClose( void )
{
	if( m_Db )
	{
		sqlite3_close(m_Db);
		m_Db = NULL;
		return 0;
	}

	handleSqlError( 0, "DbBase:Tried to close already closed db %s", m_strDbFileName.c_str() );
	return -1;
}

//============================================================================
//! read database version from version table 
int DbBase::readDatabaseVersion( void )
{
	std::string prepString = "SELECT * FROM DBBASE_VERSION";
	int iVersion{ 0 };

	m_DbMutex.lock();
	if( 0 == dbOpen() )
	{
		sqlite3_stmt * poSqlStatement = nullptr;
		int iResult = sqlite3_prepare_v2( m_Db, prepString.c_str(), (int)( prepString.length() + 1 ), &poSqlStatement, NULL );
		if( SQLITE_OK != iResult ) 
		{
            handleSqlError( 0, "DbBase::%s error %s db %s", __func__, sqlite3_errmsg(m_Db), m_strDbFileName.c_str() );
            dbClose();
            m_DbMutex.unlock();
            LogMsg( LOG_ERROR, "DbBase::%s possible corrupt db %s", __func__, m_strDbFileName.c_str() );
            vx_assert( false );
			return 0;
		}

		if( SQLITE_ROW == sqlite3_step( poSqlStatement ) )
		{
			iVersion = sqlite3_column_int(poSqlStatement, 0 );
		}

		sqlite3_finalize(poSqlStatement);
		sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
		dbClose();
	}
	else
	{
		LogMsg( LOG_ERROR, "DbBase::readDatabaseVersion db failed to open" );
	}

	m_DbMutex.unlock();
	return iVersion;
}

//============================================================================
//! write database version to version table 
int32_t DbBase::writeDatabaseVersion( int iDbVersion )
{
	int32_t rc = SQLITE_OK;
	DbBindList bindList( iDbVersion );
	if( dbTableExists( "DBBASE_VERSION" ))
	{
		return sqlExec( "UPDATE DBBASE_VERSION SET db_version=?", bindList );
	}
	else
	{
		rc = sqlExec("CREATE TABLE DBBASE_VERSION (db_version INTEGER)");
		if( SQLITE_OK == rc )
		{
			return sqlExec( "INSERT INTO DBBASE_VERSION (db_version) VALUES (?)", bindList );
		}
	}

	return rc;
}

//============================================================================
//! return true if table exists
bool DbBase::dbTableExists( const char* pTableName )
{
	m_DbMutex.lock();
	dbOpen();
	bool bHasTable = false;
	sqlite3_stmt * poSqlStatement = NULL;
	char as8Buf[ 256 ];

	sprintf( as8Buf, "SELECT name FROM sqlite_master WHERE type='table' AND name=?");

	if( SQLITE_OK == sqlite3_prepare_v2( m_Db, as8Buf, (int)strlen( as8Buf )+1, &poSqlStatement, NULL ) ) 
	{
		int result;
		result = sqlite3_bind_text( poSqlStatement       , 
								 1                    , 
								 pTableName           , 
								 strlen( pTableName ) , 
								 SQLITE_TRANSIENT     );

		if( SQLITE_OK == result )
		{
			if( SQLITE_ROW == sqlite3_step( poSqlStatement ) )
			{
				bHasTable = true;
			}
		}
	}

	sqlite3_finalize( poSqlStatement );
	dbClose();
	m_DbMutex.unlock();
	return bHasTable;
}


//============================================================================
int32_t DbBase::sqlExec( const char*		SQL_Statement, 
                       DbBindList&		bindList )
{
    int32_t result = 0;
	sqlite3_stmt* pSqlStatement	= nullptr;
	enum ESqlExecStep sqlStep{eSqlExecStepDbOpen};
	enum ESqlExecStep sqlErrorStep{eSqlExecStepDbOpen};

	do
	{					 
		switch( sqlStep )
		{
		case eSqlExecStepError:
			if( result != SQLITE_OK )
			{
				LogMsg( LOG_ERROR, "DbBase::sqlExec error %d in step %s", result, DescribeSqlStep( sqlErrorStep ) );
			}

			if( m_Db )
			{
				LogMsg( LOG_ERROR, "DbBase::sqlExec error %s in step %s db %s", sqlite3_errmsg(m_Db), DescribeSqlStep( sqlErrorStep ), m_strDbFileName.c_str() );
			}

			dbClose();
			return -1;

		case eSqlExecStepDbOpen:
			sqlStep = eSqlExecStepPrepStatement;
			result = dbOpen();
			if( SQLITE_OK != result )
			{
				sqlErrorStep = eSqlExecStepDbOpen;
				sqlStep = eSqlExecStepError;
			}

			break;

		case eSqlExecStepPrepStatement:
			sqlStep = eSqlExecStepBindParams;
			result = sqlite3_prepare_v2( m_Db                             , 
										SQL_Statement                      , 
										( int )strlen( SQL_Statement ) + 1 , 
										&pSqlStatement                     , 
										 NULL                               );
			if( SQLITE_OK != result )
			{
				sqlErrorStep = eSqlExecStepPrepStatement;
				sqlStep = eSqlExecStepError;
			}
			break;

		case eSqlExecStepBindParams:
			sqlStep = eSqlExecStepFinalize;
			if( false == bindParams( pSqlStatement, bindList ) )
			{
				sqlErrorStep = eSqlExecStepBindParams;
				sqlStep = eSqlExecStepError;
			}

			break;

		case eSqlExecStepFinalize:
			sqlStep = eSqlExecStepDbClose;

			if( SQLITE_DONE != sqlite3_step(pSqlStatement) )
			{
				sqlErrorStep = eSqlExecStepFinalize;
				sqlStep = eSqlExecStepError;
				LogMsg( LOG_ERROR, "DbBase::sqlExec:ERROR %s while stepping db %s", sqlite3_errmsg(m_Db), m_strDbFileName.c_str() );
			}
			else
			{
				if( SQLITE_OK != sqlite3_finalize(pSqlStatement) ) 
				{
					sqlErrorStep = eSqlExecStepFinalize;
					sqlStep = eSqlExecStepError;
					LogMsg( LOG_ERROR, "DbBase::sqlExec:ERROR %s in finalize db %s", sqlite3_errmsg(m_Db), m_strDbFileName.c_str() );
				}
			}

			break;

		default:
			break;
		}

	} while( sqlStep != eSqlExecStepDbClose );

	//if( true == needToFinalize )
	//{
	//	sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
	//}

 //   if( retVal )
 //   {
 //       LogMsg( LOG_ERROR, "DbBase::sqlExec returning error %d %s for statement %s",
 //               retVal,
 //               sqlite3_errmsg( m_Db ),
 //               SQL_Statement );
 //   }

	result = dbClose();

	return 0;
}

//============================================================================
bool DbBase::bindParams( sqlite3_stmt* sqlStmt, DbBindList& bindParams )
{
	int result{ 0 };
	bool firstParam{ true };
	int bindNum{ 1 }; // binds start at 1 (not 0)
	DbBindParam* dbBind;
	do {
		bool retrievedBind{ false };
		dbBind = nullptr;
		if( firstParam )
		{
			firstParam = false;
			retrievedBind = bindParams.getFirst( &dbBind );
		}
		else
		{
			retrievedBind = bindParams.getNext( &dbBind );
		}

		if( !retrievedBind )
		{
			return false;
		}

		switch( dbBind->getType() )
		{
		case eDbBindTypeText:

			result = sqlite3_bind_text(	sqlStmt, 
										bindNum, 
										dbBind->getText(), 
										dbBind->getTextLen(), 
										SQLITE_TRANSIENT );
			break;

		case eDbBindTypeInt:

			result = sqlite3_bind_int(	sqlStmt, 
										bindNum, 
										dbBind->getInt() );
			break;

		case eDbBindTypeS64:

			result = sqlite3_bind_int64(	sqlStmt, 
											bindNum, 
											dbBind->getS64() );
			break;

		case eDbBindTypeBlob:
			{

				int len = 0;
				void * pvBlob = dbBind->getBlob( len );
				result = sqlite3_bind_blob( sqlStmt, 
											bindNum, 
											pvBlob, 
											len,
											SQLITE_TRANSIENT );
			}

			break;

        default:
            LogMsg( LOG_ERROR,"DbBase::%s UNKNOWN bind type db %s", __func__, m_strDbFileName.c_str());
            return false;
		}

		if( SQLITE_OK != result )
		{
			LogMsg( LOG_ERROR,"DbBase::%s bind error %s db %s", __func__,  m_strDbFileName.c_str());
		}

		bindNum++;
		if( !bindParams.hasNext() )
		{
			// all done
			return true;
		}

	} while( true );

	return false;
}

//============================================================================
int32_t DbBase::sqlExec( std::string& statement )
{
	return sqlExec( statement.c_str() );
}

//============================================================================
int32_t DbBase::sqlExec( const char* SQL_Statement )
{
	char *SQL_Error;
	int retval;
	int32_t rc = dbOpen();
	if( 0 == rc )
	{
		retval = sqlite3_exec( m_Db, SQL_Statement, NULL, NULL, &SQL_Error );
		if (!(SQLITE_OK == retval))
		{
			handleSqlError( 0, "DbBase:sqlite3_exec: error %s executing %s db %s", SQL_Error, SQL_Statement, m_strDbFileName.c_str() );
		
			sqlite3_free(SQL_Error);
			dbClose();
			return -1;
		}

		rc = dbClose();
	}

	return rc;
}

//============================================================================
void DbBase::handleSqlError( int32_t rc, const char* errMsg, ... )
{
	std::array<char, 1024> szBuffer;
	va_list arg_ptr;
	va_start(arg_ptr, errMsg);
	vsnprintf(szBuffer.data(), szBuffer.size(), errMsg, arg_ptr);
	va_end(arg_ptr);
	szBuffer[szBuffer.size() - 1] = 0;

	LogMsg( LOG_ERROR, szBuffer.data() );
	onSqlError( rc, szBuffer.data() );
}

//============================================================================
//! start query and use DbCursor to access column's data.. be sure to call DbCursor.close() when done
DbCursor * DbBase::startQuery( const char* pSqlString )
{
	vx_assert( pSqlString );
	int32_t rc = dbOpen();
	if( 0 == rc )
	{
		sqlite3_stmt * poSqlStatement = nullptr;
		int iResult = sqlite3_prepare_v2( m_Db, pSqlString, (int)strlen( pSqlString ), &poSqlStatement, NULL );
		if( SQLITE_OK != iResult ) 
		{
			const char* sqliteErrMsg = sqlite3_errmsg(m_Db);
			handleSqlError( LOG_ERROR, "DbBase::StartDataQuery: error %s statement %s db %s", sqliteErrMsg, pSqlString, m_strDbFileName.c_str() );
			return NULL;
		}
		DbCursor * poCursor = new DbCursor();
		poCursor->m_DbBase = this;
		poCursor->m_Stmt = poSqlStatement;
		return poCursor;
	}

	return NULL;
}

//============================================================================
DbCursor* DbBase::startQuery( const char* pSqlString, const char* textParam )
{
	vx_assert( pSqlString );
	const char*   srcStr = "DbBase::StartDataQueryTxt: error";
	DbCursor*     retVal = NULL;
	int           iResult;

	sqlite3_stmt* poSqlStatement = nullptr;

	if( 0 == dbOpen() )
	{
		poSqlStatement = NULL;
		iResult = sqlite3_prepare_v2(	m_Db, 
										pSqlString, 
										( int )strlen( pSqlString ), 
										&poSqlStatement, 
										NULL );

		if( SQLITE_OK == iResult ) 
		{
			iResult = sqlite3_bind_text(	poSqlStatement, 
											1, 
											textParam, 
                                            strlen( textParam ),
											SQLITE_TRANSIENT );

			if( SQLITE_OK == iResult )
			{
				retVal = new DbCursor();
				retVal->m_DbBase = this;
				retVal->m_Stmt = poSqlStatement;
			}
			else
			{
				LogMsg( LOG_ERROR, "ERROR: %s BIND TEXT %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "ERROR: %s %s statement %s db %s", srcStr, sqlite3_errmsg( m_Db ), pSqlString, m_strDbFileName.c_str() );
		}
	}

	return retVal;
}

//============================================================================
DbCursor* DbBase::startQuery( const char* pSqlString, const char* textParam, int secondParam )
{
	vx_assert( pSqlString );
	const char* srcStr = "DbBase::StartDataQueryTxtInt: error";
	DbCursor* retVal = NULL;
	int           iResult;

	sqlite3_stmt* poSqlStatement = nullptr;

	if( 0 == dbOpen() )
	{
		poSqlStatement = NULL;
		iResult = sqlite3_prepare_v2( m_Db,
			pSqlString,
			( int )strlen( pSqlString ),
			&poSqlStatement,
			NULL );

		if( SQLITE_OK == iResult )
		{
			iResult = sqlite3_bind_text( poSqlStatement,
				1,
				textParam,
				strlen( textParam ),
				SQLITE_TRANSIENT );

			if( SQLITE_OK == iResult )
			{
				iResult = sqlite3_bind_int( poSqlStatement,
					2,
					secondParam);

				if( SQLITE_OK == iResult )
				{
					retVal = new DbCursor();
					retVal->m_DbBase = this;
					retVal->m_Stmt = poSqlStatement;
				}
				else
				{
					LogMsg( LOG_ERROR, "ERROR: %s BIND INTEGER %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
				}
			}
			else
			{
				LogMsg( LOG_ERROR, "ERROR: %s BIND TEXT %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "ERROR: %s %s statement %s db %s", srcStr, sqlite3_errmsg( m_Db ), pSqlString, m_strDbFileName.c_str() );
		}
	}

	return retVal;
}

//============================================================================
DbCursor* DbBase::startQuery( const char* pSqlString, const char* textParam1, const char* textParam2, int thirdParam )
{
	vx_assert( pSqlString );
	const char* srcStr = "DbBase::StartDataQueryTxtInt: error";
	DbCursor* retVal = NULL;
	int           iResult;

	sqlite3_stmt* poSqlStatement = nullptr;

	if( 0 == dbOpen() )
	{
		poSqlStatement = NULL;
		iResult = sqlite3_prepare_v2( m_Db,
			pSqlString,
			( int )strlen( pSqlString ),
			&poSqlStatement,
			NULL );

		if( SQLITE_OK == iResult )
		{
			iResult = sqlite3_bind_text( poSqlStatement,
				1,
				textParam1,
				strlen( textParam1 ),
				SQLITE_TRANSIENT );

			if( SQLITE_OK == iResult )
			{
				iResult = sqlite3_bind_text( poSqlStatement,
					1,
					textParam2,
					strlen( textParam2 ),
					SQLITE_TRANSIENT );

				if( SQLITE_OK == iResult )
				{
					iResult = sqlite3_bind_int( poSqlStatement,
						2,
						thirdParam );

					if( SQLITE_OK == iResult )
					{
						retVal = new DbCursor();
						retVal->m_DbBase = this;
						retVal->m_Stmt = poSqlStatement;
					}
					else
					{
						LogMsg( LOG_ERROR, "ERROR: %s BIND INTEGER %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
					}
				}
				else
				{
					LogMsg( LOG_ERROR, "ERROR: %s BIND TEXT2 %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
				}
			}
			else
			{
				LogMsg( LOG_ERROR, "ERROR: %s BIND TEXT1 %s db %s", srcStr, sqlite3_errmsg( m_Db ), m_strDbFileName.c_str() );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "ERROR: %s %s statement %s db %s", srcStr, sqlite3_errmsg( m_Db ), pSqlString, m_strDbFileName.c_str() );
		}
	}

	return retVal;
}

//============================================================================
//! start query and use DbCursor to access column's data.. be sure to call DbCursor.close() when done
// TODO use bind list to avoid possible injection
DbCursor * DbBase::startQueryInsecure( const char* pSqlString, ... )
{
	char SQL_Statement[4095];

	va_list arg_ptr;
	va_start(arg_ptr, pSqlString);
	vsnprintf(SQL_Statement, 4095, pSqlString, arg_ptr);
	SQL_Statement[4094] = 0;
	va_end(arg_ptr);
	return startQuery( SQL_Statement );
}

//============================================================================
//! get the row id of the last inserted row
int64_t DbBase::getLastInsertId( void )
{
	if( m_Db )
	{
		return sqlite3_last_insert_rowid( m_Db );
	}

	return 0;
}

//============================================================================
DbCursor::DbCursor()
{
}

//============================================================================
//! close database and delete ourself
void DbCursor::close( void )
{
	int iResult = sqlite3_finalize( m_Stmt );

	if( SQLITE_OK != iResult ) 
	{
		LogMsg( LOG_ERROR, "DatabaseClass::CloseQuery: error %s db %s", sqlite3_errmsg(m_DbBase->m_Db), m_DbBase->m_strDbFileName.c_str() );
	}

	vx_assert( ( DbBase* )0 != m_DbBase );
	m_DbBase->dbClose();
	delete this;
}

//============================================================================
bool DbCursor::getNextRow( void )
{
	int iResult = sqlite3_step( m_Stmt );
	switch( iResult ) 
	{
	case SQLITE_ROW:
		return true;
		break;
	case SQLITE_DONE:
		return false;
		break;
	default:
		LogMsg( LOG_ERROR, "DbBase::GetDbRow: error %s db %s", sqlite3_errmsg(m_DbBase->m_Db), m_DbBase->m_strDbFileName.c_str() );
		break;
	}
	return false;
}

//============================================================================
uint8_t DbCursor::getByte( int iColumnIdx )
{
	return (uint8_t)sqlite3_column_int( m_Stmt, iColumnIdx );
}

//============================================================================
int32_t DbCursor::getS32( int iColumnIdx )
{
	return (int32_t)sqlite3_column_int( m_Stmt, iColumnIdx );
}

//============================================================================
int64_t DbCursor::getS64( int iColumnIdx )
{
	return sqlite3_column_int64( m_Stmt, iColumnIdx );
}

//============================================================================
float DbCursor::getF32( int iColumnIdx )
{
	return (float)sqlite3_column_double( m_Stmt, iColumnIdx );
}

//============================================================================
double DbCursor::getF64( int iColumnIdx )
{
	return (double)sqlite3_column_double( m_Stmt, iColumnIdx );
}

//============================================================================
const char* DbCursor::getString(int iColumnIdx )
{
	const char* charStr = (const char *)sqlite3_column_text( m_Stmt, iColumnIdx );
	if( !charStr )
	{
		LogMsg( LOG_ERROR, "DbCursor::getString: error %s db %s", sqlite3_errmsg(m_DbBase->m_Db), m_DbBase->m_strDbFileName.c_str() );
		return "";
	}

	return charStr;
}

//============================================================================
//! return blob from column.. 
//! if( piMaxLen != null ) then return length of blob in piMaxLen 
void * DbCursor::getBlob( int iColumnIdx, int * piRetLen )
{
	if( nullptr != piRetLen )
	{
		*piRetLen =  sqlite3_column_bytes( m_Stmt, iColumnIdx );
	}

	void * voidPtr = (void *)sqlite3_column_blob( m_Stmt, iColumnIdx );
	if( !voidPtr )
	{
		LogMsg( LOG_ERROR, "DbCursor::getBlob: error %s db %s", sqlite3_errmsg(m_DbBase->m_Db), m_DbBase->m_strDbFileName.c_str() );
	}

	return voidPtr;
}


//============================================================================
bool DbBase::deleteDatabase( void )
{
	if( m_strDbFileName.empty() )
	{
		LogMsg( LOG_ERROR, "DbBase::deleteDatabase: error file name empty for db %s", m_strDatabaseName.c_str() );
		return false;
	}

	if( VxFileUtil::fileExists( m_strDbFileName.c_str(), false ) )
	{
		lockDb();
		VxFileUtil::deleteFile( m_strDbFileName.c_str() );
		unlockDb();
	}

	int32_t rc = doDatabaseStartup();


	return 0 == rc;
}
