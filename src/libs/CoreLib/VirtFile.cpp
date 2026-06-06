//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VirtFile.h"
#include "VirtFileMgr.h" 

#include "VxDebug.h"


class VirtFile
{
public:
	VirtFile() = delete;
	VirtFile( VFile* vFile, const char* fileName, const char* fileMode );


protected:
	//=== vars ===//
	VFile*						m_VFile{ nullptr };
	std::string					m_FileName;
	std::string					m_FileMode;
};


extern VirtFileMgr& GetVirtFileMgr();

//============================================================================
VirtFile::VirtFile( VFile* vFile, const char* fileName, const char* fileMode )
	: m_VFile(vFile)
	, m_FileName( fileName )
	, m_FileMode( fileMode )
{ 
}

//============================================================================
VFile* VirtFileOpen( const char* fileName, const char* fileMode )
{
	return GetVirtFileMgr().fileOpen( fileName, fileMode );
}

//============================================================================
int VirtFileClose( VFile* fp )
{
	return GetVirtFileMgr().fileClose( fp );
}

//============================================================================
int VirtFileEof( VFile* fp )
{
	return GetVirtFileMgr().fileEof( fp );
}

//============================================================================
int VirtFileError( VFile* fp )
{
	return fp->m_Error;
}

//============================================================================
int VirtFileFlush( VFile* fp )
{
	return GetVirtFileMgr().fileFlush( fp );
}

//============================================================================
size_t VirtFileRead( void* buf, size_t size, size_t count, VFile* fp )
{
	return GetVirtFileMgr().fileRead( buf, size, count, fp );
}

//============================================================================
size_t VirtFileWrite( const void* buf, size_t size, size_t count, VFile* fp )
{
	return GetVirtFileMgr().fileWrite( buf, size, count, fp );
}

//============================================================================
int VirtFileGetC( VFile* fp )
{
	return GetVirtFileMgr().fileGetC( fp );
}

//============================================================================
char* VirtFileGetS( char* buf, int size, VFile* fp )
{
	return GetVirtFileMgr().fileGetS( buf, size, fp );
}

//============================================================================
int VirtFilePutC( int ch, VFile* fp )
{
	return GetVirtFileMgr().filePutC( ch, fp );
}

int VirtFilePutS( const char* s, VFile* fp )
{
	return GetVirtFileMgr().filePutS( s, fp );
}

int VirtFileGetPos( VFile* fp, fpos_t* pos )
{
	return GetVirtFileMgr().fileGetPos( fp, pos );
}

int VirtFileSetPos( VFile* fp, const fpos_t* pos )
{
	return GetVirtFileMgr().fileSetPos( fp, pos );
}

int VirtFileSeek( VFile* fp, size_t offset, int whence )
{
	return GetVirtFileMgr().fileSeek( fp, offset, whence );
}
