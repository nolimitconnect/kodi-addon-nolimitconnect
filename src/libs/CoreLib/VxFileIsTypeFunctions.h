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

#include "VxFileInfo.h"

//============================================================================
bool VxIsPhotoFile( std::string& cs );
bool VxIsPhotoFileExtention( const char* pExt );
bool VxIsAudioFile( std::string& cs );
bool VxIsAudioFileExtention( const char* pExt );
bool VxIsVideoFile( std::string& cs );
bool VxIsVideoFileExtention( const char* pExt );
bool VxIsDocumentFile( std::string& cs );
bool VxIsDocumentFileExtention( const char* pExt );
bool VxIsArcOrCDImageFile( std::string& cs );
bool VxIsArcOrCDImageFileExtention( const char* pExt );
bool VxIsExecutableFile( std::string& cs );
bool VxIsExecutableFileExtention( const char* pExt );
bool VxIsThumbnailFile( std::string& cs );
bool VxIsThumbnailFileExtention( const char* pExt );

bool VxIsRecognizedFile( std::string& cs );
bool VxIsRecognizedFileExtention( const char* pExt );

bool VxIsShortcutFileExtention( const char* pExt );
bool VxIsShortcutFile( std::string& cs );

uint8_t	VxFileExtensionToFileTypeFlag( const char*	pFileExt );

bool VxIsMediaFile( uint8_t u8FileTypeFlag ); // multimedia
bool VxShouldOpenFile( uint8_t u8FileTypeFlag ); // includes docs

std::string VxGetFileExtensionsFromFileType( uint8_t fileType );
