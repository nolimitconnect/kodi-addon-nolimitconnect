//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <CoreLib/VxFileInfo.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VFile {
	FILE*						m_FILE;
	int64_t						m_FileLen;
	int64_t						m_FileOffs;
	int							m_Error;
	uint16_t					m_FileContentType;
	uint16_t					m_VirtFileType;
    uint16_t					m_ProviderFileType;
} VFile;

uint64_t VFileExists( const char* fileName );
bool	VFileDirectoryExists( const char* dirPath );
bool    VFileIsProviderFile( const char* fileName );

VFile*	VFileOpen( const char* fileName, const char* fileMode );
int		VFileClose( VFile* fp );

int		VFileEof( VFile* fp );
int		VFileError( VFile* fp );
int		VFileFlush( VFile* fp );

size_t	VFileRead( void* buf, size_t size, size_t count, VFile* fp );
size_t	VFileWrite( const void* buf, size_t size, size_t count, VFile* fp );

int		VFileGetC( VFile* fp );
char*	VFileGetS( char* buf, int size, VFile* fp );
int		VFilePutC( int ch, VFile* fp );
int		VFilePutS( const char* s, VFile* fp );

int		VFileGetPos( VFile* fp, fpos_t* pos );
int		VFileSetPos( VFile* fp, const fpos_t* pos );
int		VFileSeek( VFile* fp, size_t offset, int whence );
int		VFileSeek64( VFile* fp, uint64_t offs );

int		VFilePrintf( VFile* fp, const char* msg, ... );

bool    VFileGetFileInfo( const char* fileNameAndPath, VxFileInfoBase& fileInfoBase );

#ifdef __cplusplus
}
#endif
