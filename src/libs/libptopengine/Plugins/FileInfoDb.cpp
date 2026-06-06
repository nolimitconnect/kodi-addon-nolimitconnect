//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoDb.h"
#include <Plugins/FileInfo.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>

namespace
{
	std::string 		TABLE_FILE_INFO	 				= "file_info";
	std::string 		CREATE_COLUMNS_FILE_INFO		= " (asset_id TEXT PRIMARY KEY, thumb_id TEXT, online_id TEXT, file_name TEXT, fileNameAndPath TEXT, file_length BIGINT, file_type INTEGER, file_time BIGINT, file_hash BLOB ) ";

	const int			COLUMN_FILE_INFO_ASSET_ID		= 0;
	const int			COLUMN_FILE_INFO_THUMB_ID		= 1;
	const int			COLUMN_FILE_INFO_ONLINE_ID		= 2;
	const int			COLUMN_FILE_INFO_FILE_NAME		= 3;
	const int			COLUMN_FILE_NAME_AND_PATH		= 4;
	const int			COLUMN_FILE_INFO_FILE_LEN		= 5;
	const int			COLUMN_FILE_INFO_FILE_TYPE		= 6;
	const int			COLUMN_FILE_INFO_FILE_TIME		= 7;
	const int			COLUMN_FILE_INFO_FILE_HASH		= 8;
}

//============================================================================
FileInfoDb::FileInfoDb( std::string fileLibraryDbName )
	: DbBase( fileLibraryDbName.c_str() )
	, m_FileInfoDbName( fileLibraryDbName )
{
}

//============================================================================
//! create tables in database 
int32_t FileInfoDb::onCreateTables( int iDbVersion )
{
	lockFileInfoDb();
    std::string strCmd = "CREATE TABLE " + TABLE_FILE_INFO + CREATE_COLUMNS_FILE_INFO;
    int32_t rc = sqlExec(strCmd);
	unlockFileInfoDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t FileInfoDb::onDeleteTables( int iOldVersion ) 
{
	lockFileInfoDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_FILE_INFO;
    int32_t rc = sqlExec(strCmd);
	unlockFileInfoDb();
	return rc;
}

//============================================================================
void FileInfoDb::purgeAllFileLibrary( void ) 
{
	lockFileInfoDb();
    std::string strCmd = "DELETE FROM " + TABLE_FILE_INFO;
    int32_t rc = sqlExec( strCmd );
	unlockFileInfoDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "FileInfoDb::purgeAllFileLibrary error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "FileInfoDb::purgeAllFileLibrary success" );
	}
}

//============================================================================
void FileInfoDb::removeFile( std::string& fileNameAndPath )
{
	lockFileInfoDb();
	DbBindList bindList( fileNameAndPath.c_str() );
	sqlExec( "DELETE FROM file_info WHERE fileNameAndPath=?", bindList );
	unlockFileInfoDb();
}

//============================================================================
void FileInfoDb::removeFile( VxGUID& onlineId, VxGUID& assetId )
{
	lockFileInfoDb();
	DbBindList bindList( assetId.toHexString().c_str() );
	sqlExec( "DELETE FROM file_info WHERE asset_id=?", bindList );
	unlockFileInfoDb();
}

//============================================================================
void FileInfoDb::addFile( VxGUID& onlineId, std::string& fileName, std::string& fileNameAndPath, int64_t fileLen, uint8_t fileType, VxGUID& assetId, VxGUID& thumbId, VxSha1Hash& fileHashId, int64_t fileTime )
{
	removeFile( fileNameAndPath );

	lockFileInfoDb();
	DbBindList bindList( assetId.toHexString().c_str() );
	bindList.add( thumbId.toHexString().c_str() );
	bindList.add( onlineId.toHexString().c_str() );
	bindList.add( fileName.c_str() );
	bindList.add( fileNameAndPath.c_str() );
	bindList.add( fileLen );
	bindList.add( (int)fileType );
	bindList.add( fileTime );
	bindList.add( (void *)fileHashId.getHashData(), FILE_HASH_LEN_BYTES );
	
	int32_t rc  = sqlExec( "INSERT INTO file_info (asset_id,thumb_id,online_id,file_name,fileNameAndPath,file_length,file_type,file_time,file_hash) values(?,?,?,?,?,?,?,?,?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "FileInfoDb::%s error %d", __func__, rc );
	}

	unlockFileInfoDb();
}

//============================================================================
void FileInfoDb::addFile( FileInfo& libFileInfo )
{
	addFile(	libFileInfo.getOnlineId(),
				libFileInfo.getFileName(),
				libFileInfo.getFileNameAndPath(),
				libFileInfo.getFileLength(),
				libFileInfo.getFileType(),
				libFileInfo.getAssetId(),
				libFileInfo.getThumbId(),
				libFileInfo.getFileHashId(),
				libFileInfo.getFileTime()
				 );
}

//============================================================================
void FileInfoDb::getAllFiles( std::map<VxGUID, FileInfo>& sharedFileList )
{
	std::string fileName;
	std::string fileNameAndPath;
	uint8_t fileType;
	int64_t fileLen;
	std::string destfile;
	std::string onlineIdStr;
	VxGUID onlineId;
	std::string assetIdStr;
	VxGUID assetId;
	std::string thumbIdStr;
	VxGUID thumbId;
	lockFileInfoDb();
	std::vector<std::string> deletedFiles; 
	DbCursor * cursor = startQuery( "SELECT * FROM file_info" );
	if( cursor )
	{
		while( cursor->getNextRow() )
		{
			onlineIdStr = cursor->getString( COLUMN_FILE_INFO_ONLINE_ID );
			onlineId.fromVxGUIDHexString( onlineIdStr.c_str() );

			assetIdStr = cursor->getString( COLUMN_FILE_INFO_ASSET_ID );
			assetId.fromVxGUIDHexString( assetIdStr.c_str() );

			thumbIdStr = cursor->getString( COLUMN_FILE_INFO_THUMB_ID );
			thumbId.fromVxGUIDHexString( thumbIdStr.c_str() );

			fileName = cursor->getString( COLUMN_FILE_INFO_FILE_NAME );
			fileNameAndPath = cursor->getString( COLUMN_FILE_NAME_AND_PATH );
			fileLen =  cursor->getS64( COLUMN_FILE_INFO_FILE_LEN );
			fileType = (uint8_t)cursor->getS32( COLUMN_FILE_INFO_FILE_TYPE );

			if( fileLen && onlineId.isValid() && assetId.isValid() )
			{
				FileInfo libFileInfo( onlineId, fileName, fileNameAndPath, fileLen, fileType, assetId );
				libFileInfo.setFileHashId( ( uint8_t* )cursor->getBlob( COLUMN_FILE_INFO_FILE_HASH ) );
				uint64_t fileTime = cursor->getS64( COLUMN_FILE_INFO_FILE_TIME );
				libFileInfo.setFileTime( fileTime );
				libFileInfo.setThumbId( thumbId );

				sharedFileList[assetId] = libFileInfo;
			}
			else
			{
				deletedFiles.push_back( fileName );
			}
		}

		cursor->close();
	}

	unlockFileInfoDb();
	std::vector<std::string>::iterator iter;
	for( iter = deletedFiles.begin(); iter != deletedFiles.end(); ++iter )
	{
		removeFile( (*iter) );
	}
}


