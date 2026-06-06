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

#include <BaseInfo/BaseInfo.h>

#include <CoreLib/AssetDefs.h>
#include <CoreLib/GroupieId.h>
#include <CoreLib/HostedId.h>
#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxFileInfo.h>
#include <CoreLib/VxFileTypeMasks.h>
#include <CoreLib/VxSha1Hash.h>

#include <stdint.h>
#include <string>

#define ASSET_LOC_FLAG_PERSONAL_RECORD			0x0001
#define ASSET_LOC_FLAG_LIBRARY					0x0002
#define ASSET_LOC_FLAG_SHARED_FILE				0x0004
#define ASSET_LOC_FLAG_CHAT_ROOM				0x0008

#define ASSET_ATTRIB_FLAG_CIRCULAR			    0x0001
#define ASSET_ATTRIB_TEMPORARY			        0x0002
#define ASSET_ATTRIB_DELETED			        0x0004

class FileInfo;
class VxThread;

class AssetBaseInfo : public VxFileInfoBase, public BaseInfo
{
public:
    AssetBaseInfo();
    AssetBaseInfo( const AssetBaseInfo& rhs );
    AssetBaseInfo( const VxFileInfoBase& rhs );
    AssetBaseInfo( FileInfo& rhs );
    AssetBaseInfo( enum EAssetType assetType );
    AssetBaseInfo( enum EAssetType assetType, VxGUID& onlineId, int64_t modifiedTime = 0 );
    AssetBaseInfo( enum EAssetType assetType, VxGUID& onlineId, VxGUID& assetId, int64_t modifiedTime = 0 );
    AssetBaseInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath );
    AssetBaseInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, VxGUID& assetId );
    AssetBaseInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen );
    AssetBaseInfo( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen, VxGUID& assetId );
    AssetBaseInfo( enum EAssetType assetType, VxGUID& creatorId, VxGUID& assetId );
    virtual ~AssetBaseInfo() = default;

    AssetBaseInfo&				operator=( const AssetBaseInfo& rhs );

    virtual FileInfo            getFileInfo( void );
    virtual VxFileInfoBase      getFileInfoBase( void ) { return *this; }

    virtual void                clear( void );

    virtual bool                addToBlob( PktBlobEntry& blob ) override;
    virtual bool                extractFromBlob( PktBlobEntry& blob ) override;

    virtual void                setPluginType( EPluginType pluginType )         { m_PluginType = pluginType; }
    virtual EPluginType		    getPluginType( void )                           { return m_PluginType; }

    virtual void		        setHostedId( HostedId& hostedId );
    virtual HostedId		    getHostedId( void );
    virtual GroupieId		    getCreatorGroupieId( void );
    virtual GroupieId		    getDestGroupieId( void );
    virtual GroupieId		    getHistoryGroupieId( void );
    virtual bool                isHistoryMatch( GroupieId& groupieId );
    virtual bool                isPluginMatch( EPluginType pluginType );

    virtual bool				isValid( bool logErrIfInvalid = true );
    virtual bool				isValidFile( bool logErrIfInvalid = true );
    virtual bool				isFileHashValid( void );
    virtual bool				isValidThumbnail( void );
    virtual bool				isMine( void );
    virtual bool				isMyHistory( void );

    virtual bool                validateAssetExist( void );

    virtual bool				isDirectory( void ) override;
    virtual bool				isUnknownAsset( void )                          { return ( 0 == m_u16AssetType ) ? true : false; }
    virtual bool				isChatTextAsset( void )                         { return ( eAssetTypeChatText & m_u16AssetType ) ? true : false; }
    virtual bool				isChatFaceAsset( void )                         { return ( eAssetTypeChatFace & m_u16AssetType ) ? true : false; }
    virtual bool				isPhotoAsset( void )                            { return ( eAssetTypePhoto & m_u16AssetType ) ? true : false; }
    virtual bool				isThumbAsset( void )                            { return ( eAssetTypeThumbnail & m_u16AssetType ) ? true : false; }
    virtual bool				isAudioAsset( void )                            { return ( eAssetTypeAudio & m_u16AssetType ) ? true : false; }
    virtual bool				isVideoAsset( void )                            { return ( eAssetTypeVideo & m_u16AssetType ) ? true : false; }
    virtual bool				isFileAsset( void );
    virtual bool				hasFileName( void );

    void						setIsChatRoomRecord( bool isSharedAsset )	    { if( isSharedAsset ) m_LocationFlags |= ASSET_LOC_FLAG_CHAT_ROOM; else m_LocationFlags &= ~ASSET_LOC_FLAG_CHAT_ROOM; }
    bool						isChatRoomRecord( void )				        { return m_LocationFlags & ASSET_LOC_FLAG_CHAT_ROOM ? true : false; }
    virtual void				setIsPersonalRecord( bool isRecord )            { if( isRecord ) m_LocationFlags |= ASSET_LOC_FLAG_PERSONAL_RECORD; else m_LocationFlags &= ~ASSET_LOC_FLAG_PERSONAL_RECORD; }
    virtual bool				isPersonalRecord( void )                        { return m_LocationFlags & ASSET_LOC_FLAG_PERSONAL_RECORD ? true : false; }

    virtual void				setIsInLibrary( bool isInLibrary )              { if( isInLibrary ) m_LocationFlags |= ASSET_LOC_FLAG_LIBRARY; else m_LocationFlags &= ~ASSET_LOC_FLAG_LIBRARY; }
    virtual bool				isInLibrary( void )                             { return m_LocationFlags & ASSET_LOC_FLAG_LIBRARY ? true : false; }
    virtual void				setIsSharedFileAsset( bool isSharedAsset )      { if( isSharedAsset ) m_LocationFlags |= ASSET_LOC_FLAG_SHARED_FILE; else m_LocationFlags &= ~ASSET_LOC_FLAG_SHARED_FILE; }
    virtual bool				isSharedFileAsset( void )                       { return m_LocationFlags & ASSET_LOC_FLAG_SHARED_FILE ? true : false; }

    virtual bool				isMediaFileInUse( void )                        { return m_LocationFlags != 0; }

    virtual void				setIsCircular( bool isCircular )                { if( isCircular ) m_AttributeFlags |= ASSET_ATTRIB_FLAG_CIRCULAR; else m_AttributeFlags &= ~ASSET_ATTRIB_FLAG_CIRCULAR; }
    virtual bool				isCircular( void )                              { return m_AttributeFlags & ASSET_ATTRIB_FLAG_CIRCULAR ? true : false; }
    virtual void				setIsTemporary( bool isTemp )                   { if( isTemp ) m_AttributeFlags |= ASSET_ATTRIB_TEMPORARY; else m_AttributeFlags &= ~ASSET_ATTRIB_TEMPORARY; }
    virtual bool				isTemporary( void )                             { return m_AttributeFlags & ASSET_ATTRIB_TEMPORARY ? true : false; }
    virtual bool				isPermanent( void )                             { return !isTemporary(); }
    virtual void				setIsDeleted( bool isDeleted )                  { if( isDeleted ) m_AttributeFlags |= ASSET_ATTRIB_DELETED; else m_AttributeFlags &= ~ASSET_ATTRIB_DELETED; }
    virtual bool				isDeleted( void )                               { return m_AttributeFlags & ASSET_ATTRIB_DELETED ? true : false; }

    virtual bool 				getIsAssetStreamable( void )                    { return isAudioAsset() || isVideoAsset(); }
    virtual void				setIsStream( bool isStreaming )			        { m_IsStreaming = isStreaming; }
	virtual bool 				isStream( void )						        { return m_IsStreaming; }

    virtual bool                getIsQueued( void )                             { return eAssetSendStateQueued == m_AssetSendState; }

    // assetName is usually just the file name
    virtual void				setAssetName( std::string assetName )           { setFileName( assetName ); }
    virtual void				setAssetNameFromFileNameAndPath( std::string fullFileName );
    virtual std::string&		getAssetName( void )                            { return getFileName(); }

    virtual void				setAssetNameAndPath( std::string assetNameAndPath ) { setFileNameAndPath( assetNameAndPath ); }
    virtual std::string&		getAssetNameAndPath( void )                     { return getFileNameAndPath(); }

    virtual std::string			getRemoteAssetName( void )                      { return getFileName(); }

    virtual void				setAssetTag( const char* assetTagText );
    virtual void				setAssetTag( std::string& assetTagText )        { m_AssetTag = assetTagText; }
    virtual std::string&		getAssetTag( void )                             { return m_AssetTag; }

    virtual void				setAssetType( enum EAssetType assetType )       { m_u16AssetType = (uint16_t)assetType; }
    virtual EAssetType			getAssetType( void )                            { return (EAssetType)m_u16AssetType; }

    virtual void				setAssetLength( int64_t assetLength )           { setFileLength( assetLength ); }
    virtual int64_t				getAssetLength( void )                          { return getFileLength(); }
    virtual void				updateAssetLength( int64_t assetLength );

    virtual void				setAssetHashId( VxSha1Hash& id )                { m_AssetHash = id; }
    virtual void				setAssetHashId( uint8_t * id )                  { m_AssetHash.setHashData( id ); }
    virtual VxSha1Hash&			getAssetHashId( void )                          { return m_AssetHash; }

    virtual void				setAssetUniqueId( VxGUID& uniqueId )            { m_UniqueId = uniqueId; }
    virtual void				setAssetUniqueId( const char* guid )            { m_UniqueId.fromVxGUIDHexString( guid ); }
    virtual VxGUID&				getAssetUniqueId( void )                        { return m_UniqueId; }

    virtual VxGUID&			    assureAssetUniqueId( void );
    virtual VxGUID&				generateNewUniqueId( bool ifNotValid = false ); // generates unique id, assigns it to asset and returns reference to it

    virtual void				setCreatorId( VxGUID creatorId )                { BaseInfo::setOnlineId( creatorId ); }
    virtual void				setCreatorId( const char* creatorId )           { BaseInfo::setOnlineId( creatorId ); }
    virtual VxGUID&				getCreatorId( void )                            { return BaseInfo::getOnlineId(); }

    virtual void				setHistoryId( VxGUID historyId )               { m_HistoryId = historyId; }
    virtual void				setHistoryId( const char* historyId )           { m_HistoryId.fromVxGUIDHexString( historyId ); }
    virtual VxGUID&				getHistoryId( void )                            { return m_HistoryId; }

    virtual void				setAdminId( VxGUID& adminId )                   { m_AdminId = adminId; }
    virtual void				setAdminId( const char* adminId )               { m_AdminId.fromVxGUIDHexString( adminId ); }
    virtual VxGUID&				getAdminId( void )                              { return m_AdminId; }

    virtual void				setLocationFlags( uint32_t locFlags )           { m_LocationFlags = locFlags; }
    virtual uint32_t			getLocationFlags( void )                        { return m_LocationFlags; }

    virtual void				setAttributeFlags( uint16_t locFlags )          { m_AttributeFlags = locFlags; }
    virtual uint16_t			getAttributeFlags( void )                       { return m_AttributeFlags; }

    virtual void				setAssetSendState( enum EAssetSendState sendState )  { m_AssetSendState = sendState; }
    virtual EAssetSendState		getAssetSendState( void )                       { return m_AssetSendState; }

    virtual void				setCreationTime( uint64_t timestamp )           { m_CreationTime = timestamp; m_InfoModifiedTime = timestamp; m_AccessedTime = timestamp; }
    virtual uint64_t			getCreationTime( void )                         { return m_CreationTime; }

    virtual void				setModifiedTime( uint64_t timestamp )           { BaseInfo::setInfoModifiedTime( timestamp ); m_AccessedTime = timestamp; }
    virtual uint64_t			getModifiedTime( void )                         { return BaseInfo::getInfoModifiedTime(); }

    virtual void				setAccessedTime( uint64_t timestamp )           { m_AccessedTime = timestamp; }
    virtual uint64_t			getAccessedTime( void )                         { return m_AccessedTime; }

    virtual void				setExpiresTime( uint64_t timestamp )            { m_ExpiresTime = timestamp; }
    virtual uint64_t			getExpiresTime( void )                          { return m_ExpiresTime; }

    virtual void				setPlayPosition( int pos0to100000 )             { m_PlayPosition0to100000 = pos0to100000; }
    virtual int					getPlayPosition( void )                         { return m_PlayPosition0to100000; }

    virtual void				updateAssetInfo( VxThread* callingThread );
    virtual bool				needsHashGenerated( void );

    static const char*          getDefaultFileExtension( EAssetType assetType );
    static const char*          getSubDirectoryName( EAssetType assetType );

    virtual void                printValues( uint32_t logMsgType = 1 ) const override;

    virtual void				setDestUserId( VxGUID& destOnlineId )           { m_DestOnlineId = destOnlineId; }
    virtual void				setDestUserId( const char* destOnlineId )       { m_DestOnlineId.fromVxGUIDHexString( destOnlineId ); }
    virtual VxGUID&				getDestUserId( void )                           { return m_DestOnlineId; }

    virtual VxGUID&	            getSendToId( void )                             { if( m_DestOnlineId.isValid() ) return m_DestOnlineId; return m_HistoryId; }

    virtual std::string         describe( void );

protected:
    void                        assureValidTimes( void ) override;

public:
    //=== vars ===//
    EPluginType                 m_PluginType{ ePluginTypeInvalid };
	std::string					m_AssetTag;
	VxGUID						m_UniqueId;
	VxGUID						m_HistoryId; 
    VxGUID						m_AdminId; 
	VxSha1Hash					m_AssetHash;
    uint16_t					m_u16AssetType{ VXFILE_TYPE_UNKNOWN };
    uint16_t					m_AttributeFlags{ 0 };
	uint32_t					m_LocationFlags{ 0 };
    int64_t						m_CreationTime{ 0 };
    int64_t						m_AccessedTime{ 0 };
    int64_t						m_ExpiresTime{ 0 }; // time when will be removed. 0 = never
	EAssetSendState			    m_AssetSendState{ eAssetSendStateNone };
    int						    m_PlayPosition0to100000{ 0 };
    VxGUID						m_DestOnlineId; 

    bool                        m_IsStreaming{ false };  // temporary
};
