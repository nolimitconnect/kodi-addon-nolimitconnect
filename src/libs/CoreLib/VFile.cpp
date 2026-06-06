//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VFile.h"
#include "VirtFileMgr.h"

#include <array>

#include <stdarg.h>

//============================================================================
uint64_t VFileExists( const char* fileName )
{
    return GetVirtFileMgr().fileExists( fileName );
}

//============================================================================
bool VFileDirectoryExists( const char* dirPath )
{
    return GetVirtFileMgr().directoryExists( dirPath );
}

//============================================================================
bool VFileIsProviderFile( const char* fileName )
{
    return GetVirtFileMgr().fileIsProviderFile( fileName );
}

//============================================================================
VFile* VFileOpen( const char* fileName, const char* fileMode )
{
	return GetVirtFileMgr().fileOpen( fileName, fileMode );
}

//============================================================================
int VFileClose( VFile* fp )
{
	return GetVirtFileMgr().fileClose( fp );
}

//============================================================================
int VFileEof( VFile* fp )
{
	return GetVirtFileMgr().fileEof( fp );
}

//============================================================================
int VFileError( VFile* fp )
{
	return GetVirtFileMgr().fileError( fp );
}

//============================================================================
int VFileFlush( VFile* fp )
{
	return GetVirtFileMgr().fileFlush( fp );
}

//============================================================================
size_t VFileRead( void* buf, size_t size, size_t count, VFile* fp )
{
	return GetVirtFileMgr().fileRead( buf, size, count, fp );
}

//============================================================================
size_t VFileWrite( const void* buf, size_t size, size_t count, VFile* fp )
{
	return GetVirtFileMgr().fileWrite( buf, size, count, fp );
}

//============================================================================
int VFileGetC( VFile* fp )
{
	return GetVirtFileMgr().fileGetC( fp );
}

//============================================================================
char* VFileGetS( char* buf, int size, VFile* fp )
{
	return GetVirtFileMgr().fileGetS( buf, size, fp );
}

//============================================================================
int VFilePutC( int ch, VFile* fp )
{
	return GetVirtFileMgr().filePutC( ch, fp );
}

//============================================================================
int VFilePutS( const char* s, VFile* fp )
{
	return GetVirtFileMgr().filePutS( s, fp );
}

//============================================================================
int VFileGetPos( VFile* fp, fpos_t* pos )
{
	return GetVirtFileMgr().fileGetPos( fp, pos );
}

//============================================================================
int VFileSetPos( VFile* fp, const fpos_t* pos )
{
	return GetVirtFileMgr().fileSetPos( fp, pos );
}

//============================================================================
int VFileSeek( VFile* fp, size_t offset, int whence )
{
	return GetVirtFileMgr().fileSeek( fp, offset, whence );
}

//============================================================================
int VFileSeek64( VFile* fp, uint64_t offs )
{
	return GetVirtFileMgr().fileSeek64( fp, offs );
}

//============================================================================
int	VFilePrintf( VFile* fp, const char* msg, ... )
{
	const int MAX_PRINTF_LEN = 4096;
	std::array<char, MAX_PRINTF_LEN> szBuffer;
	va_list argList;
	va_start(argList, msg);
	int len = vsnprintf( szBuffer.data(), MAX_PRINTF_LEN, msg, argList);
	va_end(argList);
	if( len > 0 )
	{
		VFileWrite( szBuffer.data(), 1, len, fp );
	}

	return len;
}


//============================================================================
bool VFileGetFileInfo( const char* fileNameAndPath, VxFileInfoBase& fileInfoBase )
{
    return GetVirtFileMgr().getFileInfo( fileNameAndPath, fileInfoBase );
}
