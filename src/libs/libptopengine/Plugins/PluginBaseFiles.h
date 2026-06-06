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

#include "PluginBase.h"
#include "PluginSessionMgr.h"
#include "FileInfoBaseMgr.h"

#include <P2PEngine/FileShareSettings.h>

#include <PktLib/VxCommon.h>

class FileInfoBaseMgr;
class FileRxSession;
class FileTxSession;
class FileShareSettings;
class FileInfoBaseMgr;
class FileInfoXferMgr;
class IToGui;
class P2PEngine;
class PktFileListReply;
class FileInfo;
class VxNetIdent;
class VxFileShredder;

class PluginBaseFiles : public PluginBase
{
public:
	PluginBaseFiles( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, FileInfoBaseMgr& fileInfoBaseMgr );
	virtual ~PluginBaseFiles() = default;

	FileInfoBaseMgr&			getFileInfoMgr( void )			{ return m_FileInfoMgr; }

	void						wantFileXferCallback( FileXferCallback* callback, bool wantCallback );

	virtual void				onAfterUserLogOnThreaded( void ) override;

    virtual bool                fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    virtual void				fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    virtual bool				fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

    void						fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings ) override;
    void						fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings ) override;

    void						fromGuiCancelDownload( VxGUID& fileInstance ) override;
    void						fromGuiCancelUpload( VxGUID& fileInstance ) override;

	bool						fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;
	bool						fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

	EXferError					fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo ) override;

	virtual bool				fromGuiBrowseFiles( VxGUID& appInstId, std::string& dir, uint8_t fileFilterMask );

	virtual bool				fromGuiSetFileIsShared( FileInfo& fileInfo, bool isShared );
	virtual bool				fromGuiSetFileIsShared( std::string& fileNameAndPath, bool isShared );
	virtual bool				fromGuiGetFileIsShared( FileInfo& fileInfo );
	virtual bool				fromGuiGetIsFileShared( std::string& fileNameAndPath );

	// returns -1 if unknown else percent downloaded
	virtual int					fromGuiGetFileDownloadState( uint8_t * fileHashId );
	virtual bool				fromGuiQueryFileHash( FileInfo& fileInfo );
	virtual void				fromGuiFileHashGenerated( std::string& fileNameAndPath, int64_t fileLen, VxSha1Hash& fileHash );

    virtual void				onSharedFilesUpdated( uint16_t u16FileTypes ) override;

	virtual bool				isServingFiles( void );

	virtual void				deleteFile( std::string fileNameAndPath, bool shredFile );

	bool						ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb = false ) override;

protected:
	virtual void				sendFileSearchResultToGui( VxGUID& searchSessionId, VxGUID& onlineId, FileInfo& fileInfo );

	void						onPktPluginOfferReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	virtual void				onPktFileGetReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileChunkReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileGetCompleteReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileXferCancel         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	virtual void				onPktFindFileReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFindFileReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileListReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileListReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileShareErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	virtual void				onPktFileInfoInfoReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoInfoReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoAnnReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoAnnReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoSearchReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoSearchReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoMoreReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	virtual void				onPktFileInfoMoreReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	virtual bool				updateFromFileInfoSearchBlob( VxGUID& searchSessionId, VxGUID& hostOnlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, PktBlobEntry& blobEntry, int fileInfoCount );
	virtual bool				requestMoreFileInfoFromServer( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, VxGUID& nextSearchAssetId, std::string& searchText );

	virtual ECommErr			searchRequest( PktFileInfoSearchReply& pktReply, VxGUID& specificAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
	virtual ECommErr			searchMoreRequest( PktFileInfoMoreReply& pktReply, VxGUID& nextFileAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	// should be overwitten by Plugin specific class
	virtual bool                fileInfoSearchResult( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, FileInfo& fileInfo ) { return false; };
	virtual void                fileInfoSearchCompleted( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, ECommErr commErr ) {};

    bool						isFileShared( std::string& fileNameAndPath );
    virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override;
    virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override;
    virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) override;	

	void						onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override;

	//=== vars ===//
	PluginSessionMgr			m_PluginSessionMgr;
	FileInfoBaseMgr&			m_FileInfoMgr;
	FileInfoXferMgr&			m_FileInfoXferMgr;
};


