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

#include <GuiInterface/IDefs.h>

#include <PluginSettings/PluginSetting.h>
#include <AssetBase/BaseXferInterface.h>
#include <AssetMgr/AssetMgr.h>
#include <ThumbMgr/ThumbMgr.h>
#include <ThumbMgr/ThumbXferMgr.h>
#include <ThumbMgr/ThumbXferInterface.h>

#include <CoreLib/VxMutex.h>
#include <CoreLib/MediaCallbackInterface.h>

#include <PktLib/PktPluginHandlerBase.h>
#include <PktLib/VxCommon.h>

class AssetMgr;
class AssetBaseInfo;
class ConnectId;
class FileInfo;
class FileShareSettings;
class GroupieInfo;
class GroupieId;
class HostedInfo;
class IToGui;
class NetServiceHdr;
class OfferBaseInfo;
class P2PEngine;
class P2PSession;
class PktAnnounce;
class PktHostInviteAnnounceReq;
class PktPluginOfferReply;
class PktPluginOfferReq;
class PluginMgr;
class PluginSessionBase;
class PluginSetting;
class RxSession;
class SearchParams;
class ThumbMgr;
class TxSession;
class VxSktBase;

class PluginBase : public PktPluginHandlerBase, public MediaCallbackInterface, public BaseXferInterface
{
public:
	class AutoPluginLock
	{
	public:
		AutoPluginLock( PluginBase* pluginBase ) 
		: m_Mutex(pluginBase->getPluginMutex())	
		{ 
			m_Mutex.lock(); 
		}
		~AutoPluginLock()
		{ 
			m_Mutex.unlock(); 
		}

		VxMutex&						m_Mutex;
	};

	PluginBase(	P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* netIdent, EPluginType pluginType );
	virtual ~PluginBase() override = default;

	virtual	EMediaModule		getMediaModule( void ) = 0; // for subscriptions to audio and video capture/play

	virtual void				lockPlugin( void )										{ m_PluginMutex.lock(); }
	virtual void				unlockPlugin( void )									{ m_PluginMutex.unlock(); }

    virtual void				pluginStartup( void );
    virtual void				pluginShutdown( void )									{}
	virtual bool				isPluginEnabled( void );
	virtual EFriendState		getPluginPermission( void );
	virtual void				setPluginPermission( EFriendState eFriendState );

	virtual void				onMyOnlineUrlIsValid( bool isValidUrl )					{};

	virtual bool				isAccessAllowed( VxNetIdent* netIdent, bool logAccessError = true, const char* accessReason = nullptr );

	virtual void				setIsPluginInSession( bool inSession )					{ m_bPluginIsInSession = inSession; }
	virtual bool				getIsPluginInSession( void )							{ return m_bPluginIsInSession; }

	virtual void				setIsServerInSession( bool inSession )					{ m_ServerIsInSession = inSession; }
	virtual bool				getIsServerInSession( void )							{ return m_ServerIsInSession; }

	const char*					describePlugin( void );

	bool						isVoicePlugin( void );

	virtual bool				addVoicePairTx( EPluginType pluginType, VxGUID & onlineId );
	virtual bool				removeVoicePairTx( EPluginType pluginType, VxGUID & onlineId );
	virtual bool				userNeedsVoicePairTx( EPluginType pluginType, VxGUID& onlineId );
	virtual int					needVoiceTxCount( EPluginType pluginType ); 
	virtual bool				getVoiceTxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList );
	virtual bool				isFirstVoicePairTx( EPluginType pluginType, VxGUID & onlineId );
	virtual void				updateRequestMicrophone( EPluginType pluginType, int prevNeedCnt, int needCnt );

	virtual bool				addVoicePairRx( EPluginType pluginType, VxGUID & onlineId );
	virtual bool				removeVoicePairRx( EPluginType pluginType, VxGUID & onlineId );
	virtual bool				userNeedsVoicePairRx( EPluginType pluginType, VxGUID& onlineId );
	virtual int					needVoiceRxCount( EPluginType pluginType );
	virtual bool				getVoiceRxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList );
	virtual bool				isFirstVoicePairRx( EPluginType pluginType, VxGUID & onlineId );
	virtual void				updateRequestMixer( EPluginType pluginType, int prevNeedCnt, int needCnt );

	virtual bool				addVideoPairTx( EPluginType pluginType, VxGUID& onlineId );
	virtual bool				removeVideoPairTx( EPluginType pluginType, VxGUID& onlineId );
	virtual bool				userNeedsVideoPairTx( EPluginType pluginType, VxGUID& onlineId );
	virtual int					needVideoTxCount( EPluginType pluginType );
	virtual bool				getVideoTxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList );
	virtual bool				isFirstVideoPairTx( EPluginType pluginType, VxGUID& onlineId );
	virtual void				updateRequestVideoCapture( EPluginType pluginType, int prevNeedCnt, int needCnt );

	virtual bool				addVideoPairRx( EPluginType pluginType, VxGUID& onlineId );
	virtual bool				removeVideoPairRx( EPluginType pluginType, VxGUID& onlineId );
	virtual bool				userNeedsVideoPairRx( EPluginType pluginType, VxGUID& onlineId );
	virtual int					needVideoRxCount( EPluginType pluginType );
	virtual bool				getVideoRxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList );
	virtual bool				isFirstVideoPairRx( EPluginType pluginType, VxGUID& onlineId );
	virtual void				updateRequestVideoPlay( EPluginType pluginType, int prevNeedCnt, int needCnt );

	//=== getter/setters ===//
	virtual P2PEngine&			getEngine( void )										{ return m_Engine; }
    virtual IToGui&			    getToGui( void );
    virtual void				setPluginType( EPluginType pluginType );
    virtual EPluginType			getPluginType( void ) override							{ return m_ePluginType; }
	virtual EPluginType			getAssetOverridePluginType( void ) override;

    virtual bool                setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs = 0 );
    virtual PluginSetting&      getPluginSetting( void )                                { return m_PluginSetting; }

    virtual EHostType			getHostType( void );

	virtual EPluginType			getClientPluginType( void );
	virtual EPluginType			getServerPluginType( void );

	virtual PluginMgr&			getPluginMgr( void )									{ return m_PluginMgr;	}
	virtual	VxMutex&			getPluginMutex( void )									{ return m_PluginMutex; }	

    virtual	VxMutex&			getAssetXferMutex( void ) override						{ return m_PluginMutex; }					

	virtual	EAppState			getPluginState( void );
	virtual	void				setPluginState( EAppState ePluginState )				{ m_ePluginState = ePluginState;};

	EPluginAccess				getPluginAccessState( VxNetIdent* netIdent );

    virtual ThumbMgr&			getThumbMgr( void )									    { return m_ThumbMgr; }
    virtual ThumbXferMgr&       getThumbXferMgr( void )							        { return m_ThumbXferMgr; }

	virtual EMembershipState	getMembershipState( VxNetIdent* netIdent )				{ return eMembershipStateJoinDenied; }

	virtual bool				getHostedInfo( HostedInfo& hostedInfo )					{ return false; }
	virtual bool				getGroupieInfo( GroupieInfo& hostedInfo )				{ return false; }

	virtual void				fromGuiUserLoggedOn( void )								{};

	virtual bool                fromGuiStartPluginSession( PluginSessionBase* poOffer )	{ return true; };
    virtual bool                fromGuiStartPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() )	{ return true; };
    virtual void				fromGuiStopPluginSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() )	{};
    virtual bool				fromGuiIsPluginInSession( VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() )	{ return true; }

	virtual void				fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings );
	virtual void				fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings );
	virtual int					fromGuiDeleteFile( std::string& fileName, bool shredFile )	{ return 0; }

	virtual void				fromGuiCancelDownload( VxGUID& fileInstance ) {};
	virtual void				fromGuiCancelUpload( VxGUID& fileInstance ) {};

	virtual bool				fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )	{ return false; };
	virtual bool				fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )			{ return false; };

	virtual EXferError			fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo );

	virtual bool				fromGuiInstMsg( VxGUID& onlineId, const char*	pMsg );
	virtual bool				fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk );

	virtual bool				fromGuiTodGameActionSend( VxGUID& onlineId, ETodGameAction todGameAction )				{ return false; };

	virtual void				fromGuiRelayPermissionCount( int userPermittedCount, int anonymousCount )				{};

	virtual void				fromGuiUpdatePluginPermission( EPluginType pluginType, EFriendState pluginPermission )	{};

	virtual void				onAppStartup( void );
	virtual void				onAppShutdown( void );

	virtual void				onSpeexData( uint16_t * pu16SpeexData, uint16_t u16SpeexDataLen )							{};
	virtual void				fromGuiVideoData( uint8_t * pu8VidData, uint32_t u32VidDataLen, int iRotation )				{};
    virtual bool				fromGuiSendAsset( AssetBaseInfo& assetInfo )												{ return false; };
	virtual bool				fromGuiMultiSessionAction( VxGUID& onlineId, EMSessionAction mSessionAction, int pos0to100000, VxGUID lclSessionId = VxGUID::nullVxGUID() ) { return false; }; 

    //=== hosting ===//
    virtual void				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )	        {};
    virtual void				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )	            {};
	virtual void				fromGuiLeaveHost( HostedId& adminId )														{};
	virtual void				fromGuiUnJoinHost( HostedId& adminId )														{};
    virtual void				fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )            {};

    virtual void                updateHostSearchList( EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )  {};
	virtual void				fromGuiSendAnnouncedList( EHostType hostType, VxGUID& sessionId ) {};

	virtual bool				fromGuiRequestPluginThumb( VxNetIdent* netIdent, VxGUID& thumbId ) { return false; }
	virtual void				fromGuiListAction( EListAction listAction ) {};

	virtual bool				fromGuiDownloadWebPage( EWebPageType webPageType, VxGUID& onlineId ) { return false; }
	virtual bool				fromGuiCancelWebPage( EWebPageType webPageType, VxGUID& onlineId ) { return false; }

	virtual bool				fromGuiDownloadFileList( VxGUID& onlineId, VxGUID& sessionId, uint8_t fileTypes = 0 ) { return false; }
	virtual bool				fromGuiDownloadFileListCancel( VxGUID& onlineId, VxGUID& sessionId ) { return false; }

	virtual void				fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing ) {};

	virtual void				toGuiRxedPluginOffer( VxGUID onlineId, EPluginType pluginType, OfferBaseInfo& offerInfo, VxGUID& lclSessionId ) {};
	virtual void				toGuiRxedOfferReply( VxGUID onlineId, EPluginType pluginType, OfferBaseInfo& offerInfo, VxGUID& lclSessionId, EOfferResponse offerResponse ) {};

	virtual void				toGuiFileUploadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo );
	virtual void				toGuiFileDownloadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo );

	virtual void				toGuiFileXferState( VxGUID& localSessionId, EXferDirection xferDir, EXferState xferState, EXferError xferErr, int param = 0 );
	virtual void				toGuiFileDownloadComplete( VxGUID& lclSessionId, std::string& fileName, EXferError xferError );
	virtual void				toGuiFileUploadComplete( VxGUID& lclSessionId, std::string& fileName, EXferError xferError );

    //=== connections ===//
	virtual void				onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )	{};
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) = 0;
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) = 0;
	virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) = 0;

	virtual void				onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) = 0;

    bool						txPacket( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* poPkt, EPluginType overridePlugin = ePluginTypeInvalid ) override;

    //=== maintenence ===//
	virtual void				onSharedFilesUpdated( uint16_t u16FileTypes )									{};
    virtual void				onMyPktAnnounceChange( PktAnnounce& pktAnn )									{};
    virtual void				onThreadOncePer15Minutes( void )												{};
	virtual void				onAfterUserLogOnThreaded( void )												{};
    virtual	void				onPluginSettingChange( PluginSetting& pluginSetting, int64_t modifiedTimeMs )   {};

    //=== packet handlers ===//
    virtual void				onPktUserConnect			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktUserDisconnect			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

    virtual void				onPktPluginOfferReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktPluginOfferReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void				onPktSessionStartReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktSessionStartReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktSessionStopReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktSessionStopReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void				onPktMyPicSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktMyPicSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerPicChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerPicChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerGetChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerGetChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerPutChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktWebServerPutChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void				onPktThumbGetReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbGetReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbGetCompleteReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbGetCompleteReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktThumbXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    // error handling for invalid packet
    virtual void				onInvalidRxedPacket( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, const char* msg = "" );

    //=== sessions ===//
    virtual void				onSessionStart( PluginSessionBase* poSession, bool pluginIsLocked );
    virtual void				onSessionEnded( PluginSessionBase* poSession, 
                                                bool pluginIsLocked,
                                                EOfferResponse offerResponse = eOfferResponseUserOffline ) {};

    virtual EPluginAccess	    canAcceptNewSession( VxNetIdent* netIdent );

    virtual P2PSession *		createP2PSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
    virtual P2PSession *		createP2PSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
    virtual RxSession *			createRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
    virtual RxSession *			createRxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
    virtual TxSession *			createTxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );
    virtual TxSession *			createTxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

    //=== http ===//
	virtual void				handlePluginSpecificSkt( std::shared_ptr<VxSktBase>& sktBase ) {};
	virtual int32_t				handlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr )		{ return -1; }
	virtual int32_t				handlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )				{ return -1; }
	// default handlers
	virtual void				handleToGuiOfferRequest( VxNetIdent* netIdent, PktPluginOfferReq* pktReq );
	virtual void				handleToGuiOfferResponse( VxNetIdent* netIdent, PktPluginOfferReply* pktReply );

    virtual EPluginType         getDestinationPluginOverride( EHostType hostType );

	virtual bool				ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb = false ) { return false; }

	virtual void				onLoadedFilesReady( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) {};
	virtual void				onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) {};

	virtual std::string			getIncompleteFileXferDirectory( VxGUID& onlineId ) { return ""; }

    virtual void				onFileDownloadStart( bool started, VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& lclSessionId, std::string fileName, VxGUID& assetId ) {};
	virtual bool				onFileDownloadComplete( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& lclSessionId, std::string& fileName, VxGUID& assetId, VxSha1Hash& sha11Hash ) { return true; }

	virtual void                onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) {};
	virtual void                onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) {};
	virtual void                onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) {};
	virtual void                onGroupDirectUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) {};
	virtual void                onGroupRelayedUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) {};

	virtual	void				onNetworkConnectionReady( bool requiresRelay ) {};

	virtual	void				broadcastToClients( VxPktHdr* pktHdr, VxGUID& requesterOnlineId, std::shared_ptr<VxSktBase>& sktBaseRequester, bool includeRequester = true ) {};
	virtual	void				broadcastToClients( VxPktHdr* pktHdr, VxGUID& excludedOnlineId ) {};

	virtual	bool				isMyAccessAllowedFromHim( VxGUID& onlineId, EPluginType pluginType );
	virtual	bool				isHisAccessAllowedFromMe( VxGUID& onlineId, EPluginType pluginType );

	virtual void				sendSessionStop( std::shared_ptr<VxSktBase>& sktBase, PluginSessionBase* sessionBase );

protected:
	virtual void				makeShortFileName( const char* pFullFileName, std::string& strShortFileName );

    static std::string          getThumbXferDbName( EPluginType pluginType );
    static std::string          getThumbXferThreadName( EPluginType pluginType );
	void						logCommError( ECommErr commErr, const char* desc, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
	ECommErr					getCommAccessState( VxNetIdent* netIdent );

	virtual	void				handlePktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual	void				handlePktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual	void				updateFromHostInviteSearchBlob( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, PktBlobEntry& blobEntry, int hostInfoCount );
	virtual	bool				requestMoreHostInvitesFromNetworkHost( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& nextHostOnlineId );

	virtual bool                assureIdentityExist( HostedInfo& hostedInfo );

	//=== vars ===//
	EPluginType					m_ePluginType{ ePluginTypeInvalid };
	P2PEngine&					m_Engine;
	PluginMgr&					m_PluginMgr;
	VxNetIdent*					m_MyIdent = nullptr;
	EAppState					m_ePluginState{ eAppStateInvalid };
	
	VxMutex						m_PluginMutex;

    AssetMgr&                   m_AssetMgr;
    ThumbMgr&                   m_ThumbMgr;
	ThumbXferInterface			m_ThumbXferInterface;
    ThumbXferMgr                m_ThumbXferMgr; 
	
    PluginSetting               m_PluginSetting;

	bool						m_bPluginIsInSession = false;
	bool						m_ServerIsInSession = false;

	static VxMutex				m_VoicePairTxMutex;
	static std::vector<std::pair<EPluginType, VxGUID>>	m_VoiceTxList;
	static VxMutex				m_VoicePairRxMutex;
	static std::vector<std::pair<EPluginType, VxGUID>>	m_VoiceRxList;

	static VxMutex				m_VideoPairTxMutex;
	static std::vector<std::pair<EPluginType, VxGUID>>	m_VideoTxList;
	static VxMutex				m_VideoPairRxMutex;
	static std::vector<std::pair<EPluginType, VxGUID>>	m_VideoRxList;

	VxGUID						m_MediaSessionId;
};
