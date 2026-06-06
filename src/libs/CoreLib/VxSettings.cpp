//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "config_corelib.h"

#include "VxDebug.h"
#include "VxParse.h"
#include "VxSettings.h"
#include "sqlite3.h"

#include <memory.h>
#include <string.h>
#include <stdio.h>

namespace
{
bool IsCorruptDbError( int sqliteErrCode )
{
	const int primaryErrCode = sqliteErrCode & 0xFF;
	return ( SQLITE_CORRUPT == primaryErrCode ) || ( SQLITE_NOTADB == primaryErrCode );
}

int32_t MigrateTableDropKeyColumn( sqlite3* db, const char* tableName, const char* valueType )
{
	char sqlStatement[512];
	char oldTableName[128];
	snprintf( oldTableName, sizeof(oldTableName), "%s_old_v1", tableName );

	snprintf( sqlStatement, sizeof(sqlStatement), "DROP TABLE IF EXISTS %s", oldTableName );
	if( SQLITE_OK != sqlite3_exec( db, sqlStatement, NULL, NULL, NULL ) )
	{
		return -1;
	}

	snprintf( sqlStatement, sizeof(sqlStatement), "ALTER TABLE %s RENAME TO %s", tableName, oldTableName );
	if( SQLITE_OK != sqlite3_exec( db, sqlStatement, NULL, NULL, NULL ) )
	{
		return -2;
	}

	snprintf( sqlStatement, sizeof(sqlStatement), "CREATE TABLE %s (setting TEXT PRIMARY KEY, value %s)", tableName, valueType );
	if( SQLITE_OK != sqlite3_exec( db, sqlStatement, NULL, NULL, NULL ) )
	{
		return -3;
	}

	snprintf( sqlStatement, sizeof(sqlStatement), "INSERT OR REPLACE INTO %s (setting, value) SELECT setting, value FROM %s", tableName, oldTableName );
	if( SQLITE_OK != sqlite3_exec( db, sqlStatement, NULL, NULL, NULL ) )
	{
		return -4;
	}

	snprintf( sqlStatement, sizeof(sqlStatement), "DROP TABLE %s", oldTableName );
	if( SQLITE_OK != sqlite3_exec( db, sqlStatement, NULL, NULL, NULL ) )
	{
		return -5;
	}

	return 0;
}
}


#define VXSETTINGS_DB_VERSION 0x02

//============================================================================
VxSettings::VxSettings( const char* settingDbName )
: DbBase( settingDbName )
{
}

//============================================================================
//! override onCreateTables to create our tables
int32_t VxSettings::onCreateTables( int iDbVersion )
{
	m_DbMutex.lock();
	int32_t rc = sqlExec( (char*)"CREATE TABLE BOOL (setting TEXT PRIMARY KEY, value TINYINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE INT (setting TEXT PRIMARY KEY, value INTEGER)" );
	rc |= sqlExec( (char*)"CREATE TABLE int8_t (setting TEXT PRIMARY KEY, value TINYINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE uint8_t (setting TEXT PRIMARY KEY, value TINYINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE int16_t (setting TEXT PRIMARY KEY, value SMALLINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE uint16_t (setting TEXT PRIMARY KEY, value SMALLINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE int32_t (setting TEXT PRIMARY KEY, value INTEGER)" );
	rc |= sqlExec( (char*)"CREATE TABLE uint32_t (setting TEXT PRIMARY KEY, value INTEGER)" );
	rc |= sqlExec( (char*)"CREATE TABLE int64_t (setting TEXT PRIMARY KEY, value BIGINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE uint64_t (setting TEXT PRIMARY KEY, value BIGINT)" );
	rc |= sqlExec( (char*)"CREATE TABLE float (setting TEXT PRIMARY KEY, value REAL)" );
	rc |= sqlExec( (char*)"CREATE TABLE double (setting TEXT PRIMARY KEY, value REAL)" );
	rc |= sqlExec( (char*)"CREATE TABLE string (setting TEXT PRIMARY KEY, value TEXT)" );
	rc |= sqlExec( (char*)"CREATE TABLE vector_strings (setting TEXT PRIMARY KEY, value TEXT)" );
	rc |= sqlExec( (char*)"CREATE TABLE blob (setting TEXT PRIMARY KEY, value BLOB)" );
	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSettings::DbCreateTables:ERROR %d creating table" );
	}

	m_DbMutex.unlock();
	return rc;
}
//============================================================================
//! override onDeleteTables to delete our tables
int32_t VxSettings::onDeleteTables( int oldDbVersion )
{
	m_DbMutex.lock();
	int32_t rc = sqlExec( (char*)"DROP TABLE BOOL" );
	rc |= sqlExec( (char*)"DROP TABLE INT" );
	rc |= sqlExec( (char*)"DROP TABLE int8_t" );
	rc |= sqlExec( (char*)"DROP TABLE uint8_t" );
	rc |= sqlExec( (char*)"DROP TABLE int16_t" );
	rc |= sqlExec( (char*)"DROP TABLE uint16_t" );
	rc |= sqlExec( (char*)"DROP TABLE int32_t" );
	rc |= sqlExec( (char*)"DROP TABLE uint32_t" );
	rc |= sqlExec( (char*)"DROP TABLE int64_t" );
	rc |= sqlExec( (char*)"DROP TABLE uint64_t" );
	rc |= sqlExec( (char*)"DROP TABLE float" );
	rc |= sqlExec( (char*)"DROP TABLE double" );
	rc |= sqlExec( (char*)"DROP TABLE string" );
	rc |= sqlExec( (char*)"DROP TABLE vector_strings" );
	rc |= sqlExec( (char*)"DROP TABLE blob" );

	m_DbMutex.unlock();
	return rc;
}

//============================================================================
//! override onUpgradeDatabase to migrate schema without data loss
int32_t VxSettings::onUpgradeDatabase( int iOldDbVersion, int iNewDbVersion )
{
	if( ( iOldDbVersion < 2 ) && ( iNewDbVersion >= 2 ) )
	{
		m_DbMutex.lock();
		if( 0 != dbOpen() )
		{
			m_DbMutex.unlock();
			return -1;
		}

		if( SQLITE_OK != sqlite3_exec( m_Db, "BEGIN IMMEDIATE", NULL, NULL, NULL ) )
		{
			dbClose();
			m_DbMutex.unlock();
			return -2;
		}

		int32_t rc = 0;
		rc |= MigrateTableDropKeyColumn( m_Db, "BOOL", "TINYINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "INT", "INTEGER" );
		rc |= MigrateTableDropKeyColumn( m_Db, "int8_t", "TINYINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "uint8_t", "TINYINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "int16_t", "SMALLINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "uint16_t", "SMALLINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "int32_t", "INTEGER" );
		rc |= MigrateTableDropKeyColumn( m_Db, "uint32_t", "INTEGER" );
		rc |= MigrateTableDropKeyColumn( m_Db, "int64_t", "BIGINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "uint64_t", "BIGINT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "float", "REAL" );
		rc |= MigrateTableDropKeyColumn( m_Db, "double", "REAL" );
		rc |= MigrateTableDropKeyColumn( m_Db, "string", "TEXT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "vector_strings", "TEXT" );
		rc |= MigrateTableDropKeyColumn( m_Db, "blob", "BLOB" );

		if( rc )
		{
			sqlite3_exec( m_Db, "ROLLBACK", NULL, NULL, NULL );
			dbClose();
			m_DbMutex.unlock();
			LogMsg( LOG_ERROR, "VxSettings::onUpgradeDatabase:ERROR %d migrating from %d to %d", rc, iOldDbVersion, iNewDbVersion );
			return -3;
		}

		if( SQLITE_OK != sqlite3_exec( m_Db, "COMMIT", NULL, NULL, NULL ) )
		{
			sqlite3_exec( m_Db, "ROLLBACK", NULL, NULL, NULL );
			dbClose();
			m_DbMutex.unlock();
			return -4;
		}

		dbClose();
		m_DbMutex.unlock();

		return writeDatabaseVersion( iNewDbVersion );
	}

	return DbBase::onUpgradeDatabase( iOldDbVersion, iNewDbVersion );
}

//============================================================================
//! startup Settings.. if database doesn't exist then create it and call DbCreateTables
int32_t VxSettings::vxSettingsStartup( const char* pDbFileName )
{
	int32_t rc = dbStartup( VXSETTINGS_DB_VERSION, pDbFileName );
	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSettings::vxSettingsStartup:ERROR %d", rc );
	}
	return rc;
}

//============================================================================
//! shutdown Settings
void VxSettings::vxSettingsShutdown( void )
{
	int32_t rc = dbShutdown();
	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSettings::vxSettingsShutdown:ERROR %d", rc );
	}
}

//============================================================================
//! remove a key value from database
void VxSettings::removeBoolIniValueFromDb( const char* pSettingName )
{
	m_DbMutex.lock();
	
	char SQL_Statement[ 2048 ];
	if( 0 == dbOpen() )
	{
		const char* tableName = "BOOL";
		sprintf( SQL_Statement, "DELETE FROM %s WHERE setting='%s'", tableName, pSettingName );
		if( SQLITE_OK != sqlite3_exec( m_Db, SQL_Statement, NULL, NULL, NULL ) )
		{
			// LogMsg( LOG_VERBOSE, "VxSettings::removeBoolIniValueFromDb:ERROR %s", sqlite3_errmsg( m_Db ) );
		}
	}

	dbClose();

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, bool& bValue )
{
	m_DbMutex.lock();

	sqlite3_stmt* poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "BOOL", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, (int)bValue ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, int8_t& s8Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "int8_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, s8Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}
//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, uint8_t& u8Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "uint8_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, u8Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}
//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, int16_t& s16Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "int16_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, s16Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, uint16_t& u16Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "uint16_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, u16Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, int32_t& s32Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "int32_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, s32Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, uint32_t& u32Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "uint32_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int( poSqlStatement, 1, u32Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, int64_t& s64Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "int64_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int64( poSqlStatement, 1, s64Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, uint64_t& u64Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "uint64_t", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_int64( poSqlStatement, 1, u64Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, float& f32Value )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "float", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_double( poSqlStatement, 1, f32Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, double& f64Value )
{
	m_DbMutex.lock();
	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "double", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_double( poSqlStatement, 1, f64Value ) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, std::string& strValue )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "string", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_blob( poSqlStatement, 1, strValue.c_str(), (int)strValue.length() + 1, SQLITE_TRANSIENT) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, const char* pValue )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "string", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_blob( poSqlStatement, 1, pValue, (int)strlen(pValue) + 1, SQLITE_TRANSIENT) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! set and save value to database
void VxSettings::setIniValue( const char* pSettingName, std::vector<std::string>& aoStrValues )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "vector_strings", pSettingName ) )
	{
		std::string strCommaDelimStrings;
		std::vector<std::string>::iterator iter;
		for(iter = aoStrValues.begin(); iter != aoStrValues.end(); ++iter )
		{
			strCommaDelimStrings += *iter;
			strCommaDelimStrings += ",";
		}

		bool bError = (SQLITE_OK != sqlite3_bind_blob( poSqlStatement, 1, strCommaDelimStrings.c_str(), (int)strCommaDelimStrings.length() + 1, SQLITE_TRANSIENT) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! save a object as blob into the database
void VxSettings::setIniValue( const char* pSettingName, void * pvObject, int iObjectLen )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 ==  prepareIniSet( &poSqlStatement, "blob", pSettingName ) )
	{
		bool bError = (SQLITE_OK != sqlite3_bind_blob( poSqlStatement, 1, pvObject, iObjectLen, SQLITE_TRANSIENT) );
		finalizeIniSetTransaction( poSqlStatement, bError );
	}

	m_DbMutex.unlock();
}

//============================================================================
//=== set value functions ===//
//============================================================================

//============================================================================
//! get value from database.. return default if doesn't exist
void VxSettings::getIniValue( const char* pSettingName, bool& bValue, bool bDefault )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "BOOL", pSettingName ) )
	{
		bValue = (0 == sqlite3_column_int( poSqlStatement, 0 ) ) ? false : true;
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		bValue = bDefault;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, int8_t& s8Value, int8_t s8Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "int8_t", pSettingName ) )
	{
		s8Value = (int8_t)sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		s8Value = s8Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, uint8_t& u8Value, uint8_t u8Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "uint8_t", pSettingName ) )
	{
		u8Value = (uint8_t)sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		u8Value = u8Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, int16_t& s16Value, int16_t s16Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "int16_t", pSettingName ) )
	{
		s16Value = (int16_t)sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		s16Value = s16Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, uint16_t& u16Value, uint16_t u16Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "uint16_t", pSettingName ) )
	{
		u16Value = (uint16_t)sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		u16Value = u16Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, int32_t& s32Value, int32_t s32Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "int32_t", pSettingName ) )
	{
		s32Value = (int32_t)sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		s32Value = s32Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, uint32_t& u32Value, uint32_t u32Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "uint32_t", pSettingName ) )
	{
		u32Value = sqlite3_column_int( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		u32Value = u32Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, int64_t& s64Value, int64_t s64Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "int64_t", pSettingName ) )
	{
		s64Value = (int64_t)sqlite3_column_int64( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		s64Value = s64Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, uint64_t& u64Value, uint64_t u64Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "uint64_t", pSettingName ) )
	{
		u64Value = (uint64_t)sqlite3_column_int64( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		u64Value = u64Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, float& f32Value, float f32Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "float", pSettingName ) )
	{
		f32Value = (float)sqlite3_column_double( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		f32Value = f32Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, double& f64Value, double f64Default )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	if( 0 < prepareIniQuery( &poSqlStatement, "double", pSettingName ) )
	{
		f64Value = (double)sqlite3_column_double( poSqlStatement, 0 );
		finalizeIniGetTransaction( poSqlStatement );
	}
	else
	{
		f64Value = f64Default;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, std::string& strValue, const char* pDefault )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	int  iBlobBytes;
	int32_t rc = -1;
	if( 0 < ( iBlobBytes = prepareIniQuery( &poSqlStatement, "string", pSettingName ) ) )
	{
		int iStrLen = sqlite3_column_bytes( poSqlStatement, 0 );
		if( iStrLen )
		{
			strValue = (const char*)sqlite3_column_blob( poSqlStatement, 0 );
			rc = 0;
		}
		else
		{
			strValue = "";
		}

		finalizeIniGetTransaction( poSqlStatement );
	}
	if( rc )
	{
		strValue = pDefault;
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, char * pRetBuf, int iBufLen, const char* pDefault )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	int  iBlobBytes;
	int32_t rc = -1;
	if( 0 < ( iBlobBytes = prepareIniQuery( &poSqlStatement, "string", pSettingName ) ) )
	{
		if( iBlobBytes > iBufLen )
		{
			LogMsg( LOG_ERROR, "VxSettings::getIniValue:ERROR setting %s buffer to small", pSettingName  );
		}
		else
		{
			int iStrLen = sqlite3_column_bytes( poSqlStatement, 0 );
			memcpy( pRetBuf, sqlite3_column_blob( poSqlStatement, 0 ), iBlobBytes );
			pRetBuf[iStrLen] = 0;
			rc = 0;
		}

		finalizeIniGetTransaction( poSqlStatement );
	}
	if( rc )
	{
		strcpy( pRetBuf, pDefault );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! get value from database.. return default if doesnt exist
void VxSettings::getIniValue( const char* pSettingName, std::vector<std::string>& aoStrValues )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	int  iBlobBytes;

	if( 0 < ( iBlobBytes = prepareIniQuery( &poSqlStatement, "vector_strings", pSettingName ) ) )
	{
		std::string strCommaDelimStrings;
		int iStrLen = sqlite3_column_bytes( poSqlStatement, 0 );
		if( iStrLen )
		{
			strCommaDelimStrings = (const char*)sqlite3_column_blob( poSqlStatement, 0 );
			StdStringSplit( strCommaDelimStrings, ',', aoStrValues );
		}

		finalizeIniGetTransaction( poSqlStatement );
	}

	m_DbMutex.unlock();
}

//============================================================================
//! copy blob from database.. if length doesnt match will return false
bool VxSettings::getIniValue( const char* pSettingName, void * pvRetBuf, int iBufLen )
{
	m_DbMutex.lock();

	sqlite3_stmt * poSqlStatement{ nullptr };
	int  iBlobBytes;
	bool bSuccess = false;
	if( 0 < ( iBlobBytes = prepareIniQuery( &poSqlStatement, "blob", pSettingName ) ) )
	{
		if( iBlobBytes > iBufLen )
		{
			LogMsg( LOG_ERROR, "VxSettings::getIniValue:ERROR setting %s blob buffer len to small", pSettingName  );
		}
		else
		{
			memcpy( pvRetBuf, sqlite3_column_blob(poSqlStatement, 0 ), iBlobBytes );
			bSuccess = true;
		}

		finalizeIniGetTransaction( poSqlStatement );
	}

	m_DbMutex.unlock();

	return bSuccess;
}

//============================================================================
//! prepare sql statement set value
int32_t VxSettings::prepareIniSet( sqlite3_stmt ** ppoRetSqlStatement,
									const char* pTableName,
									const char* pSettingName )
{
	char SQL_Statement[2048];
	SQL_Statement[0] = 0;
	if( 0 == dbOpen() )
	{
		sprintf(SQL_Statement, "DELETE FROM %s WHERE setting='%s'", pTableName, pSettingName );
		if( SQLITE_OK != sqlite3_exec( m_Db, SQL_Statement, NULL, NULL, NULL ) )
		{
			LogMsg( LOG_VERBOSE, "VxSettings::prepareIniSet:ERROR %s", sqlite3_errmsg(m_Db) );
		}
		sprintf( SQL_Statement, "INSERT INTO %s (setting, value) VALUES ('%s',?)", pTableName, pSettingName ); 
		if( SQLITE_OK == sqlite3_prepare_v2(m_Db, SQL_Statement, (int)strlen(SQL_Statement)+1, ppoRetSqlStatement, NULL) )
		{
			return 0;
		}
		else
		{
			LogMsg( LOG_ERROR, "VxSettings::prepareIniSet:ERROR %s", sqlite3_errmsg(m_Db) );
			vx_assert( false );
		}
	}

    sqlite3_finalize( *ppoRetSqlStatement );
	sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
	dbClose();
	return -1;
}


//============================================================================
//! prepare sql statement to query value
int32_t VxSettings::prepareIniQuery( sqlite3_stmt ** ppoRetSqlStatement,
									const char* pTableName,
									const char* pSettingName )
{
	char SQL_Statement[2048];
	SQL_Statement[0] = 0;
	int32_t rc = dbOpen();
	if( 0 == rc )
	{
		sprintf(SQL_Statement, "SELECT value FROM %s WHERE setting='%s'", pTableName, pSettingName );
		if( SQLITE_OK == sqlite3_prepare_v2(m_Db, SQL_Statement, (int)strlen(SQL_Statement)+1, ppoRetSqlStatement, NULL) )
		{
			if( SQLITE_ROW == (rc = sqlite3_step( * ppoRetSqlStatement ) ) )
			{
				int iBlobBytes = sqlite3_column_bytes( * ppoRetSqlStatement, 0);
				if( iBlobBytes > 0 )
				{
					return iBlobBytes;
				}
				else
				{
					LogMsg( LOG_ERROR, "VxSettings::prepareIniQuery:ERROR %s column bytes table %s setting %s",
						sqlite3_errmsg(m_Db), pTableName, pSettingName );
					rc = -2;
				}
			}
			else if( SQLITE_DONE == rc ) 
			{
				//LogMsg( LOG_VERBOSE, "VxSettings::prepareIniQuery: setting %s NOT FOUND", pSettingName );
				rc = 0;
			}
			else 
			{
				const int sqliteErrCode = sqlite3_extended_errcode( m_Db );
				LogMsg( LOG_ERROR, "VxSettings::prepareIniQuery:ERROR %s stepping table %s setting %s sqliteErrCode %d",
					sqlite3_errmsg(m_Db), pTableName, pSettingName, sqliteErrCode );
				if( IsCorruptDbError( sqliteErrCode ) )
				{
					setIsValid( false );
				}
				rc = -3;
			}
		}
		else
		{
			const int sqliteErrCode = sqlite3_extended_errcode( m_Db );
			LogMsg( LOG_ERROR, "VxSettings::prepareIniQuery:ERROR %s preparing table %s setting %s sqliteErrCode %d",
				sqlite3_errmsg(m_Db), pTableName, pSettingName, sqliteErrCode );
			if( IsCorruptDbError( sqliteErrCode ) )
			{
				setIsValid( false );
			}
			rc = -4;
		}

		sqlite3_finalize( *ppoRetSqlStatement );
		sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
		dbClose();
	}
	else
	{
		rc = -5;
	}

	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSettings::prepareIniQuery:ERROR %d table %s setting %s", rc, pTableName, pSettingName );
	}

	return rc;
}

//============================================================================
//! finalize and close db
int32_t VxSettings::finalizeIniSetTransaction( sqlite3_stmt * poSqlStatement, bool bErrorOccured )
{
	int32_t rc = -1;
	if( false == bErrorOccured )
	{
		if( SQLITE_DONE != sqlite3_step(poSqlStatement) )
		{
            sqlite3_finalize( poSqlStatement );
			LogMsg( LOG_ERROR, "VxSettings::finalizeIniSetTransaction:ERROR %s stepping", sqlite3_errmsg(m_Db) );
		}
		else
		{
			if( SQLITE_OK == ( rc = sqlite3_finalize(poSqlStatement) ) )
			{
				rc = 0;
			}
			else
			{
                sqlite3_finalize( poSqlStatement );
				LogMsg( LOG_ERROR, "VxSettings::finalizeIniSetTransaction:ERROR %s finalize", sqlite3_errmsg(m_Db) );
			}
		}
	}
	else
	{
        sqlite3_finalize( poSqlStatement );
		LogMsg( LOG_ERROR, "VxSettings::finalizeIniSetTransaction:ERROR %s", sqlite3_errmsg(m_Db) );
        vx_assert( false );
	}

   
	sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
	dbClose();
	return rc;
}

//============================================================================
//! finalize and close db
int32_t VxSettings::finalizeIniGetTransaction( sqlite3_stmt * poSqlStatement, bool bErrorOccured )
{
	int32_t rc = -1;
	if( false == bErrorOccured )
	{
		if( SQLITE_OK == ( rc = sqlite3_finalize(poSqlStatement) ) )
		{
			rc = 0;
		}
		else
		{
			LogMsg( LOG_ERROR, "VxSettings::finalizeTransaction:ERROR %s", sqlite3_errmsg(m_Db) );
		}
	}

	sqlite3_exec(m_Db,"END",NULL,NULL,NULL);
	dbClose();
	return rc;
}
