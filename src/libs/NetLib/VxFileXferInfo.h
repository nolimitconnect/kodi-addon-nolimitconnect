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

#include <CoreLib/VxXferDefs.h>

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxSha1Hash.h>
#include <CoreLib/AssetDefs.h>

#include <string>

class VFile;

class VxFileXferInfo
{
public:
    VxFileXferInfo() = default;
	VxFileXferInfo( VxGUID& lclSessionId );
	virtual ~VxFileXferInfo();

	bool						calcProgress( void ); // returns true if has changed since last calcProgress was called
	void						setProgress( int percentProgress )			{ m_PercentProgress = percentProgress; }
	int							getProgress( void )							{ return ( m_PercentProgress < 0 ) ? 0 : m_PercentProgress; }

	void						setFileHashId( uint8_t * fileHashId )		{ m_FileHashId.setHashData( fileHashId ); }
	void						setFileHashId( VxSha1Hash& fileHashId )		{ m_FileHashId = fileHashId; }
	VxSha1Hash&					getFileHashId( void )						{ return m_FileHashId; }

    void						setAssetId( VxGUID& assetId )               { m_AssetId = assetId; }
	VxGUID&						getAssetId( void )							{ return m_AssetId; }
    void						setAssetType( EAssetType assetType )        { m_AssetType = assetType; }
    EAssetType                  getAssetType( void )						{ return m_AssetType; }

	void						setFileOffset( uint64_t fileOffs )			{ m_u64FileOffs = fileOffs; }
	uint64_t					getFileOffset( void )						{ return m_u64FileOffs; }
	void						setFileLength( uint64_t fileLen )			{ m_u64FileLen = fileLen; }
	uint64_t					getFileLength( void )						{ return m_u64FileLen; }

	void						setXferDirection( EXferDirection dir )		{ m_XferDirection = dir; }
	EXferDirection				getXferDirection( void )					{ return m_XferDirection; }

	void						setLclFileName( const char* fileName )		{ m_strLocalFileName = fileName; }
	std::string&				getLclFileName( void )						{ return m_strLocalFileName; }
    void						setLclFileNameAndPath( const char* fileNameAndPath )	{ m_LclFileNameAndPath = fileNameAndPath; }
    std::string&				getLclFileNameAndPath( void )				{ return m_LclFileNameAndPath; }
	void						setRmtFileName( const char* fileName )		{ m_strRemoteFileName = fileName; }
	std::string&				getRmtFileName( void )						{ return m_strRemoteFileName; }

	void						setLclSessionId( VxGUID& lclId )			{ m_LclSessionId = lclId; }
	VxGUID&						getLclSessionId( void )						{ return m_LclSessionId; }
	void						setRmtSessionId( VxGUID& rmtId )			{ m_RmtSessionId = rmtId; }
	VxGUID&						getRmtSessionId( void )						{ return m_RmtSessionId; }

	std::string					getDownloadIncompleteFileName( void );
	std::string					getDownloadCompleteFileName( void );

	void						setIsStream( bool isStreaming )			{ m_IsStreaming = isStreaming; }
	bool 						isStream( void )						{ return m_IsStreaming; }

	void						setIsOpened( bool isOpen )				{ m_IsOpened = isOpen; }
	bool 						isOpened( void )						{ return m_IsOpened || m_hFile; }

	bool						useFileIo( void )						{ return eXferDirectionRx != m_XferDirection || !isStream(); }

	void						clear( void );

	//=== vars ===//
    VFile*						m_hFile{nullptr};
    uint64_t					m_u64FileOffs{0};					 // current offset into file we are at
    uint64_t					m_u64FileLen{0};                     // total file length
	VxGUID						m_AssetId;
    EAssetType                  m_AssetType{eAssetTypeUnknown};
	bool						m_IsOpened{ false };

protected:
	VxGUID						m_LclSessionId;
	VxGUID						m_RmtSessionId;
	VxSha1Hash					m_FileHashId;
    std::string					m_strRemoteFileName;
    std::string					m_strLocalFileName;
    std::string					m_LclFileNameAndPath;
    EXferDirection				m_XferDirection{eXferDirectionRx};
    int							m_PercentProgress{0};
	bool						m_IsStreaming{ false };
};


