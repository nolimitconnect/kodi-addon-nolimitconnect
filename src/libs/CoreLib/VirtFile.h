#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

VFile* VirtFileOpen( const char* fileName, const char* fileMode );
int VirtFileClose(VFile* fp);
int VirtFileEof(VFile* fp);
int VirtFileError(VFile* fp);
int VirtFileFlush(VFile* fp);

size_t VirtFileRead(void* buf, size_t size, size_t count, VFile* fp);
size_t VirtFileWrite(const void* buf, size_t size, size_t count, VFile* fp);

int VirtFileGetC(VFile* fp);
char* VirtFileGetS(char* buf, int size, VFile* fp);
int VirtFilePutC(int ch, VFile* fp);
int VirtFilePutS(const char* s, VFile* fp);

int VirtFileGetPos( VFile* fp, fpos_t* pos );
int VirtFileSetPos( VFile* fp, const fpos_t* pos );
int VirtFileSeek( VFile* fp, size_t offset, int whence);

#ifdef __cplusplus
}
#endif
