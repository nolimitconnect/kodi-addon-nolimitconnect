#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileXferCallback.h"

#include <GuiInterface/IDefs.h>

#include <P2PEngine/FileShareSettings.h>

#include <PktLib/VxCommon.h>

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxMutex.h>

#include <map>

class FileInfo;
class FileRxSession;
class FileShareXferSession;
class FileTxSession;
class FileInfoBaseMgr;
class FileLibraryMgr;
class OfferBaseInfo;
class P2PEngine;
class PluginBase;
class PluginMgr;
class FileInfo;
class VxFileXferInfo;
class VxPktHdr;
class VxSha1Hash;

class PktFileGetCompleteReq;
class PktFileSendReq;
class PktFileChunkReq;
class PktFileSendCompleteReq;
class PktFileListReply;

class FileInfoXferMgr
{
public:
	enum EFileXOptions
	{
		eFileXOptionReplaceIfExists		= 0,
		eFileXOptionResumeIfExists		= 1,
		eFileXOptionFailIfExists		= 2
	};

	typedef std::map<VxGUID, FileRxSession*>::iterator FileRxIter;
	typedef std::vector<FileTxSession*>::iterator FileTxIter;

	FileInfoXferMgr() = delete;
	FileInfoXferMgr(const FileInfoXferMgr& rhs ) = delete;
	FileInfoXferMgr( P2PEngine& engine, PluginBase& plugin, FileInfoBaseMgr& fileInfoMgr );
	virtual ~FileInfoXferMgr();

	FileInfoXferMgr& operator=( const FileInfoXferMgr& rhs ) = delete;

	EPluginType					getPluginType( void );

	void						wantFileXferCallback( FileXferCallback* callback, bool wantCallback );

	void						announcePkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	void						setMoveCompletedFilesToDownloadFolder( bool moveOnCompleted )	{ m_MoveOnCompletedToDownloadsFolder = moveOnCompleted; }
	bool						getMoveCompletedFilesToDownloadFolder( void )					{ return m_MoveOnCompletedToDownloadsFolder; }

	void						fileAboutToBeDeleted( std::string& fileName );

	virtual void				onAfterUserLogOnThreaded( void );

	virtual void				fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings );
	virtual void				fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings );

	virtual bool				fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual void				fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual bool				fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() );

	void						fromGuiCancelDownload( VxGUID& fileInstance );
	void						fromGuiCancelUpload( VxGUID& fileInstance );

	bool						fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo );
	bool						fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo );

	EXferError					fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo );
	// returns -1 if unknown else percent downloaded
	virtual int					fromGuiGetFileDownloadState( uint8_t * fileHashId );

	virtual bool				startDownload( FileInfo& fileInfo, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	virtual void				onConnectionLost			( std::shared_ptr<VxSktBase>& sktBase );

	virtual void				onPktPluginOfferReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktPluginOfferReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				onPktFileGetReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileChunkReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileGetCompleteReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileXferCancel         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				onPktFindFileReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFindFileReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileListReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileListReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileInfoReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileInfoReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktFileInfoErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				onPktStreamCtrlReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktStreamCtrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );

protected:
	virtual void				onFileReceived( FileRxSession* xferSession, std::string& fileName, EXferError error);
	virtual void				onFileSent( FileTxSession* xferSession, std::string& fileName, EXferError error );

	bool						isFileDownloading( VxSha1Hash& fileHashId );
	bool						isFileInDownloadFolder( const char* pPartialFileName );

	virtual FileRxSession*		findRxSessionSendToId( VxGUID& sendToId );
	virtual FileRxSession*		findRxSessionSessionId( VxGUID& lclSessionId );
	virtual FileRxSession*		findOrCreateRxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual FileRxSession*		findOrCreateRxSession( VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual FileTxSession*		findTxSessionSendToId( VxGUID& sendToId );
	virtual FileTxSession*		findTxSessionSessionId( VxGUID& lclSessionId );
	virtual FileTxSession*		createTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual FileTxSession*		findOrCreateTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual FileTxSession*		findOrCreateTxSession( VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );

	virtual EXferError			beginFileReceive( FileRxSession* xferSession, PktFileSendReq* poPkt );
	virtual EXferError			beginFileSend( FileTxSession* xferSession );

	virtual void				endFileXferSession( FileRxSession* xferSession );
	virtual void				endFileXferSession( FileTxSession* xferSession );

	virtual EXferError			rxFileChunk( FileRxSession* xferSession, PktFileChunkReq* poPkt );
	virtual EXferError			txNextFileChunk( FileTxSession* xferSession );

	virtual void				finishFileReceive( FileRxSession* xferSession, PktFileSendCompleteReq* poPkt );
	virtual void				finishFileReceive( FileRxSession* xferSession, PktFileGetCompleteReq* poPkt );

	virtual int32_t				sendFileShareError(		std::shared_ptr<VxSktBase>&		sktBase,		// socket
														int				iPktType,	// type of packet
														unsigned short	u16Cmd,		// packet command
														long			rc,			// error code
														const char*		pMsg, ...);	// error message

	EXferError					beginFileGet( FileRxSession* xferSession );
	EXferError					canTxFile( VxGUID sendToId, VxGUID& assetId, VxSha1Hash& fileHashId );
	bool						isViewFileListMatch( FileTxSession* xferSession, FileInfo& fileInfo );
	void						clearRxSessionsList( void );
	void						clearTxSessionsList( void );
	void						checkQueForMoreFilesToSend( void );

protected:
	EXferError					setupFileDownload( VxFileXferInfo& xferInfo, VxGUID& sendToId );
	bool						makeIncompleteFileName( std::string& strRemoteFileName, std::string& strRetIncompleteFileName, VxGUID& sendToId );
	EXferError					sendNextFileChunk( VxFileXferInfo& xxferInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& skt );

	void						lockClientList( void )		{ m_FileXferCallbackMutex.lock(); }
	void						unlockClientList( void )	{ m_FileXferCallbackMutex.unlock(); }

	void						sendFileXferCancel( FileShareXferSession* xferSession );
	void						sendFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, FileShareXferSession* xferSession );

	void 						addTxSession( FileTxSession* xferSession );

	//=== vars ====//
	P2PEngine&					m_Engine;
	PluginBase&					m_Plugin;
	PluginMgr&					m_PluginMgr;
	FileInfoBaseMgr&			m_FileInfoMgr;

	std::map<VxGUID, FileRxSession*>	m_RxSessions;
	std::vector<FileTxSession*>		m_TxSessions;

	bool						m_bIsInSession{ true };
	bool						m_bIsInitialized{ false };

	EFileXOptions				m_eFileRxOption{ eFileXOptionReplaceIfExists };
	FileShareSettings			m_FileShareSettings;
	bool						m_MoveOnCompletedToDownloadsFolder{ false };

	std::vector<FileXferCallback*>    m_FileXferCallbackClients;
    VxMutex						m_FileXferCallbackMutex;
};



