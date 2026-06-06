#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/DbBase.h>
#include <CoreLib/VxGUID.h>

#include <map>

class FileInfo;
class VxSha1Hash;

class FileInfoDb : public DbBase
{
public:
	FileInfoDb() = delete;
	FileInfoDb( std::string fileLibraryDbName );
	virtual ~FileInfoDb() = default;

	void						lockFileInfoDb( void )				{ m_FileInfoDbMutex.lock(); }
	void						unlockFileInfoDb( void )			{ m_FileInfoDbMutex.unlock(); }

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	void 						addFile( VxGUID& onlineId, std::string& fileName, std::string& fileNameAndPath, int64_t fileLen, uint8_t fileType, VxGUID& assetId, VxGUID& thumbId, VxSha1Hash& fileHashId, int64_t fileTime = 0 );
	void 						addFile( FileInfo& libFileInfo );
	void						removeFile( std::string& fileNameAndPath );
	void						removeFile( VxGUID& onlineId, VxGUID& assetId );

	void						getAllFiles( std::map<VxGUID, FileInfo>& sharedFileList );
	void						purgeAllFileLibrary( void ); 

	std::string&				getFileInfoDbName( void ) { return m_FileInfoDbName; }

protected:
	VxMutex						m_FileInfoDbMutex;
	std::string					m_FileInfoDbName{ "" };
};

