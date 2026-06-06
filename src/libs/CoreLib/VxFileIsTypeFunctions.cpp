//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "config_corelib.h"

#include "VxFileIsTypeFunctions.h"
#include "VxFileUtil.h"
#include "VxParse.h"

#include <string.h>

#define PHOTO_FILE_EXTENTIONS			"jpg,jpeg,bmp,gif,png,pcx,tif,tiff,ico,pbm,pgm,sgv,svgz,tga,wbmp,webp,xbm,xpm"				
#define AUDIO_FILE_EXTENTIONS			"mp3,wav,wma,ogg,opus"				
#define VIDEO_FILE_EXTENTIONS			"asf,mpg,mpeg,mp4,3gp,mov,avi,divx,mkv,wmv,rm,flv"				
#define DOCUMENT_FILE_EXTENTIONS		"doc,txt,htm,html,pdf"	
#define EXECUTABLE_FILE_EXTENTIONS		"exe,com,bat,cmd"		
#define CDIMAGE_OR_ARC_FILE_EXTENTIONS	"7z,zip,rar,tar,gz,iso,cue,ccd,img,sub,bin,mds,nrg,pdi,mds,vob"
#define THUMBNAIL_FILE_EXTENTIONS	    "nlt"
#define COMBINED_FILE_EXTENTIONS		"jpg,jpeg,bmp,gif,png,pcx,tif,tiff,ico,pbm,pgm,sgv,svgz,tga,wbmp,webp,xbm,xpm,mp3,wav,wma,ogg,asf,mpg,mpeg,mp4,3gp,mov,avi,divx,mkv,wmv,rm,flv,exe,com,bat,cmd,7z,zip,rar,tar,doc,txt,htm,html,pdf,nlt"	

//============================================================================
std::string VxGetFileExtensionsFromFileType( uint8_t fileType )
{
	std::string fileExt;
	if( fileType == VXFILE_TYPE_AUDIO_VIDEO )
	{
		fileExt = VIDEO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += AUDIO_FILE_EXTENTIONS;
	}
	else if( fileType == VXFILE_TYPE_AUDIO_VIDEO_PHOTO )
	{
		fileExt = VIDEO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += AUDIO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += PHOTO_FILE_EXTENTIONS;
	}
	else if( fileType == VXFILE_TYPE_ALLNOTEXE )
	{
		fileExt = VIDEO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += AUDIO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += PHOTO_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += DOCUMENT_FILE_EXTENTIONS;
		fileExt += ",";
		fileExt += CDIMAGE_OR_ARC_FILE_EXTENTIONS;
	}
	else if( fileType == VXFILE_TYPE_ANY )
	{
		fileExt = COMBINED_FILE_EXTENTIONS;
	}
	else
	{
		if( fileType == VXFILE_TYPE_PHOTO )
		{
			fileExt = PHOTO_FILE_EXTENTIONS;
		}
		else if( fileType == VXFILE_TYPE_AUDIO )
		{
			fileExt = AUDIO_FILE_EXTENTIONS;
		}
		else if( fileType == VXFILE_TYPE_VIDEO )
		{
			fileExt = VIDEO_FILE_EXTENTIONS;
		}
		else if( fileType == VXFILE_TYPE_DOC )
		{
			fileExt = DOCUMENT_FILE_EXTENTIONS;
		}
		else if( fileType == VXFILE_TYPE_ARCHIVE_OR_CDIMAGE )
		{
			fileExt = CDIMAGE_OR_ARC_FILE_EXTENTIONS;
		}
		else if( fileType == VXFILE_TYPE_EXECUTABLE )
		{
			fileExt = VXFILE_TYPE_EXECUTABLE;
		}
	}

	return fileExt;
}

//============================================================================
bool VxIsPhotoFile( std::string & cs )
{
	std::string csExt; 
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsPhotoFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsPhotoFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( PHOTO_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsAudioFile( std::string & cs )
{
	std::string csExt; 
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsAudioFileExtention( csExt.c_str() );
}
//============================================================================
bool VxIsAudioFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( AUDIO_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsVideoFile( std::string &cs )
{
	std::string csExt;
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsVideoFileExtention( csExt.c_str() );
}
//============================================================================
bool VxIsVideoFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( VIDEO_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsDocumentFile( std::string &cs )
{
	std::string csExt;
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsDocumentFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsDocumentFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( DOCUMENT_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsArcOrCDImageFile( std::string &cs )
{
	std::string csExt;
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsArcOrCDImageFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsArcOrCDImageFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( CDIMAGE_OR_ARC_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsExecutableFile( std::string &cs )
{
	std::string csExt;
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsExecutableFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsExecutableFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( EXECUTABLE_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsThumbnailFile( std::string &cs )
{
    std::string csExt;
    VxFileUtil::getFileExtension( cs, csExt );
    return VxIsThumbnailFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsThumbnailFileExtention( const char* pExt )
{
    return containsStringCaseInsensitive( THUMBNAIL_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsRecognizedFile( std::string &cs )
{
	std::string csExt;
	VxFileUtil::getFileExtension( cs, csExt );
	return VxIsRecognizedFileExtention( csExt.c_str() );
}

//============================================================================
bool VxIsRecognizedFileExtention( const char* pExt )
{
	return containsStringCaseInsensitive( COMBINED_FILE_EXTENTIONS, pExt );
}

//============================================================================
bool VxIsShortcutFileExtention( const char* pExt )
{
#ifdef TARGET_OS_WINDOWS
	return containsStringCaseInsensitive( pExt, "lnk" );
#else
	return false; // no support for linux yet
#endif //TARGET_OS_WINDOWS
}

//============================================================================
bool VxIsShortcutFile( std::string& fileName )
{
#ifdef TARGET_OS_WINDOWS
	std::string csExt;
	VxFileUtil::getFileExtension( fileName, csExt );
	if( csExt.size() )
	{
		return VxIsShortcutFileExtention( csExt.c_str() );
	}
#endif //TARGET_OS_WINDOWS	
	return false;
}

//============================================================================
uint8_t	VxFileExtensionToFileTypeFlag( const char*	pFileExt )
{
	uint8_t u8FileType = VXFILE_TYPE_OTHER;
	if( pFileExt )
	{
		const char* extension = strrchr( pFileExt, '.' );
		if( extension )
		{
			extension++;
		}
		else
		{
			extension = pFileExt;
		}

		if( VxIsPhotoFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_PHOTO;
		}
		else if( VxIsAudioFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_AUDIO;
		}
		else if( VxIsVideoFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_VIDEO;
		}
		else if( VxIsDocumentFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_DOC;
		}
		else if( VxIsArcOrCDImageFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_ARCHIVE_OR_CDIMAGE;
		}
		else if( VxIsExecutableFileExtention( extension ) )
		{
			u8FileType = VXFILE_TYPE_EXECUTABLE;
		}
	}

	return u8FileType;
}

//============================================================================
bool VxIsMediaFile( uint8_t u8FileTypeFlag )
{
	return ( u8FileTypeFlag & ( VXFILE_TYPE_PHOTO | VXFILE_TYPE_AUDIO | VXFILE_TYPE_VIDEO )) ? true : false;
}

//============================================================================
bool VxShouldOpenFile( uint8_t u8FileTypeFlag )
{
	return ( u8FileTypeFlag & ( VXFILE_TYPE_PHOTO | VXFILE_TYPE_AUDIO | VXFILE_TYPE_VIDEO | VXFILE_TYPE_DOC )) ? true : false;
}

