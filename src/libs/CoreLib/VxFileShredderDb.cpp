//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxFileShredderDb.h"

#include "VxDebug.h"
#include "VxParse.h"

#include <string.h>

namespace
{
	const char* SHRED_FILES_DB_NAME = "ShredFilesDb.db3";
}

//============================================================================
VxFileShredderDb::VxFileShredderDb()
: DbBase( "VxFileShredderDb" )
{
}

//============================================================================
void VxFileShredderDb::initShredderDb( std::string& dataDirectory )
{
	std::string dbFileName = dataDirectory + SHRED_FILES_DB_NAME;
	dbStartup( 1, dbFileName );
}

//============================================================================
int32_t VxFileShredderDb::onCreateTables( int iDbVersion )
{
	int32_t rc = sqlExec( "CREATE TABLE table_files (file_name TEXT)" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
int32_t VxFileShredderDb::onDeleteTables( int iOldVersion )
{
	int32_t rc = sqlExec( (char *)"DROP TABLE table_files" );
	vx_assert( 0 == rc );
	return rc;
}

//============================================================================
void VxFileShredderDb::getShredList( std::vector<std::string>&	shredList )
{
	m_DbMutex.lock();
	DbCursor * cursor =  startQuery( "SELECT * FROM table_files" ); 
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			const char* tempStr = cursor->getString( 0 );
			if( tempStr 
				&& strlen( tempStr ) )
			{
				shredList.push_back( tempStr );
			}
		}

		cursor->close();
	}

	m_DbMutex.unlock();
}

//============================================================================
void VxFileShredderDb::addFileToShred( std::string& fileName )
{
	m_DbMutex.lock();
	bool fileExists = false;
	DbCursor * cursor = startQuery(  "SELECT * FROM table_files WHERE file_name=?", fileName.c_str() );
	if( NULL != cursor )
	{
		if( cursor->getNextRow() )
		{
			fileExists = true;
		}

		cursor->close();
	}

	if( fileExists )
	{
		// already in database
		m_DbMutex.unlock();
		return;
	}

	DbBindList bindList( fileName.c_str() );
	sqlExec( "INSERT INTO table_files (file_name) VALUES(?)", bindList );
	m_DbMutex.unlock();
}

//============================================================================
void VxFileShredderDb::removeFileToShred( std::string& fileName )
{
	m_DbMutex.lock();
	DbBindList bindList( fileName.c_str() );
	sqlExec( "DELETE FROM table_files WHERE file_name=?", bindList );
	m_DbMutex.unlock();
}

