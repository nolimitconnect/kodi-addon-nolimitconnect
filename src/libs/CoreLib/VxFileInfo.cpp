//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxFileInfo.h"

#include "VxFileIsTypeFunctions.h"
#include "VxFileUtil.h"

//============================================================================
VxFileInfoBase::VxFileInfoBase( const char* fileName, const char* fileNameAndPath, uint8_t fileType )
: m_u8FileType( fileType )
, m_FileName(fileName)
    , m_FileNameAndPath( fileNameAndPath )
{ 
}

//============================================================================
VxFileInfoBase::VxFileInfoBase( const char* fileName, const char* fileNameAndPath, int64_t fileLen, uint8_t fileType )
    : m_s64FileLen( fileLen )
    , m_u8FileType( fileType )
    , m_FileName( fileName )
    , m_FileNameAndPath( fileNameAndPath )
{
}

//============================================================================
VxFileInfoBase::VxFileInfoBase( const VxFileInfoBase& rhs )
    : m_s64FileLen( rhs.m_s64FileLen )
    , m_u8FileType( rhs.m_u8FileType )
    , m_FileName( rhs.m_FileName )
    , m_FileNameAndPath( rhs.m_FileNameAndPath )
{ 
}

//============================================================================
VxFileInfoBase& VxFileInfoBase::operator=(const VxFileInfoBase& rhs)
{ 
    if( this != &rhs )
    {
        m_s64FileLen		= rhs.m_s64FileLen;
        m_u8FileType		= rhs.m_u8FileType;
        m_FileName			= rhs.m_FileName;
        m_FileNameAndPath   = rhs.m_FileNameAndPath;
    }

    return *this;
}

//============================================================================
std::string VxFileInfoBase::getFileExtension( void ) const
{ 
    if( !m_FileName.empty() )
    {
        auto periodPos = m_FileName.rfind( '.' );
        if (periodPos != std::string::npos)
        {
            return m_FileName.substr(periodPos + 1, m_FileName.length());
        }
    }

    return "";
}

//============================================================================
std::string VxFileInfoBase::getFilePath( void )
{
    std::string path;
    if( !isContentProviderFile() )
    {
        std::string fileName;
        VxFileUtil::seperatePathAndFile( m_FileNameAndPath, path, fileName );
    }

    return path;
}

//============================================================================
bool VxFileInfoBase::fileIsAvailable( void )
{
    return VxFileUtil::fileExists( m_FileNameAndPath.c_str() );
}

//============================================================================
bool VxFileInfoBase::isContentProviderFile( void ) const
{
    return VxFileUtil::fileIsProviderFile( m_FileNameAndPath.c_str() );
}

//============================================================================
bool VxFileInfoBase::isExecutableFile( void )
{
    return (VXFILE_TYPE_EXECUTABLE & m_u8FileType);
}

//============================================================================
bool VxFileInfoBase::isDirectory( void )
{
    return (VXFILE_TYPE_DIRECTORY & m_u8FileType);
}

//============================================================================
bool VxFileInfoBase::isShortcutFile( void )
{
    return VxIsShortcutFile( getFileNameAndPath() );
}

//============================================================================
void VxFileInfoBase::assureTrailingDirectorySlash( void )
{
	if( isDirectory() )
	{
        VxFileUtil::assureTrailingDirectorySlash( getFileNameAndPath() );
	}
}

//============================================================================
const char* VxFileInfoBase::describeFileType( uint8_t fileType )
{
	switch( fileType )
	{
	case VXFILE_TYPE_PHOTO:
		return "Photo: ";
	case VXFILE_TYPE_AUDIO:
		return "Audio: ";
	case VXFILE_TYPE_VIDEO:
		return "Video: ";
	case VXFILE_TYPE_DOC:
		return "Documents: ";
	case VXFILE_TYPE_ARCHIVE_OR_CDIMAGE:
		return "Archive Or ISO: ";
	case VXFILE_TYPE_EXECUTABLE:
		return "Executable: ";
	case VXFILE_TYPE_DIRECTORY:
		return "Folder: ";
	default:
		return "Other: ";
	}
}

//============================================================================
VxFileInfo::VxFileInfo( const char* fileName, const char* fileNameAndPath, uint8_t fileType )
: VxFileInfoBase( fileName, fileNameAndPath, fileType )
{ 
}

//============================================================================
VxFileInfo::VxFileInfo(const VxFileInfo& rhs)
    : VxFileInfoBase( rhs )
    , m_FileHashId( rhs.m_FileHashId )
    , m_IsInLibrary( rhs.m_IsInLibrary )
    , m_IsShared( rhs.m_IsShared )
{
}

//============================================================================
VxFileInfo& VxFileInfo::operator=(const VxFileInfo& rhs) 
{	
	if( this != &rhs )
	{
        VxFileInfoBase::operator=( rhs );
		m_FileHashId		= rhs.m_FileHashId;
		m_IsInLibrary		= rhs.m_IsInLibrary;
		m_IsShared			= rhs.m_IsShared;
	}

	return *this;
}
