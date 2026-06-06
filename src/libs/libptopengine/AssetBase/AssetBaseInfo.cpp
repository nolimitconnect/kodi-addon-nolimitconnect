//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "AssetBaseInfo.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>

#include <PktLib/VxSearchDefs.h>

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxFileLists.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <sys/types.h>
#include <sys/stat.h>

//============================================================================
AssetBaseInfo::AssetBaseInfo()
    : BaseInfo()
{ 
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType )
    : BaseInfo()
    , m_u16AssetType( (uint16_t) assetType )
{ 
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( const AssetBaseInfo& rhs )
: VxFileInfoBase( rhs )
, BaseInfo( rhs )
, m_PluginType( rhs.m_PluginType )
, m_AssetTag( rhs.m_AssetTag )
, m_UniqueId( rhs.m_UniqueId )
, m_HistoryId( rhs.m_HistoryId )
, m_AdminId( rhs.m_AdminId )
, m_AssetHash( rhs.m_AssetHash )
, m_u16AssetType( rhs.m_u16AssetType )
, m_AttributeFlags( rhs.m_AttributeFlags )
, m_LocationFlags( rhs.m_LocationFlags )
, m_CreationTime( rhs.m_CreationTime )
, m_AccessedTime( rhs.m_AccessedTime ) 
, m_ExpiresTime( rhs.m_ExpiresTime )
, m_AssetSendState( rhs.m_AssetSendState )
, m_PlayPosition0to100000( rhs.m_PlayPosition0to100000 )
, m_DestOnlineId( rhs.m_DestOnlineId )
, m_IsStreaming( rhs.m_IsStreaming )
{   
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( const VxFileInfoBase& rhs )
: VxFileInfoBase( rhs )
{   
	m_u16AssetType = VxFileNameToAssetType( getFileName() );
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( FileInfo& rhs )
    : VxFileInfoBase( rhs )
    , BaseInfo( rhs )
	, m_UniqueId( rhs.getAssetId() )
	, m_HistoryId( rhs.getOnlineId() )
	, m_AssetHash( rhs.getFileHashId() )
    , m_u16AssetType( VxFileNameToAssetType( getFileName() ) )
	, m_CreationTime( rhs.getFileTime() )
{
	m_u16AssetType = rhs.getFileType();
	m_u8FileType = rhs.getFileType(); // TODO is this needed anymore?
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, VxGUID& creatorId, int64_t modifiedTime )
: BaseInfo( creatorId, modifiedTime )
, m_u16AssetType( (uint16_t) assetType )
, m_CreationTime( modifiedTime ? modifiedTime : GetTimeStampMs() )
, m_AccessedTime( m_CreationTime )
{
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType,  VxGUID& creatorId, VxGUID& assetId, int64_t modifiedTime )
: BaseInfo( creatorId, modifiedTime )
, m_UniqueId( assetId )
, m_u16AssetType( (uint16_t) assetType )
, m_CreationTime( modifiedTime ? modifiedTime : GetTimeStampMs() )
, m_AccessedTime( m_CreationTime )
{
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, std::string fileName, std::string fileNameAndPath )
: VxFileInfoBase( fileName.c_str(), fileNameAndPath.c_str(), (uint8_t)assetType )
, BaseInfo()
, m_u16AssetType( (uint16_t) assetType )
, m_CreationTime( GetTimeStampMs() )
, m_AccessedTime( m_CreationTime )
{ 
    BaseInfo::setInfoModifiedTime( m_CreationTime );
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, std::string fileName, std::string fileNameAndPath, VxGUID& assetId )
    : VxFileInfoBase( fileName.c_str(), fileNameAndPath.c_str(), (uint8_t)assetType )
    , BaseInfo()
    , m_UniqueId( assetId )
    , m_u16AssetType( (uint16_t)assetType )
	, m_CreationTime( GetTimeStampMs() )
	, m_AccessedTime( m_CreationTime )
{
	BaseInfo::setInfoModifiedTime( m_CreationTime );
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen )
: VxFileInfoBase( fileName.c_str(), fileNameAndPath.c_str(), fileLen, (uint8_t)assetType )
, BaseInfo()
, m_u16AssetType( (uint16_t) assetType )
, m_CreationTime( GetTimeStampMs() )
, m_AccessedTime( m_CreationTime )
{
    BaseInfo::setInfoModifiedTime( m_CreationTime );
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen, VxGUID& assetId )
    : VxFileInfoBase( fileName.c_str(), fileNameAndPath.c_str(), fileLen, (uint8_t)assetType )
    , BaseInfo()
    , m_UniqueId( assetId )
    , m_u16AssetType( (uint16_t)assetType )
	, m_CreationTime( GetTimeStampMs() )
	, m_AccessedTime( m_CreationTime )

{
	BaseInfo::setInfoModifiedTime( m_CreationTime );
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
AssetBaseInfo::AssetBaseInfo( EAssetType assetType, VxGUID& creatorId, VxGUID& assetId )
: BaseInfo( creatorId )
, m_UniqueId( assetId )
, m_u16AssetType( (uint16_t) assetType )
, m_CreationTime( GetTimeStampMs() )
, m_AccessedTime( m_CreationTime )
{
	m_UniqueId.assureIsValidGUID();
	assureValidTimes();
}

//============================================================================
FileInfo AssetBaseInfo::getFileInfo( void )
{
	FileInfo fileInfo( *this, m_AssetHash );
	if( getDestUserId().isValid() )
	{
		fileInfo.setOnlineId( getDestUserId() );
	}

	return fileInfo;
}

//============================================================================
void AssetBaseInfo::clear( void )
{
    m_FileName.clear();
    m_FileNameAndPath.clear();
}

//============================================================================
bool AssetBaseInfo::addToBlob( PktBlobEntry& blob )
{
	blob.resetWrite();
	if( BaseInfo::addToBlob( blob ) )
	{
		uint8_t pluginType = (uint8_t)m_PluginType;
		bool result = blob.setValue( pluginType );
        result &= blob.setValue( m_FileName );
		result &= blob.setValue( m_UniqueId );
		result &= blob.setValue( m_HistoryId );
		result &= blob.setValue( m_AdminId );
		result &= blob.setValue( m_AssetHash );
        result &= blob.setValue( m_s64FileLen );
		result &= blob.setValue( m_u16AssetType );
		result &= blob.setValue( m_AttributeFlags );
		result &= blob.setValue( m_LocationFlags );
		result &= blob.setValue( m_CreationTime );
		result &= blob.setValue( m_AccessedTime );
		result &= blob.setValue( m_ExpiresTime );
		result &= blob.setValue( m_AssetTag );
		result &= blob.setValue( m_DestOnlineId );
		return result;

		// not sent in blob
        // m_AssetNameAndPath
        // m_AssetSendState
        // m_PlayPosition0to100000
	}

	return false;
}

//============================================================================
bool AssetBaseInfo::extractFromBlob( PktBlobEntry& blob )
{
	blob.resetRead();
	if( BaseInfo::extractFromBlob( blob ) )
	{
		uint8_t pluginType{ 0 };
		bool result = blob.getValue( pluginType );
        result &= blob.getValue( m_FileName );
		result &= blob.getValue( m_UniqueId );
		result &= blob.getValue( m_HistoryId );
		result &= blob.getValue( m_AdminId );
		result &= blob.getValue( m_AssetHash );
        result &= blob.getValue( m_s64FileLen );
		result &= blob.getValue( m_u16AssetType );
		result &= blob.getValue( m_AttributeFlags );
		result &= blob.getValue( m_LocationFlags );
		result &= blob.getValue( m_CreationTime );
		result &= blob.getValue( m_AccessedTime );
		result &= blob.getValue( m_ExpiresTime );
		result &= blob.getValue( m_AssetTag );
		result &= blob.getValue( m_DestOnlineId );
		m_PluginType = (EPluginType)pluginType;
		return result;

		// not sent in blob
        // m_AssetNameAndPath
        // m_AssetSendState
        // m_PlayPosition0to100000
	}

	return false;
}

//============================================================================
bool AssetBaseInfo::validateAssetExist( void )
{
	bool exists = true;
    if( ( isFileAsset() || isThumbAsset() )  && !VxFileUtil::fileExists( getFileNameAndPath().c_str() ) )
	{
		exists = false;
	}

	return exists;
}

//============================================================================
AssetBaseInfo& AssetBaseInfo::operator=( const AssetBaseInfo& rhs ) 
{	
	if( this != &rhs )
	{
        VxFileInfoBase::operator=( rhs );
        BaseInfo::operator=( rhs );
		m_PluginType				= rhs.m_PluginType;
		m_UniqueId					= rhs.m_UniqueId;
		m_HistoryId					= rhs.m_HistoryId; 
		m_AdminId					= rhs.m_AdminId; 
		m_AssetHash					= rhs.m_AssetHash;
		m_u16AssetType				= rhs.m_u16AssetType;
        m_AttributeFlags			= rhs.m_AttributeFlags;
        m_LocationFlags				= rhs.m_LocationFlags;
		m_CreationTime				= rhs.m_CreationTime;
        m_AccessedTime              = rhs.m_AccessedTime;
        m_ExpiresTime               = rhs.m_ExpiresTime;
		m_AssetTag					= rhs.m_AssetTag;
		m_AssetSendState			= rhs.m_AssetSendState;
        m_PlayPosition0to100000     = rhs.m_PlayPosition0to100000;
		m_DestOnlineId				= rhs.m_DestOnlineId;
		m_IsStreaming				= rhs.m_IsStreaming;
	}

	return *this;
}

//============================================================================
bool AssetBaseInfo::isValid( bool logErrIfInvalid )
{
	bool isValidAsset = VXFILE_TYPE_UNKNOWN != m_u16AssetType && m_UniqueId.isValid();
	if( m_AdminId.isValid() )
	{
		isValidAsset &= getPluginType() != ePluginTypeInvalid;
	}

	if( isStream() )
	{
		isValidAsset &= 0 != m_CreationTime && 0 != m_InfoModifiedTime && !m_FileName.empty();
	}
	else if( isFileAsset() )
	{
		isValidAsset &= isValidFile( logErrIfInvalid ) && 0 != m_CreationTime && 0 != m_InfoModifiedTime;
	}
	else if( getAssetType() != eAssetTypeThumbnail && getAssetType() != eAssetTypeChatFace )
	{
		isValidAsset &= getCreatorId().isValid() && 0 != m_CreationTime && 0 != m_InfoModifiedTime;
	}

	if( !isValidAsset )
	{
		if( logErrIfInvalid )
		{
			vx_assert( isValidAsset );

			printValues( LOG_ERROR );
		}
	}

	return isValidAsset;
}

//============================================================================
bool AssetBaseInfo::isValidFile( bool logErrIfInvalid )
{
    bool valid = !getFileName().empty() && m_s64FileLen == (int64_t)VxFileUtil::fileExists( getFileNameAndPath().c_str() );
	if( !valid && logErrIfInvalid )
	{
        LogMsg( LOG_ERROR, "AssetBaseInfo::isValidFile fail %" PRId64 " %s ", m_s64FileLen, m_FileName.empty() ? "NO FILE NAME" : m_FileName.c_str() );
		vx_assert( false );
	}

	return valid;
}

//============================================================================
bool AssetBaseInfo::isFileHashValid( void )
{
	if( !isFileAsset() )
	{
		return true;
	}

	return m_AssetHash.isHashValid();
}

//============================================================================
bool AssetBaseInfo::isValidThumbnail( void )
{
    // dont use isValid.. thumbs may not have a creator id if is an emoticon
    bool valid = getAssetType() == eAssetTypeThumbnail && isValidFile() && m_InfoModifiedTime && m_UniqueId.isValid();
	if( !valid )
	{
		LogMsg( LOG_ERROR, "AssetBaseInfo::isValidThumbnail invalid " );
		vx_assert( false );
	}

	return valid;
}

//============================================================================
bool AssetBaseInfo::isMine( void )
{
	return isValid() && getCreatorId() == GetPtoPEngine().getMyOnlineId();
}

//============================================================================
bool AssetBaseInfo::isMyHistory( void )
{
	return isValid() && getHistoryId() == GetPtoPEngine().getMyOnlineId();
}

//============================================================================
VxGUID& AssetBaseInfo::assureAssetUniqueId( void )
{
	if( !m_UniqueId.isValid() )
	{
		VxGUID::generateNewVxGUID( m_UniqueId );
	}

	return m_UniqueId;
}

//============================================================================
// generates unique id, assigns it to asset and returns reference to it
VxGUID& AssetBaseInfo::generateNewUniqueId( bool ifNotValid )
{
	if( !ifNotValid || ( ifNotValid && !m_UniqueId.isValid() ) )
	{
		VxGUID::generateNewVxGUID( m_UniqueId );
	}

	return m_UniqueId;
}

//============================================================================
bool AssetBaseInfo::isFileAsset( void )
{
	return (0 != ( m_u16AssetType & ( eAssetTypeThumbnail | eAssetTypePhoto | eAssetTypeAudio | eAssetTypeVideo | eAssetTypeDocument | eAssetTypeArchives | eAssetTypeExe | eAssetTypeOtherFiles ) ) );
}

//============================================================================
bool AssetBaseInfo::hasFileName( void )
{
	return isFileAsset() || isThumbAsset();
}

//============================================================================
bool AssetBaseInfo::needsHashGenerated( void )
{
	if( ( false == m_AssetHash.isHashValid() )
		&& hasFileName() )
	{
		return true;
	}

	return false;
}

//============================================================================
bool AssetBaseInfo::isDirectory( void )
{
	return ( VXFILE_TYPE_DIRECTORY & m_u16AssetType ) ? true : false;
}

//============================================================================
void AssetBaseInfo::setAssetTag( const char* assetTagText )
{
	if( assetTagText )
		m_AssetTag = assetTagText;
	else
		m_AssetTag = "";
}

//============================================================================
const char* AssetBaseInfo::getDefaultFileExtension( EAssetType assetType )
{
	const char* extension = ".txt";
	switch( assetType )
	{
	case eAssetTypePhoto:
		extension = ".jpg";
		break;

	case eAssetTypeAudio:
		extension = ".opus";
		break;

	case eAssetTypeVideo:
		extension = ".avi";
		break;

	default:
		break;
	}

	return extension;
}

//============================================================================
const char* AssetBaseInfo::getSubDirectoryName( EAssetType assetType )
{
	const char* subDir = "asset/";
	switch( assetType )
	{
	case eAssetTypeDocument:
		subDir = "documents/";
		break;

	case eAssetTypeArchives:
		subDir = "archives/";
		break;

	case eAssetTypeOtherFiles:
		subDir = "other_files/";
		break;

	case eAssetTypePhoto:
        subDir = "image/";
		break;

	case eAssetTypeAudio:
		subDir = "audio/";
		break;

	case eAssetTypeVideo:
		subDir = "video/";
		break;

	default:
		break;
	}

	return subDir;
}

//============================================================================
void AssetBaseInfo::updateAssetInfo( VxThread* callingThread )
{
	return;
}

//============================================================================
void AssetBaseInfo::printValues( uint32_t logMsgType ) const
{
	LogMsg( logMsgType, "*Begin AssetBaseInfo" );
    LogMsg( logMsgType, "m_AssetName=(%s)", m_FileName.c_str() );
    LogMsg( logMsgType, "m_AssetNameAndPath=(%s)", m_FileNameAndPath.c_str() );
	LogMsg( logMsgType, "m_PluginType=(%s)", GetPluginName( m_PluginType ) );
	LogMsg( logMsgType, "m_UniqueId=(%s)", m_UniqueId.toOnlineIdString().c_str() );
	LogMsg( logMsgType, "m_HistoryId=(%s)", m_HistoryId.toOnlineIdString().c_str() );
	LogMsg( logMsgType, "m_AdminId=(%s)", m_AdminId.toOnlineIdString().c_str() );
	LogMsg( logMsgType, "m_AssetHash=(%s)", m_AssetHash.toString().c_str() );
    LogMsg( logMsgType, "m_s64AssetLen=(%lld)", m_s64FileLen );
	LogMsg( logMsgType, "m_u16AssetType=(%d)", m_u16AssetType );
	LogMsg( logMsgType, "m_AttributeFlags=(0x%4.4X)", m_AttributeFlags );
	LogMsg( logMsgType, "m_AttributeFlags=(0x%8.8X)", m_LocationFlags );
	LogMsg( logMsgType, "m_CreationTime=(%lld)", m_CreationTime );
	LogMsg( logMsgType, "m_AccessedTime=(%lld)", m_AccessedTime );
	LogMsg( logMsgType, "m_ExpiresTime=(%lld)", m_ExpiresTime );
	LogMsg( logMsgType, "m_AssetTag=(%s)", m_AssetTag.c_str() );
	LogMsg( logMsgType, "m_AssetSendState=(%d)", m_AssetSendState );
	LogMsg( logMsgType, "m_PlayPosition0to100000=(%d)", m_PlayPosition0to100000 );

	BaseInfo::printValues( logMsgType );
	LogMsg( logMsgType, "*End AssetBaseInfo" );
}

//============================================================================
void AssetBaseInfo::updateAssetLength( int64_t assetLength )
{
	// TODO regenerate hash id
    setFileLength( assetLength );
}

//============================================================================
void AssetBaseInfo::setHostedId( HostedId& hostedId )
{
	setPluginType( HostTypeToClientPlugin( hostedId.getHostType() ) );
	setAdminId( hostedId.getHostOnlineId() );
}

//============================================================================
HostedId AssetBaseInfo::getHostedId( void )
{
	return HostedId( getAdminId(), PluginTypeToHostType( getPluginType() ) );
}

//============================================================================
GroupieId AssetBaseInfo::getCreatorGroupieId( void )
{
    HostedId hostedId = getHostedId();
    return GroupieId( getCreatorId(), hostedId );
}

//============================================================================
GroupieId AssetBaseInfo::getHistoryGroupieId( void )
{
    HostedId hostedId = getHostedId();
    return GroupieId( m_HistoryId, hostedId );
}

//============================================================================
bool AssetBaseInfo::isHistoryMatch( GroupieId& groupieId )
{
	if( groupieId.getHostType() == eHostTypePeerUser )
	{
		if( ( getPluginType() == ePluginTypeMessenger || getPluginType() == ePluginTypePersonalRecorder ) &&
			( groupieId.getUserOnlineId() == m_HistoryId && groupieId.getHostOnlineId() == m_DestOnlineId ) || // user sent to me
			( groupieId.getHostOnlineId() == m_HistoryId && groupieId.getUserOnlineId() == m_DestOnlineId ) ) // I sent to user
		{
			return true;
		}
	}
	else if( m_AdminId.isValid() && m_AdminId == groupieId.getHostOnlineId() )
	{
		// hosted needs to match host and admin
		if( PluginTypeToHostType( getPluginType() ) == groupieId.getHostType() )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool AssetBaseInfo::isPluginMatch( EPluginType pluginType )
{
	if( pluginType == ePluginTypePersonalRecorder || getPluginType() == ePluginTypePersonalRecorder )
	{
		// personal records should not be visible anywhere except in personal recorder applet
		if( pluginType != getPluginType() )
		{
			return false;
		}
	}

	return true;
}

//============================================================================
GroupieId AssetBaseInfo::getDestGroupieId( void )
{
    HostedId hostedId = getHostedId();
    return GroupieId( getDestUserId(), hostedId );
}

//============================================================================
void AssetBaseInfo::assureValidTimes( void )
{
	if( !m_CreationTime )
    {
        m_CreationTime = GetTimeStampMs();
    }

    if( !m_InfoModifiedTime )
    {
        m_InfoModifiedTime = GetTimeStampMs();
    }
}

//============================================================================
void AssetBaseInfo::setAssetNameFromFileNameAndPath( std::string fullFileName )
{
	setAssetNameAndPath( fullFileName );
	std::string justFileName;
	std::string justPath;
	VxFileUtil::seperatePathAndFile( fullFileName, justPath, justFileName );
	if( !justFileName.empty() )
	{
		setFileName( justFileName );
	}
}

//============================================================================
std::string AssetBaseInfo::describe( void )
{
    if( !isValid( false ) )
    {
        return "Invalid AssetBaseInfo";
    } 
    else if( isPhotoAsset() )
    {
        return "Photo Asset: ";
    }
    else if( isVideoAsset() )
    {
        return "Video Asset: ";
    }
    else if( isAudioAsset() )
    {
        return "Audio Asset: ";
    }
    else if( isThumbAsset() )
    {
        return "Thumbnail Asset ";
    }
    else  if( isChatTextAsset() )
    {
        return "Chat Text Asset ";
    }
    else  if( isChatFaceAsset() )
    {
        return "Chat Face Asset ";
    }
    else
    {
        return "AssetBaseInfo: " + getOnlineId().toOnlineIdString();
    }
}