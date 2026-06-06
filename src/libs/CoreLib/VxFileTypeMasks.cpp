//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxFileTypeMasks.h"

uint8_t FileFilterToVxFileType( EFileFilterType fileFilter )
{
	uint8_t fileMask = VXFILE_TYPE_ALLNOTEXE;
	switch( fileFilter )
	{
	case eFileFilterPhoto:
	case eFileFilterPhotoOnly:
		return VXFILE_TYPE_PHOTO;

	case eFileFilterAudio:
	case eFileFilterAudioOnly:
		return VXFILE_TYPE_AUDIO;

	case eFileFilterVideo:
	case eFileFilterVideoOnly:
		return VXFILE_TYPE_VIDEO;

	case eFileFilterDocuments:
		return VXFILE_TYPE_DOC;

	case eFileFilterArchive:
		return VXFILE_TYPE_ARCHIVE_OR_CDIMAGE;

	case eFileFilterOther:
		return VXFILE_TYPE_OTHER;

	default:
		break;
	}

	return fileMask;
}
