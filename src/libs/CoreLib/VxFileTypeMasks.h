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

#include <stdint.h>

#define VXFILE_TYPE_UNKNOWN					0x00

#define VXFILE_TYPE_MASK					0xff		
#define VXFILE_TYPE_PHOTO					0x01		
#define VXFILE_TYPE_AUDIO					0x02		
#define VXFILE_TYPE_VIDEO					0x04		
#define VXFILE_TYPE_DOC						0x08
#define VXFILE_TYPE_ARCHIVE_OR_CDIMAGE		0x10
#define VXFILE_TYPE_EXECUTABLE				0x20	
#define VXFILE_TYPE_OTHER					0x40
#define VXFILE_TYPE_DIRECTORY				0x80

#define VXFILE_TYPE_AUDIO_VIDEO				0x06
#define VXFILE_TYPE_AUDIO_VIDEO_PHOTO		0x07
#define VXFILE_TYPE_ALLNOTEXE				0x5F
#define VXFILE_TYPE_ANY						0x7F

enum EFileFilterType 
{
	eFileFilterAll					= 0,

	eFileFilterPhoto				= 1,
	eFileFilterAudio				= 2,
	eFileFilterVideo				= 3,
	eFileFilterDocuments			= 4,
	eFileFilterArchive				= 5,
	eFileFilterOther				= 6,

    eFileFilterEnd                  = 7,

    eFileFilterPhotoOnly            = 8,
    eFileFilterAudioOnly            = 9,
    eFileFilterVideoOnly            = 10,

	eMaxFileFilterType
};

uint8_t FileFilterToVxFileType( EFileFilterType fileFilter );
