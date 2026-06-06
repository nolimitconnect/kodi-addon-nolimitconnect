//============================================================================
// Copyright (C) 2012 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <Plugins/FileInfo.h>
#include <AssetBase/AssetBaseInfo.h>
#include <OfferBase/OfferBaseInfo.h>

#include <PktLib/VxSearchDefs.h>
#include <CoreLib/PktBlobEntry.h>

#include <NetLib/VxFileXferInfo.h>

#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxDebug.h>

#include <sys/types.h>
#include <sys/stat.h>

//============================================================================
FileInfo::FileInfo()
{ 
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( const FileInfo& rhs )
    : VxFileInfoBase( rhs )
    , m_OnlineId( rhs.m_OnlineId )
    , m_u32Attributes( rhs.m_u32Attributes )
    , m_FileHash( rhs.m_FileHash )
    , m_ContainedInDir( rhs.m_ContainedInDir )
    , m_AssetId( rhs.m_AssetId )
    , m_ThumbId( rhs.m_ThumbId )
    , m_FileTime( rhs.m_FileTime )
    , m_XferSessionId( rhs.m_XferSessionId )
    , m_IsInLibrary( rhs.m_IsInLibrary )
    , m_IsSharedFile( rhs.m_IsSharedFile )
	, m_IsStreaming( rhs.m_IsStreaming )
{
    assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( const VxFileInfoBase& rhs )
    : VxFileInfoBase( rhs )
{
}

//============================================================================
FileInfo::FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName )
    : VxFileInfoBase( justFileName.c_str(), fullFileName.c_str() )
    , m_OnlineId( onlineId )
{ 
    determineFilePath();
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType )
    : VxFileInfoBase( justFileName.c_str(), fullFileName.c_str(), fileLen, fileType )
    , m_OnlineId( onlineId )
	, m_u32Attributes(0) 
	, m_ContainedInDir("")
{ 
    determineFilePath();
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType, VxGUID& assetId )
    : VxFileInfoBase( justFileName.c_str(), fullFileName.c_str(), fileLen, fileType )
    , m_OnlineId( onlineId )
	, m_AssetId( assetId )
{
    determineFilePath();
	assureValidAssetId(); // in case assetId is invalid
}

//============================================================================
FileInfo::FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType, VxGUID& assetId, VxSha1Hash& sha1Hash )
    : VxFileInfoBase( justFileName.c_str(), fullFileName.c_str(), fileLen, fileType )
    , m_OnlineId( onlineId )
	, m_FileHash( sha1Hash )
    , m_AssetId( assetId )
{
    determineFilePath();
	assureValidAssetId(); // in case assetId is invalid
}

//============================================================================
FileInfo::FileInfo( AssetBaseInfo& assetInfo )
    : VxFileInfoBase( assetInfo )
    , m_OnlineId( assetInfo.getCreatorId() )
	, m_AssetId( assetInfo.getAssetUniqueId() )
	, m_ThumbId( assetInfo.getThumbId() )
	, m_FileTime( assetInfo.getCreationTime() )
	, m_IsInLibrary( assetInfo.isInLibrary() )
	, m_IsSharedFile( assetInfo.isSharedFileAsset() )
	, m_IsStreaming( assetInfo.isStream() )
{
    determineFilePath();
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( AssetBaseInfo& assetInfo, VxSha1Hash& sha1Hash )
	: FileInfo( assetInfo )
{
	m_FileHash = sha1Hash;
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( OfferBaseInfo& offerInfo )
	: VxFileInfoBase( offerInfo.getAssetName().c_str(), offerInfo.getAssetNameAndPath().c_str(), offerInfo.getAssetLength(), (uint8_t)offerInfo.getAssetType() )
	, m_OnlineId( offerInfo.getHistoryId() )
	, m_FileHash( offerInfo.getOfferHashId() )
    , m_AssetId( offerInfo.getOfferId() )
    , m_ThumbId( offerInfo.getThumbId() )
	, m_FileTime( offerInfo.getCreationTime() )
{
	assureValidAssetId();
}

//============================================================================
FileInfo::FileInfo( VxFileXferInfo& xferInfo, VxGUID onlineId )
	: VxFileInfoBase( xferInfo.getRmtFileName().c_str(), xferInfo.getLclFileNameAndPath().c_str(), xferInfo.getFileLength(), (uint8_t)xferInfo.getAssetType() )
	, m_OnlineId( onlineId )
	, m_FileHash( xferInfo.getFileHashId() )
    , m_AssetId( xferInfo.getAssetId() )
    , m_ThumbId()
	, m_IsStreaming( xferInfo.isStream() )
{
	assureValidAssetId();
}

//============================================================================
FileInfo& FileInfo::operator=( const FileInfo& rhs ) 
{	
    if( &rhs != this)
    {
        getFileInfoBase()       = rhs;
        m_OnlineId				= rhs.m_OnlineId;
        m_u32Attributes			= rhs.m_u32Attributes;
        m_FileHash				= rhs.m_FileHash;
        m_ContainedInDir		= rhs.m_ContainedInDir;
        m_AssetId				= rhs.m_AssetId;
        m_ThumbId				= rhs.m_ThumbId;
        m_FileTime				= rhs.m_FileTime;
        m_XferSessionId			= rhs.m_XferSessionId;
        m_IsInLibrary			= rhs.m_IsInLibrary;
        m_IsSharedFile			= rhs.m_IsSharedFile;
		m_IsStreaming			= rhs.m_IsStreaming;
    }

	return *this;
}

//============================================================================
VxGUID& FileInfo::initializeNewXferSessionId( void )
{
	m_XferSessionId.initializeWithNewVxGUID();
	return m_XferSessionId;
}

//============================================================================
bool FileInfo::isValid( bool includeHashValid )
{
    bool valid = !getFileName().empty() && m_s64FileLen && m_u8FileType && m_AssetId.isValid();
	if( includeHashValid )
	{
		// if using hash then is probably remote and does not require a local path (example web page files)
		valid &= m_FileHash.isHashValid();
	}
	else
	{
		valid &= !getFileNameAndPath().empty();
	}

	return valid;
}

//============================================================================
void FileInfo::setFileNameAndPath( std::string fileNameAndPath )
{
    if( !fileNameAndPath.empty() )
	{
        m_FileNameAndPath = fileNameAndPath;
        determineFilePath();
	}
}

//============================================================================
bool FileInfo::determineFilePath( void )
{
	if( m_ContainedInDir.empty() )
	{
		if( isDirectory() )
		{
            if( !getFileNameAndPath().empty() )
			{
                m_ContainedInDir = getFileNameAndPath();
			}
		}
        else if( !getFileNameAndPath().empty() )
		{
            // with android new permissions and obcsured file names it is not possible to get directory from path and name
#if !defined(TARGET_OS_ANDROID)
			std::string fileName;
            int32_t rc = VxFileUtil::seperatePathAndFile( getFileNameAndPath().c_str(), m_ContainedInDir, fileName );
			if( 0 != rc || m_ContainedInDir.empty() )
			{
                LogMsg( LOG_ERROR, "FileInfo::determineFilePath Failed to get path from %s", getFileNameAndPath().c_str() );
			}
#endif // !defined(TARGET_OS_ANDROID)
		}
	}

	return !m_ContainedInDir.empty() && isValid( false );
}

//============================================================================
std::string FileInfo::getFilePath( void )
{
	determineFilePath();
	if( !m_ContainedInDir.empty() )
	{
		return m_ContainedInDir;
	}

	return VxFileInfoBase::getFilePath();
}

//============================================================================
bool FileInfo::isStremable( void )
{
	return VXFILE_TYPE_AUDIO & m_u8FileType || VXFILE_TYPE_VIDEO & m_u8FileType;
}

//============================================================================
void FileInfo::generateAssetId( void )
{
	if( !m_AssetId.isValid() )
	{
		m_AssetId.initializeWithNewVxGUID();
	}
}

//============================================================================
void FileInfo::assureValidAssetId( void )
{
	generateAssetId();
}

//============================================================================
bool FileInfo::matchTextAndType( std::string& searchStr, uint8_t fileType )
{
	bool result{ false };
	if( searchStr.empty() && !fileType )
	{
		// all
		result = true;
	}
	else if( searchStr.empty() && fileType )
	{
		// by file type only
		result = getFileType() & fileType;
	}
	else if( !searchStr.empty() && !fileType )
	{
		// by search string only
		result = matchText( searchStr );
	}
	else
	{
		// by search string and file type
		result = (getFileType() & fileType) && matchText( searchStr );
	}

	return result;
}

//============================================================================
bool FileInfo::matchText( std::string& searchStr )
{
    return CaseInsensitiveFindSubstr( m_FileName, searchStr ) >= 0;
}

//============================================================================
int FileInfo::calcBlobLen( void )
{
	int blobLen{ 0 };
	blobLen += sizeof( int64_t); // m_s64FileLen
    blobLen += getFileName().length() + sizeof(uint32_t); // m_JustFileName
	blobLen += sizeof( uint8_t ); // m_u8FileType
	blobLen += sizeof( int64_t ); // m_FileTime
	blobLen += sizeof( uint64_t ) * 4; // m_AssetId + m_ThumbId
	blobLen += sizeof( uint32_t ) + sizeof( VxSha1Hash ); // m_FileHash
	return blobLen;
}

//============================================================================
bool FileInfo::addToBlob( PktBlobEntry& blob )
{
	bool result = blob.setValue( m_s64FileLen ); 
    result &= blob.setValue( m_FileName );
	result &= blob.setValue( m_u8FileType );
	result &= blob.setValue( m_FileTime );
	result &= blob.setValue( m_AssetId );
	result &= blob.setValue( m_ThumbId );
	result &= blob.setValue( m_FileHash );
	return result;
}

//============================================================================
bool FileInfo::extractFromBlob( PktBlobEntry& blob )
{
	bool result = blob.getValue( m_s64FileLen );
    result &= blob.getValue( m_FileName );
	result &= blob.getValue( m_u8FileType );
	result &= blob.getValue( m_FileTime );
	result &= blob.getValue( m_AssetId );
	result &= blob.getValue( m_ThumbId );
	result &= blob.getValue( m_FileHash );
	return result;
}

//============================================================================
EAssetType FileInfo::getAssetType( void )
{
	switch( m_u8FileType )
	{
	case VXFILE_TYPE_PHOTO:
		return eAssetTypePhoto;

	case VXFILE_TYPE_AUDIO:
		return eAssetTypeAudio;

	case VXFILE_TYPE_VIDEO:
		return eAssetTypeVideo;

	case VXFILE_TYPE_DOC:
		return eAssetTypeDocument;

	case VXFILE_TYPE_ARCHIVE_OR_CDIMAGE:
		return eAssetTypeArchives;

	case VXFILE_TYPE_EXECUTABLE:
		return eAssetTypeExe;

	case VXFILE_TYPE_OTHER:
		return eAssetTypeOtherFiles;

	case VXFILE_TYPE_DIRECTORY:
		return eAssetTypeDirectory;

	default:
		return eAssetTypeUnknown;
	}
}