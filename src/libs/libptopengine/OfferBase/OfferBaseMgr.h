#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferCallback.h"
#include "OfferBaseInfoDb.h"

#include <CoreLib/Sha1GeneratorCallback.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxSemaphore.h>
#include <CoreLib/VxThread.h>

class FileInfo;
class GuiUser;
class IToGui;
class OfferCallback;
class OfferBaseInfo;
class OfferBaseInfoDb;
class OfferBaseHistoryMgr;
class P2PEngine;
class PktFileListReply;
class QWidget;

class OfferBaseMgr : public Sha1GeneratorCallback
{
public:
	OfferBaseMgr() = delete;
	OfferBaseMgr( EOfferMgrType assetMgrType );
	virtual ~OfferBaseMgr() = default;

    class AutoResourceLock
    {
    public:
        AutoResourceLock( OfferBaseMgr * assetMgrBase ) : m_Mutex(assetMgrBase->getResourceMutex())	{ m_Mutex.lock(); }
        ~AutoResourceLock()																			{ m_Mutex.unlock(); }
        VxMutex&				m_Mutex;
    };

	void                        wantOfferCallbacks( OfferCallback* client, bool enable );

    virtual OfferBaseInfoDb&    getOfferInfoDb( void ) { return m_OfferBaseInfoDb; }

    virtual std::vector<OfferBaseInfo*>&	getOfferBaseInfoList( void )				{ return m_OfferBaseInfoList; }

    // startup when user specific directory has been set after user logs on
    virtual void				fromGuiUserLoggedOn( void );
	virtual void				onPluginsInitialized( void );

    virtual bool				fromGuiSetFileIsShared( std::string fileName, std::string fileNameAndPath, bool shareFile, uint8_t * fileHashId );

	virtual void				fromGuiMakePluginOffer( QWidget* parent, EPluginType pluginType, GuiUser* guiUser, FileInfo& fileInfo ) {};

	virtual	bool				hostedOfferExists( OfferBaseInfo& offerInfo, bool updateResponse = false );
	virtual	bool				clientOfferExists( OfferBaseInfo& offerInfo );

    virtual void                onQueryHistoryOffer( OfferBaseInfo* offerInfo ) {}; // should be overriden

    VxMutex&					getResourceMutex( void )					{ return m_ResourceMutex; }
    void						lockResources( void )						{ m_ResourceMutex.lock(); }
    void						unlockResources( void )						{ m_ResourceMutex.unlock(); }

    bool						isAllowedFileOrDir( std::string strFileName );

	virtual bool				isOfferListInitialized( void )				{ return m_OfferBaseListInitialized; }

	void						offerInfoMgrStartup( VxThread* startupThread );
	void						offerInfoMgrShutdown( void );

    bool						getFileHashId( std::string& fileFullName, VxSha1Hash& retFileHashId );
	bool						getFileFullName( VxSha1Hash& fileHashId, std::string& retFileFullName );


	OfferBaseInfo*				findOffer( std::string& fileName );
	OfferBaseInfo*				findOffer( VxSha1Hash& fileHashId );
	OfferBaseInfo*				findOffer( VxGUID& offerId );

	uint16_t					getOfferBaseFileTypes( void )				{ return m_u16OfferBaseFileTypes; }
	void						updateOfferFileTypes( void );

	void						lockFileListPackets( void )				{ m_FileListPacketsMutex.lock(); }
	void						unlockFileListPackets( void )			{ m_FileListPacketsMutex.unlock(); }
	std::vector<PktFileListReply*>&	getFileListPackets( void )			{ return m_FileListPackets; }
	void						updateFileListPackets( void );

    OfferBaseInfo* 				addOfferFile( const char* fileName, const char*	fileNameAndPath, uint64_t fileLen, uint16_t fileType );

	bool						addOfferFile(	const char*		fileName, 
												const char*		fileNameAndPath,
												VxGUID&			assetId,  
												uint8_t *		hashId = 0, 
												EOfferLocation	locationFlags = eOfferLocUnknown, 
												const char*		assetTag = "", 
												int64_t		    timestamp = 0 );

	bool						addOfferFile(	const char*		fileName, 
												const char*		fileNameAndPath,
												VxGUID&			assetId,  
												VxGUID&		    creatorId, 
												VxGUID&		    historyId, 
												uint8_t *		hashId = 0, 
												EOfferLocation	locationFlags = eOfferLocUnknown, 
												const char*		assetTag = "", 
                                                int64_t			timestamp = 0 );

	bool						addOffer( OfferBaseInfo& offerInfo );

    bool						updateOffer( OfferBaseInfo& offerInfo );
	bool						removeOffer( std::string fileNameAndPath );
	bool						removeOffer( VxGUID& assetOfferId );
	void						queryHistoryOffers( VxGUID& historyId );

	void						updateOfferXferState( VxGUID& assetOfferId, EOfferSendState assetSendState, int param = 0 );

	bool						deleteDatabase( void );

	virtual void				announceOfferAdded( OfferBaseInfo* offerInfo, bool resourceLocked = false );
    virtual void				announceOfferUpdated( OfferBaseInfo* offerInfo );
    virtual void				announceOfferRemoved( OfferBaseInfo* offerInfo, bool resourceLocked = false );
    virtual void				announceOfferXferState( VxGUID& assetOfferId, EOfferSendState assetSendState, int param );
	virtual void				announceOfferAction( VxGUID& assetOfferId, EOfferAction offerAction, int param );

protected:
    virtual OfferBaseInfo*      createOfferInfo( std::string fileName, std::string fileNameAndPath, uint64_t fileLen, uint16_t fileType );
    virtual OfferBaseInfo*      createOfferInfo( OfferBaseInfo& offerInfo );

   void							lockClientList( void )						{ m_ClientListMutex.lock(); }
   void							unlockClientList( void )					{ m_ClientListMutex.unlock(); }

	void						updateOfferListFromDb( VxThread* thread );

	void						clearOfferFileListPackets( void );
	void						clearOfferInfoList( void );
	OfferBaseInfo*				createOfferInfo(	std::string		fileName,
													std::string		fileNameAndPath,
													VxGUID&			assetId,  
													uint8_t *		hashId, 
													EOfferLocation	locationFlags = eOfferLocUnknown, 
													const char*		assetTag = "", 
													int64_t			timestamp = 0 );
	bool						insertNewInfo( OfferBaseInfo* offerInfo );
	void						updateDatabase( OfferBaseInfo* offerInfo );
	void						updateOfferDatabaseSendState( VxGUID& assetOfferId, EOfferSendState sendState );

	void                        requestFileHash( OfferBaseInfo* assetInfo );
    void                        callbackSha1GenerateResult( ESha1GenResult sha1GenResult, VxGUID& assetId, Sha1Info& sha1Info );

    //=== vars ===//
    EOfferMgrType               m_OfferMgrType{ eOfferMgrNotSet };
    VxMutex						m_ResourceMutex;
    VxMutex						m_ClientListMutex;

    std::vector<OfferCallback *> m_OfferClients;
	bool						m_Initialized{ false };

	std::vector<OfferBaseInfo*>	m_WaitingForHastList;

	VxThread					m_OfferMgrStartupThread;

    uint16_t					m_u16OfferBaseFileTypes{ 0 };
	VxMutex						m_FileListPacketsMutex;
	std::vector<PktFileListReply*> m_FileListPackets;

    bool						m_OfferBaseListInitialized{ false };
    OfferBaseInfoDb				m_OfferBaseInfoDb;
    std::vector<OfferBaseInfo*>	m_OfferBaseInfoList;
};

