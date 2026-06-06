#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxMutex.h"

#include <string>
#include <vector>

enum EDbBindType
{
	eDbBindTypeNone, 
	eDbBindTypeText, 
	eDbBindTypeInt, 
	eDbBindTypeS64,    
	eDbBindTypeBlob    
} ;

class DbBindParam
{
public:
	DbBindParam( void );
	DbBindParam( const char* text );
	DbBindParam( int asInt );
	DbBindParam( int64_t asS64 );
	DbBindParam( void * blob, int len );
	virtual ~DbBindParam( void ) = default;

	EDbBindType					getType( void );
	const char*					getText( void );
	int							getInt( void );
	int64_t						getS64( void );
	unsigned short				getTextLen( void );
	void *						getBlob( int& len );

private:
	EDbBindType					m_EDbBindType;
	std::string					m_AsText;
	int							m_AsInt;
	int64_t						m_AsS64;
	void *						m_AsBlob;
	int							m_BlobLen;
};

class DbBindList
{
public:
	DbBindList( void );
	DbBindList( const char* strVal );
	DbBindList( int intVal );
	DbBindList( int64_t s64Val );
	DbBindList( void * blob, int blobLen );
	~DbBindList( void );

	bool						getFirst( DbBindParam** ppBindParam );
	bool						getNext( DbBindParam** ppBindParam );
	bool						hasNext( void );

	void						add( const char* strVal );
	void						add( int64_t s64Val );
	void						add( void * blob, int blobLen );

private:
	std::vector<DbBindParam*>::iterator m_CurrParamIter;
	std::vector<DbBindParam*>           m_ParamList;
};

class DbBase;
struct sqlite3;
struct sqlite3_stmt;

class DbCursor
{
public:
	DbCursor();

	//! close database and delete ourself
	void						close( void );

	//! fetch next row from table
	bool						getNextRow( void );
	//! return 8 bit integer from column
	uint8_t						getByte( int iColumnIdx );
	//! return 32 bit integer from column
	int32_t						getS32( int iColumnIdx );
	//! return 64 bit integer from column
	int64_t						getS64( int iColumnIdx );
	//! return float from column
	float						getF32( int iColumnIdx );
	//! return float from column
	double						getF64( int iColumnIdx );
	//! return string from column
	const char*					getString(int iColumnIdx );
	//! return blob from column.. 
	//! if( piMaxLen != null ) then return length of blob in piMaxLen 
	void *						getBlob( int iColumnIdx, int * piMaxLen = NULL );

	DbBase*						m_DbBase{ nullptr };
	sqlite3_stmt *				m_Stmt{ nullptr };
};

class DbBase
{
public:
	DbBase( std::string databaseName );	
	virtual ~DbBase() = default;

    virtual std::string&        getDatabaseName( void )                 { return m_strDatabaseName; }

	virtual	void				onSqlError( int32_t rc, const char* errMsg ){};
	//=== overrides ===//
	//! create initial database.. 
	virtual int32_t				onCreateDatabase( int iDbVersion );
	//! upgrade db from old version to new version
	virtual int32_t				onUpgradeDatabase(int iOldDbVersion, int iNewDbVersion); 
	//! create tables in database 
	virtual int32_t				onCreateTables( int iDbVersion ) = 0;
	//! delete tables from database 
	virtual int32_t				onDeleteTables( int oldDbVersion ) = 0;

	//! Initialize the database.. if doesn't exist then call onCreateDatabase and onCreateTables
	//! if version does not match then call onDeleteTables and onCreateTables
	virtual int32_t				dbStartup( int iDbVersion, std::string pDbFileName );

	//! shutdown the database
	virtual int32_t				dbShutdown( void );

	//! open the database
	virtual int32_t				dbOpen( void );
	//! close the database
	virtual int32_t				dbClose( void );

	//! read database version from version table 
	virtual int					readDatabaseVersion( void );
	//! write database version to version table 
	virtual int32_t				writeDatabaseVersion( int iDbVersion );

	//! open db then execute Sql statement then close db
	virtual int32_t				sqlExec( std::string& statement );
	//! open db then execute Sql statement then close db
	virtual int32_t				sqlExec( const char* pSqlStatment );
	//! open db then execute Sql statement then close db
	virtual int32_t				sqlExec( const char* SQL_Statement, DbBindList& bindList );

	virtual bool				bindParams( sqlite3_stmt* sqlStmt, DbBindList& bindList );


	//! start query and use DbCursor to access column's data.. be sure to call DbCursor.close() when done
	//! returns null if error
	virtual DbCursor*			startQuery( const char* pSqlString );
	virtual DbCursor*			startQueryInsecure( const char* pSqlString, ... );
	virtual DbCursor*			startQuery( const char* pSqlString, const char* textParam );
	virtual DbCursor*			startQuery( const char* pSqlString, const char* textParam, int secondParam );
	virtual DbCursor*			startQuery( const char* pSqlString, const char* textParam1, const char* textParam2, int thirdParam );

	//! get the row id of the last inserted row
	virtual int64_t				getLastInsertId( void );

	//! return true if table exists
	bool						dbTableExists( const char* pTableName );

	void						lockDb( void )		{ m_DbMutex.lock(); }
	void						unlockDb( void )	{ m_DbMutex.unlock(); }

	bool						deleteDatabase( void );

protected:
	void						handleSqlError( int32_t rc, const char* errMsg, ... );

	int32_t						doDatabaseStartup( void );

public:
	//=== vars ===//
	std::string					m_strDatabaseName;
	std::string					m_strDbFileName;
	bool						m_bDbInitialized;		// if true database initialized
	int							m_iDbVersion;
	sqlite3 *					m_Db;
	VxMutex						m_DbMutex;
};


