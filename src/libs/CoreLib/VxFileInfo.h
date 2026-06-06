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

#include "VxSha1Hash.h"
#include "VxFileTypeMasks.h"

#include <string>

class VxFileInfoBase
{
public:
    VxFileInfoBase() {};

    VxFileInfoBase( const char* fileName, const char* fileNameAndPath, uint8_t fileType = VXFILE_TYPE_UNKNOWN );
    VxFileInfoBase( const char* fileName, const char* fileNameAndPath, int64_t fileLen, uint8_t fileType = VXFILE_TYPE_UNKNOWN );
    VxFileInfoBase( const VxFileInfoBase& rhs );
    VxFileInfoBase& operator=( const VxFileInfoBase& rhs );

    virtual void				setFileName( std::string fileName )		{ m_FileName = fileName; }
    virtual std::string&		getFileName( void )						{ return m_FileName; }

    virtual std::string         getFileExtension( void ) const;

    virtual void				setFileNameAndPath( std::string fileNameAndPath )	{ m_FileNameAndPath = fileNameAndPath; }
    virtual std::string&		getFileNameAndPath( void )				{ return m_FileNameAndPath; }

    virtual std::string         getFilePath( void );

    virtual void				setFileType( uint8_t fileType )			{ m_u8FileType = fileType; }
    virtual uint8_t				getFileType( void ) const				{ return m_u8FileType; }

    virtual bool				getIsMediaFile( void ) const			{ return m_u8FileType & VXFILE_TYPE_AUDIO_VIDEO_PHOTO; }
    virtual bool				getIsVideoFile( void ) const            { return m_u8FileType & VXFILE_TYPE_VIDEO; }
    virtual bool				getIsAudioFile( void ) const            { return m_u8FileType & VXFILE_TYPE_AUDIO; }
    virtual bool				getIsImageFile( void ) const            { return m_u8FileType & VXFILE_TYPE_PHOTO; }

    virtual void				setFileLength( int64_t fileLen )		{ m_s64FileLen = fileLen; }
    virtual int64_t				getFileLength( void ) const				{ return m_s64FileLen; }

    virtual bool                fileIsAvailable( void );

    virtual bool				isContentProviderFile( void ) const;
    virtual bool				isDirectory( void );
    virtual bool				isExecutableFile( void );
    virtual bool				isShortcutFile( void );

    virtual void				assureTrailingDirectorySlash( void );

    static const char*			describeFileType( uint8_t fileType );

protected:
    //=== vars ===//
    int64_t						m_s64FileLen{ 0 };
    uint8_t						m_u8FileType{ VXFILE_TYPE_UNKNOWN };
    std::string					m_FileName;
    std::string					m_FileNameAndPath;
};

class VxFileInfo : public VxFileInfoBase
{
public:
    VxFileInfo() {};

    VxFileInfo( const char* fileName, const char* fileNameAndPath, uint8_t fileType = VXFILE_TYPE_UNKNOWN );
    VxFileInfo( const VxFileInfo& rhs );
	VxFileInfo& operator=( const VxFileInfo& rhs ); 

	VxSha1Hash&					getFileHashId( void )					{ return m_FileHashId; }
	void						setFileHashId( VxSha1Hash& id )			{ m_FileHashId = id; }
	void						setFileHashData( uint8_t * hashData )	{ m_FileHashId.setHashData( hashData ); }

    void						setIsInLibrary( bool inLibrary )		{ m_IsInLibrary = inLibrary; }
	bool						getIsInLibrary( void ) const			{ return m_IsInLibrary; }
	void						setIsSharedFile( bool isShared )		{ m_IsShared = isShared; }
	bool						getIsShared( void ) const				{ return m_IsShared; }

protected:
	//=== vars ===//
	VxSha1Hash					m_FileHashId;
	bool						m_IsInLibrary{ false };
	bool						m_IsShared{ false };

};
