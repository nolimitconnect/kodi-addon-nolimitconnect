#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoDb.h"
#include "FileInfoXferMgr.h"

#include <PktLib/VxCommon.h>

#include <CoreLib/VxThread.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/Sha1GeneratorCallback.h>

class AssetBaseInfo;
class FileInfo;
class IToGui;
class PluginBase;
class P2PEngine;
class PktFileListReply;
class PktFileInfoSearchReply;
class PktFileInfoMoreReply;
class SearchParams;
class SharedFilesMgr;
class VxSha1Hash;
class VxFileShredder;

class FileInfoBaseMgr : public Sha1GeneratorCallback
{
public:
	FileInfoBaseMgr() = delete;
	FileInfoBaseMgr( const FileInfoBaseMgr& rhs ) = delete;
	FileInfoBaseMgr( P2PEngine& engine, PluginBase& plugin, FileInfoDb& fileInfoDb );
	virtual ~FileInfoBaseMgr();

	FileInfoBaseMgr& operator=( const FileInfoBaseMgr& rhs ) = delete;

	bool						getIsInitialized( void )			{ return m_FilesInitialized; }

	P2PEngine&					getEngine( void )					{ return m_PrivateEngine; }

	FileInfoXferMgr&			getFileInfoXferMgr( void )			{ return m_FileInfoXferMgr; }

	virtual void				fileInfoMgrShutdown( void );

	void						lockFileList( void )				{ m_FilesListMutex.lock(); }
	void						unlockFileList( void )				{ m_FilesListMutex.unlock(); }
	std::map<VxGUID, FileInfo>& getFileLibraryList( void )			{ return m_FileInfoList; }

	void						setRootFolder( std::string& rootFileFolder ) { m_RootFileFolder = rootFileFolder; }
	std::string					getRootFolder( void )				{ return m_RootFileFolder; }

	virtual std::string			getIncompleteFileXferDirectory( VxGUID& onlineId );

	bool						cancelAndDelete( VxGUID& assetId );

	// returns -1 if unknown else percent downloaded
	virtual int					fromGuiGetFileDownloadState(	uint8_t *		fileHashId );
	
	virtual bool				fromGuiBrowseFiles( VxGUID& appInstId, const char* dir, uint8_t fileFilterMask ) { return false; }

	virtual bool				fromGuiSetFileIsShared( FileInfo& fileInfo, bool isShared );
	virtual bool				fromGuiGetFileIsShared( FileInfo& fileInfo );
	virtual bool				fromGuiSetFileIsShared( std::string& fileNameAndPath, bool isShared );
	virtual bool				fromGuiGetFileIsShared( std::string& fileNameAndPath );
	void						fileShareEnable( AssetBaseInfo* assetInfo, bool shareFile );
	virtual bool				fromGuiAddSharedFile( FileInfo& fileInfo, bool isShared );

	virtual bool				fromGuiQueryFileHash( FileInfo& fileInfo );
	virtual void				fromGuiFileHashGenerated( std::string& fileNameAndPath, int64_t fileLen, VxSha1Hash& fileHash );

	virtual void				updateFileTypes( void );

	virtual bool				isFileShared( std::string& fileNameAndPath, bool listIsLocked = false);
	virtual bool				isFileShared( VxGUID& assetId, bool listIsLocked = false );
	virtual bool				isFileShared( VxSha1Hash& fileHashId, bool listIsLocked = false );
	virtual bool				isFileShared( VxGUID& assetId, VxSha1Hash& fileHashId, bool listIsLocked = false );

	virtual bool				addFileToDbAndList( FileInfo& fileInfoIn );
	virtual bool				addFileToDbAndList( VxGUID& onlineId, std::string& fileName, std::string& fileNameAndPath, VxGUID& assetId );
	virtual bool				addFileToDbAndList( VxGUID&			onlineId,
													VxGUID&			assetId,
													std::string		fileName,
													std::string		fileNameAndPath,
													uint64_t		fileLen,
													uint8_t			fileType,
													VxSha1Hash&		fileHashId );

	virtual bool				removeFromDbAndList( std::string& fileNameAndPath, bool listIsLocked = false );

	bool						isAllowedFileOrDir( std::string strFileName );

	bool						getFileFullName( VxGUID& assetId, VxSha1Hash& fileHashId, std::string& retFileFullName );
	bool						getFileHashId( std::string& fileFullName, VxSha1Hash& retFileHashId );

	virtual bool				onFileDownloadComplete( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& lclSessionId, std::string& fileName, VxGUID& assetId, VxSha1Hash& sha11Hash);

	bool						loadAboutMePageStaticAssets( void );
	bool						loadStoryboardPageFileAssets( void );

	virtual void				onAfterUserLogOnThreaded( void );

	ECommErr					searchRequest( PktFileInfoSearchReply& pktReply, VxGUID& specificAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
	ECommErr					searchMoreRequest( PktFileInfoMoreReply& pktReply, VxGUID& nextFileAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	bool						startDownload( FileInfo& fileInfo, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	virtual void				toGuiRxedPluginOffer( VxGUID onlineId, EPluginType pluginType, OfferBaseInfo& offerInfo, VxGUID& lclSessionId );
	virtual void				toGuiRxedOfferReply( VxGUID onlineId, EPluginType pluginType, OfferBaseInfo& offerInfo, VxGUID& lclSessionId, EOfferResponse offerResponse );

	virtual void				toGuiFileUploadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo );
	virtual void				toGuiFileDownloadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo );

	void						updateToGuiFileXferState( VxGUID& lclSessionId, EXferDirection xferDir, EXferState xferState, EXferError xferErr, int param = 0 );
	void						toGuiFileDownloadComplete( VxGUID& lclSessionId, std::string fileName, EXferError xferError );
	void						toGuiFileUploadComplete( VxGUID& lclSessionId, std::string fileName, EXferError xferError );

	virtual void				sendFileSearchResultToGui( VxGUID& searchSessionId, VxGUID& onlineId, FileInfo& fileInfo );

protected:
	void						checkForInitializeCompleted( void );
	bool						findFileAsset( VxGUID& fileAssetId, FileInfo& fileInfo ); // assumes file list is already locked

	void						addFileToGenHashQue( VxGUID& assetId, std::string fileName, std::string fileNameAndPath );
	void						removeFileFromGenHashQue( VxGUID& assetId, std::string fileName );
	void						callbackSha1GenerateResult( ESha1GenResult sha1GenResult, VxGUID& assetId, Sha1Info& sha1Info ) override;

	bool						isFileShareServer( void );
	bool						isLibraryServer( void );

    void                        generateHashIfNeeded( FileInfo& fileInfoIn );

	//=== vars ===//
	PluginBase&					m_Plugin;

	FileInfoDb&					m_FileInfoDb;
	FileInfoXferMgr				m_FileInfoXferMgr;

	int64_t						m_LastUpdateTime{ 0 };
	int64_t						m_s64TotalByteCnt{ 0 };
	uint16_t					m_u16FileTypes{ 0 };

	VxMutex						m_FilesListMutex;
	std::map<VxGUID,FileInfo>	m_FileInfoList; // map of assetId, FileInfo
	std::vector<FileInfo>		m_FileInfoNeedHashAndSaveList;


	std::string					m_RootFileFolder{ "" };
	bool						m_FilesInitialized{ false };

private:
	P2PEngine&					m_PrivateEngine; // to avoid abiguous access
};

