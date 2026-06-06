#pragma once
//============================================================================
// Copyright (C) 2012 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/AssetDefs.h>
#include <CoreLib/VxDefs.h>
#include <CoreLib/VxSha1Hash.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/VxFileInfo.h>

#include <string>

class AssetBaseInfo;
class OfferBaseInfo;
class VxFileXferInfo;

class FileInfo : public VxFileInfoBase
{
public:
	static const int FILE_INFO_SHORTEST_SEARCH_TEXT_LEN = 3;
	static const int FILE_INFO_LONGEST_SEARCH_TEXT_LEN = 512;

	FileInfo();
    FileInfo( const FileInfo& rhs );
    FileInfo( const VxFileInfoBase& rhs );
    FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName );
    FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType );
    FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType, VxGUID& assetId );
    FileInfo( VxGUID& onlineId, const std::string& justFileName, const std::string& fullFileName, uint64_t fileLen, uint8_t fileType, VxGUID& assetId, VxSha1Hash& sha1Hash );
	FileInfo( AssetBaseInfo& assetInfo );
	FileInfo( AssetBaseInfo& assetInfo, VxSha1Hash& sha1Hash );
	FileInfo( OfferBaseInfo& offerInfo );
    FileInfo( VxFileXferInfo& xferInfo, VxGUID onlineId );

	FileInfo&					operator=( const FileInfo& rhs );

    VxFileInfoBase&             getFileInfoBase( void ) { return *this; }

	bool						isValid( bool includeHashValid = true );

	bool						isStremable( void );

	void						setIsInLibrary( bool inLibrary )		{ m_IsInLibrary = inLibrary; }
	bool						getIsInLibrary( void )					{ return m_IsInLibrary; }

	void						setIsSharedFile( bool isSharedFile )	{ m_IsSharedFile = isSharedFile; }
	bool						getIsSharedFile( void )				    { return m_IsSharedFile; }

	bool						getIsInUse( void )						{ return m_IsSharedFile || m_IsInLibrary || m_IsStreaming; }

	EAssetType					getAssetType( void );

    void                        setFileNameAndPath( std::string fileNameAndPath ) override;
    std::string&				getLocalFullFileName( void )			{ return getFileNameAndPath(); }
    std::string					getRemoteFileName( void )				{ return getFileName(); }

    std::string					getFilePath( void ) override;

	void						setXferSessionId( VxGUID& sessionId )	{ m_XferSessionId = sessionId; }
	VxGUID&						getXferSessionId( void )				{ return m_XferSessionId; }
	VxGUID&						initializeNewXferSessionId( void ); // init and return unique m_XferSessionId

	void						setOnlineId( VxGUID& assetId )			{ m_OnlineId = assetId; }
	VxGUID&						getOnlineId( void )						{ return m_OnlineId; }

	void						setAssetId( VxGUID& assetId )			{ m_AssetId = assetId; }
	VxGUID&						getAssetId( void )						{ return m_AssetId; }

	void						setThumbId( VxGUID& assetId )			{ m_ThumbId = assetId; }
	VxGUID&						getThumbId( void )						{ return m_ThumbId; }

	void						setFileTime( int64_t fileTime )			{ m_FileTime = fileTime; }
	int64_t						getFileTime( void )	const				{ return m_FileTime; }

	void						setFileHashId( VxSha1Hash& id )			{ m_FileHash = id; }
	void						setFileHashId( uint8_t * id )			{ m_FileHash.setHashData( id ); }
	VxSha1Hash&					getFileHashId( void )					{ return m_FileHash; }

	bool						matchTextAndType( std::string& searchStr, uint8_t fileType );
	bool						matchText( std::string& searchStr );

	int							calcBlobLen( void );
	bool						addToBlob( PktBlobEntry& blob );
	bool						extractFromBlob( PktBlobEntry& blob );

	void						assureValidAssetId( void );

	void						setIsStream( bool isStreaming )			{ m_IsStreaming = isStreaming; }
	bool 						isStream( void )						{ return m_IsStreaming; }

protected:
    bool						determineFilePath( void );
	void						generateAssetId( void );

public:
	//=== vars ===//
	VxGUID						m_OnlineId;
	uint32_t					m_u32Attributes{ 0 };
	VxSha1Hash					m_FileHash;
	std::string					m_ContainedInDir;
	VxGUID						m_AssetId;
	VxGUID						m_ThumbId;
	int64_t						m_FileTime{ 0 };

	VxGUID						m_XferSessionId; // temporary.. not in db and does not get added or extracted from blob
	bool						m_IsInLibrary{ true }; // temporary.. not in db does not get added or extracted from blob
	bool						m_IsSharedFile{ true }; // temporary.. not in db does not get added or extracted from blob

	bool						m_IsStreaming{ false };
};
