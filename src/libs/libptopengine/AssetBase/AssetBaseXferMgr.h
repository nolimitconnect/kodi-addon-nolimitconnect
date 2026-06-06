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

#include "AssetBaseXferDb.h"
#include "AssetBaseInfo.h"
#include "BaseXferInterface.h"

#include <GuiInterface/IDefs.h>

#include <PktLib/VxCommon.h>
#include <CoreLib/VxThread.h>
#include <CoreLib/VxGuidPairTimeList.h>

#include <map>

class AssetBaseRxSession;
class AssetBaseTxSession;
class AssetBaseMgr;
class AssetXferCallback;
class IToGui;
class P2PEngine;
class PluginMessenger;
class PluginMgr;
class PluginSessionMgr;
class VxPktHdr;
class VxSha1Hash;

class PktBaseGetReq;
class PktBaseGetReply;
class PktBaseSendReq;
class PktBaseSendReply;
class PktBaseChunkReq;
class PktBaseChunkReply;
class PktBaseGetCompleteReq;
class PktBaseGetCompleteReply;
class PktBaseSendCompleteReq;
class PktBaseSendCompleteReply;
class PktBaseXferErr;
class PktBaseListReq;
class PktBaseListReply;

class AssetBaseXferMgr
{
public:
	AssetBaseXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface );
	virtual ~AssetBaseXferMgr();

	EPluginType					getPluginType( void )							{ return m_XferInterface.getPluginType(); }

	VxMutex&					getAssetBaseQueMutex( void )					{ return m_AssetBaseSendQueMutex; }
	void						lockAssetBaseQue( void )						{ m_AssetBaseSendQueMutex.lock(); }
	void						unlockAssetBaseQue( void )						{ m_AssetBaseSendQueMutex.unlock(); }

	virtual void                wantAssetXferCallbacks( AssetXferCallback* client, bool enable );

	virtual void				fromGuiUserLoggedOn( void );

	virtual bool				fromGuiSendAssetBase( AssetBaseInfo& assetInfo );
    virtual bool				fromGuiRequestAssetBase( AssetBaseInfo& assetInfo, std::shared_ptr<VxSktBase>& sktBase, bool tmpAsset = false );
    virtual bool				fromGuiRequestAssetBase( VxNetIdent* netIdent, AssetBaseInfo& assetInfo, std::shared_ptr<VxSktBase>& sktBase, bool tmpAsset = false );
	virtual void				fromGuiCancelDownload( VxGUID& lclSessionId );
	virtual void				fromGuiCancelUpload( VxGUID& lclSessionId );

	virtual void				sendToGuiAssetAction( EAssetAction assetAction, VxGUID& assetId, int pos0to100000 );

	void						fileAboutToBeDeleted( std::string& fileName );
	virtual void				onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );
	virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );

    virtual void				onPktAssetBaseGetReq			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetBaseGetReply			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseSendReq			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseSendReply			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseChunkReq			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseChunkReply		    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetBaseGetCompleteReq	    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetBaseGetCompleteReply	    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseSendCompleteReq	    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseSendCompleteReply	    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetBaseXferErr			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	void						assetXferThreadWork( VxThread* workThread );
protected:
	virtual void				onAssetBaseReceived( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, EXferError error, bool pluginIsLocked );
	virtual void				onAssetBaseSent( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, AssetBaseInfo& assetInfo, EXferError error, bool pluginIsLocked );
    virtual void				onRequestAssetFailed( VxGUID sendToId, AssetBaseInfo& assetInfo, VxGUID& sktConnectId, bool pluginIsLocked );
	virtual void				onTxFailed( VxGUID& sendToId, VxGUID& assetUniqueId, bool pluginIsLocked );
	virtual void				onTxSuccess( VxGUID& sendToId, VxGUID& assetUniqueId, bool pluginIsLocked );
    virtual void                rebroadcastReceivedChatRoomAsset( AssetBaseInfo& assetInfo, const VxGUID& srcOnlineId, bool pluginIsLocked );

	virtual AssetBaseRxSession*		findRxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId );
    virtual AssetBaseRxSession*		findRxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId, bool logIfNotFound = true );
	virtual AssetBaseRxSession*		findOrCreateRxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual AssetBaseRxSession*		findOrCreateRxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );

	virtual AssetBaseTxSession*		findTxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId );
	virtual AssetBaseTxSession*		findTxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId );
	virtual AssetBaseTxSession*		createTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual AssetBaseTxSession*		findOrCreateTxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
	virtual AssetBaseTxSession*		findOrCreateTxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );

	virtual EXferError			        beginAssetBaseReceive( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, PktBaseSendReq* poPkt, PktBaseSendReply& pktReply );
    virtual EXferError			        beginAssetBaseReceive( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, VxGUID& lclSessionId, int64_t startOffset = 0 );

	virtual EXferError			        beginAssetBaseSend( AssetBaseTxSession* xferSession );

	virtual void				        endAssetBaseXferSession( AssetBaseRxSession* xferSession, bool pluginIsLocked );
	virtual void				        endAssetBaseXferSession( AssetBaseTxSession* xferSession, bool pluginIsLocked, bool requeAsset );

	virtual EXferError			        rxAssetBaseChunk( bool pluginIsLocked, AssetBaseRxSession* xferSession, PktBaseChunkReq* poPkt );
	virtual EXferError			        txNextAssetBaseChunk( AssetBaseTxSession* xferSession, uint32_t remoteErr, bool pluginIsLocked );

	virtual void				        finishAssetBaseReceive( AssetBaseRxSession* xferSession, PktBaseSendCompleteReq* poPkt, bool pluginIsLocked );

	void						        clearRxSessionsList( void );
	void						        clearTxSessionsList( void );
	bool						        checkQueForMoreAssetsToSend( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );

	void						        assetSendComplete( AssetBaseTxSession* xferSession );
	void						        queAsset( AssetBaseInfo& assetInfo );
	EXferError					        createAssetTxSessionAndSend( bool pluginIsLocked, AssetBaseInfo& assetInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase );
    EXferError                          createAssetRxSessionAndReceive( bool pluginIsLocked, AssetBaseInfo& assetInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, bool tmpAsset = false );

	bool						        requireFileXfer( EAssetType assetType );

    virtual void                        onAssetBaseBeginUpload( VxGUID sendToId, AssetBaseInfo& assetInfo ) {};
    virtual void                        onAssetBaseUploadError( VxGUID sendToId, AssetBaseInfo& assetInfo, EXferError xferErr ) {};

    virtual PktBaseGetReq*			    createPktBaseGetReq( void );
    virtual PktBaseGetReply*			createPktBaseGetReply( void );
    virtual PktBaseSendReq*			    createPktBaseSendReq( void );
    virtual PktBaseSendReply*			createPktBaseSendReply( void );
    virtual PktBaseChunkReq*			createPktBaseChunkReq( void );
    virtual PktBaseChunkReply*			createPktBaseChunkReply( void );
    virtual PktBaseGetCompleteReq*		createPktBaseGetCompleteReq( void );
    virtual PktBaseGetCompleteReply*	createPktBaseGetCompleteReply( void );
    virtual PktBaseSendCompleteReq*		createPktBaseSendCompleteReq( void );
    virtual PktBaseSendCompleteReply*	createPktBaseSendCompleteReply( void );
    virtual PktBaseXferErr*			    createPktBaseXferErr( void );

    virtual PktBaseListReq*			    createPktBaseListReq( void );
    virtual PktBaseListReply*			createPktBaseListReply( void );

	virtual bool						isAssetRequested( VxGUID& assetId, VxGUID& sktConnectId );
	virtual void						assetXferComplete( VxGUID& assetId, VxGUID& sktConnectId );

	void								lockClientList( void )              { m_AssetXferClientsMutex.lock(); }
    void								unlockClientList( void )            { m_AssetXferClientsMutex.unlock(); }

	void								announceXferReadyToSend( VxGUID& sendToId, std::shared_ptr<VxSktBase>& sktBase );
	void								announceXferState( VxGUID& sendToId, VxGUID& assetId, enum EAssetSendState sendState, int param );

	virtual void						addAssetXferInfoIfDoesNotExist( AssetBaseInfo& assetInfo );
	virtual void						updateAssetMgrSendState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState sendState, int param );

	void								addTxSession( AssetBaseTxSession* xferSession );

	//=== vars ===//
	bool						        m_Initialized;
	std::map<VxGUID, AssetBaseRxSession*>	m_RxSessions;
	std::vector<AssetBaseTxSession*>		m_TxSessions;
	VxMutex						        m_TxSessionsMutex;

    P2PEngine&					        m_Engine;	
    AssetBaseMgr&				        m_AssetBaseMgr;
    BaseXferInterface&                  m_XferInterface;
    PluginMgr&					        m_PluginMgr;
	
	AssetBaseXferDb				        m_AssetBaseXferDb;

	VxMutex						        m_AssetBaseSendQueMutex;
	std::vector<AssetBaseInfo>	        m_AssetBaseSendQue;
	VxThread					        m_WorkerThread;
    std::string                         m_WorkerThreadName;

	VxMutex						        m_AssetRequestedListMutex;
	VxGuidPairTimeList					m_AssetRequestedList;

	VxMutex								m_AssetXferClientsMutex;
    std::vector<AssetXferCallback*>		m_AssetXferClients;
};



