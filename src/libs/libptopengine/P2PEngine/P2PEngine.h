#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "config_appcorelibs.h"

#include "P2PConnectList.h"
#include "EngineSettings.h"
#include "EngineParams.h"

#include <AssetMgr/AssetCallbackInterface.h>
#include <BlobXferMgr/BlobCallbackInterface.h>
#include <Connections/ConnectionMgr.h>
#include <ConnectMgr/ConnectMgr.h>
#include <FromGuiMgr/FromGuiMgr.h>
#include <GroupieListMgr/GroupieListMgr.h>
#include <HostListMgr/HostedListMgr.h>
#include <HostListMgr/HostUrlListMgr.h>

#include <ConnectIdListMgr/ConnectIdListMgr.h>
#include <IdentListMgrs/FriendListMgr.h>
#include <IdentListMgrs/IgnoreListMgr.h>

#include <NetworkMonitor/NetStatusAccum.h>

#include <PluginSettings/PluginSettingMgr.h>
#include <ThumbMgr/ThumbCallbackInterface.h>
#include <ThumbMgr/ThumbMgr.h>
#include <WebPageMgr/WebPageMgr.h>

#include <GuiInterface/IDefs.h>
#include <GuiInterface/IFromGui.h>
#include <GuiInterface/IAudioInterface.h>

#include <Relay/RelayMgr.h>
#include <Search/RcScan.h>

#include <BigListLib/BigListMgr.h>
#include <Plugins/PluginLibraryServer.h>

#include <NetLib/VxSktDefs.h>
#include <PktLib/PktAnnounce.h>
#include <PktLib/PktSysHandlerBase.h>
#include <PktLib/PktsRandConnectDefs.h>
#include <PktLib/PktsImAlive.h>

#include <memory.h>

class AssetMgr;
class BlobMgr;
class ConnectMgr;
class ConnectRequest;
class FileMgr;
class FileShareSettings;
class FriendRequestMgr;
class HostServerJoinMgr;
class IToGui;
class INlc;
class IsPortOpenTest;
class OfferMgr;
class PluginMgr;
class PushToTalkMgr;
class RcConnectInfo;
class MediaProcessor;
class MemberActiveMgr;
class NetworkMgr;

class StayConnected;
class NetworkMonitor;
class NetServicesMgr;
class PluginNetServices;
class PluginFileShareServer;
class PluginSetting;
class PluginSettingMgr;
class RandConnectMgr;
class RunUrlAction;
class SendQueueMgr;
class UrlMgr;
class UserJoinMgr;
class UserOnlineMgr;
class VxPeerMgr;

class P2PEngine final:	public IFromGui,
					    public PktHandlerBase,
                        public AssetCallbackInterface,
                        public BlobCallbackInterface,
					    public MediaCallbackInterface,
                        public IAudioCallbacks
{
public:
    P2PEngine() = delete; // don't allow default constructor
    P2PEngine( const P2PEngine& ) = delete; // don't allow copy constructor

    P2PEngine( VxPeerMgr& peerMgr,
        MemberActiveMgr& memberActiveMgr,
        OfferMgr& offerMgr,
        PushToTalkMgr& pushToTalkMgr,
        RandConnectMgr& randConnectMgr,
        SendQueueMgr& sendQueueMgr,
        FriendRequestMgr& friendRequstMgr,
        FileMgr& fileMgr );
	virtual ~P2PEngine() override;

	void						startupEngine( void );
	void						shutdownEngine( void );
    bool                        isEngineCreated( void )                         { return m_IsEngineCreated; }
    bool                        isEngineReady( void )                           { return m_IsEngineReady; }

    IToGui&						getToGui( void );
	IFromGui&					getFromGuiInterface( void )						{ return *this; }
    IAudioRequests&			    getAudioRequest( void );

    AssetMgr&					getAssetMgr( void )								{ return m_AssetMgr; }
    BigListMgr&					getBigListMgr( void )							{ return m_BigListMgr; }
    ConnectionMgr&              getConnectionMgr( void )                        { return m_ConnectionMgr; }
    ConnectMgr&                 getConnectMgr( void )                           { return m_ConnectMgr; }
    BlobMgr&				    getBlobMgr( void )							    { return m_BlobMgr; }
    EngineSettings&				getEngineSettings( void )						{ return m_EngineSettings; }
	EngineParams&				getEngineParams( void )							{ return m_EngineParams; }
    ConnectIdListMgr&           getConnectIdListMgr( void )                     { return m_ConnectIdListMgr; }
    FriendListMgr&              getFriendListMgr( void )                        { return m_FriendListMgr; }
    FriendRequestMgr&           getFriendRequestMgr( void )                     { return m_FriendRequestMgr; }
    FromGuiMgr&                 getFromGuiMgr( void )                           { return m_FromGuiMgr; }
    GroupieListMgr&             getGroupieListMgr( void )                       { return m_GroupieListMgr; }

    HostServerJoinMgr&          getHostJoinMgr( void )                          { return m_HostJoinMgr; }
    HostUrlListMgr&             getHostUrlListMgr( void )                       { return m_HostUrlListMgr; }
    HostedListMgr&              getHostedListMgr( void )                        { return m_HostedListMgr; }

    IgnoreListMgr&              getIgnoreListMgr( void )                        { return m_IgnoreListMgr; }
    MemberActiveMgr&            getMemberActiveMgr( void )                      { return m_MemberActiveMgr; }

    StayConnected&				getStayConnected( void )						{ return m_StayConnected; }
    NetStatusAccum&             getNetStatusAccum( void )                       { return m_NetStatusAccum; }
    NetworkMgr&					getNetworkMgr( void )							{ return m_NetworkMgr; }
	NetworkMonitor&				getNetworkMonitor( void )						{ return m_NetworkMonitor; } 
	NetServicesMgr&				getNetServicesMgr( void )						{ return m_NetServicesMgr; }
	MediaProcessor&				getMediaProcessor( void )						{ return m_MediaProcessor; }

    OfferMgr&                   getOfferMgr( void )                             { return m_OfferMgr; }
    PushToTalkMgr&              getPushToTalkMgr( void )                        { return m_PushToTalkMgr; }
    P2PConnectList&             getConnectList( void )                          { return m_ConnectionList; }
    PluginMgr&					getPluginMgr( void )							{ return m_PluginMgr; }
    PluginSettingMgr&			getPluginSettingMgr( void )						{ return m_PluginSettingMgr; }
    RandConnectMgr&             getRandConnectMgr( void )                       { return m_RandConnectMgr; }
    RelayMgr&                   getRelayMgr( void )                             { return m_RelayMgr; }
    RcScan&						getRcScan( void )								{ return m_RcScan; }
    RunUrlAction&               getRunUrlAction( void )                         { return m_RunUrlAction; }

    SendQueueMgr&               getSendQueueMgr( void )                         { return m_SendQueueMgr; }
    ThumbMgr&                   getThumbMgr( void )                             { return m_ThumbMgr; }
    UrlMgr&                     getUrlMgr( void );

    UserJoinMgr&                getUserJoinMgr( void )                          { return m_UserJoinMgr; }

    VxPeerMgr&					getPeerMgr( void )								{ return m_PeerMgr; }
    WebPageMgr&                 getWebPageMgr( void )                           { return m_WebPageMgr; }

    std::shared_ptr<VxSktBase>& getSktLoopback( void )                          { return m_SktLoopback; }

    bool						isInternetAvailable( void );        // is internet available
    bool						isDirectConnectTested( void );      // has direct connect test completed
	bool						isNetworkOnline( void );
    bool                        isDirectConnectReady( void );       // true if have open port and ready to recieve
    bool                        isNetworkHostEnabled( void );       // true if netowrk host plugin is enabled

    bool                        getIsMyHostServiceEnabled( enum EHostType hostService );

    bool                        getHasAnyHostServiceEnabled( void );
 
    bool                        getHasAnyAnnonymousHostService( void );

    bool                        getHasFixedIpAddress( void );

    void						lockAnnouncePktAccess( void )					{ m_AnnouncePktMutex.lock(); }
    void						unlockAnnouncePktAccess( void )					{ m_AnnouncePktMutex.unlock(); }

    void						copyMyPktAnnounce( PktAnnounce& pktAnn )		{ m_AnnouncePktMutex.lock(); memcpy(&pktAnn, &m_PktAnn, sizeof(PktAnnounce)); m_AnnouncePktMutex.unlock(); }
	PktAnnounce&				getMyPktAnnounce( void )						{ return m_PktAnn; }

    void                        setPktAnnLastModTime( int64_t timeMs )          { m_PktAnnLastModTime = timeMs; }
    int64_t                     getPktAnnLastModTime( void )                    { return m_PktAnnLastModTime; }

    VxGUID&                     getMyOnlineId( void );
    std::string					getMyOnlineUrl( enum EHostType hostType = eHostTypeUnknown ) { m_AnnouncePktMutex.lock(); std::string myUrl( m_PktAnn.getMyOnlineUrl( hostType ) ); m_AnnouncePktMutex.unlock(); return myUrl; }
    VxNetIdent*				    getMyNetIdent( void )						    { return &m_PktAnn; }
    bool						addMyIdentToBlob( PktBlobEntry& blobEntry );

    bool                        setPluginSetting( PluginSetting& pluginSetting );
    bool                        getPluginSetting( enum EPluginType pluginType, PluginSetting& pluginSetting );

    void           				setPluginPermission( EPluginType pluginType, int iPluginPermission );
    EFriendState		        getPluginPermission( EPluginType pluginType );
    bool                        getIsPluginInTestState( EPluginType pluginType, VxGUID& onlineId );

    PluginLibraryServer&        getPluginLibraryServer( void )                  { return *m_PluginLibraryServer; }
	PluginFileShareServer&		getPluginFileShareServer( void )				{ return *m_PluginFileShareServer; }
	PluginNetServices&			getPluginNetServices( void )					{ return *m_PluginNetServices; }

	void           				setHasAboutMeContent( bool hasContent );
    void           				setHasStoryboardContent( bool hasContent );
	void           				setHasSharedWebCam(  bool hasShaeredWebCam );
	bool						isContactConnected( VxGUID& onlineId );

    bool                        isUserConnected( VxGUID& onlineId );
    bool                        isMemberGuest( enum EPluginType pluginType, VxGUID& onlineId ); // true if user is member of same host as I am

	//========================================================================
	// from gui
	//========================================================================
	void						assureUserSpecificDirIsSet( const char* checkReason );

    void           				fromGuiAppStartup( std::string assetDir, std::string rootDataDir, bool fromThread = false ) override;
    void           				fromGuiSetUserSpecificDir( std::string userSpecificDir, bool fromThread = false ) override;
    void           				fromGuiSetUserXferDir( std::string userXferDir, bool fromThread = false ) override;
    void           				fromGuiUserLoggedOn( VxNetIdent* netIdent, bool fromThread = false ) override;

    void           				fromGuiAppShutdown( void ) override;

    void           				fromGuiBlockUser( VxGUID& onlineId, bool fromThread = false ) override;

	bool				        fromGuiDeleteUser( VxGUID& onlineId ) override;

    uint64_t			        fromGuiGetDiskFreeSpace( const char* dir = nullptr  ) override;
    uint64_t			        fromGuiClearCache( ECacheType cacheType ) override;

    void           				fromGuiOnlineNameChanged( const char* newOnlineName ) override;
    void           				fromGuiMoodMessageChanged( const char* newMoodMessage ) override;
    void           				fromGuiIdentPersonalInfoChanged( int age, int gender, int language, int preferredContent ) override;

    void           				fromGuiSetUserHasProfilePicture( bool haveProfilePick ) override;
    void           				fromGuiUpdateMyIdent( VxNetIdent* netIdent, bool permissionAndStatsOnly = false ) override;
    void           				fromGuiQueryMyIdent( VxNetIdent* poRetIdent ) override;
    void           				fromGuiSetIdentHasTextOffers( VxGUID& onlineId, bool hasTextOffers ) override;

    bool           				fromGuiOrientationEvent( float f32RotX, float f32RotY, float f32RotZ  ) override;
    bool           				fromGuiMouseEvent( enum EMouseButtonType eMouseButType, enum EMouseEventType eMouseEventType, int iMouseXPos, int iMouseYPos  ) override;
    bool           				fromGuiMouseWheel( float f32MouseWheelDist ) override;
    bool           				fromGuiKeyEvent( enum EKeyEventType eKeyEventType, EKeyCode eKey, int iFlags = 0 ) override;

    void           				fromGuiNativeGlInit( void ) override;
    void           				fromGuiNativeGlResize( int w, int h ) override;
    int            				fromGuiNativeGlRender( void ) override;
    void           				fromGuiNativeGlPauseRender( void ) override;
    void           				fromGuiNativeGlResumeRender( void ) override;
    void           				fromGuiNativeGlDestroy( void ) override;

    void           				fromGuiMuteMicrophone( bool muteMic ) override;
    bool           				fromGuiIsMicrophoneMuted( void ) override;
    void           				fromGuiMuteSpeaker(	bool muteSpeaker ) override;
    bool           				fromGuiIsSpeakerMuted( void ) override;

    bool           				fromGuiSndRecord( enum ESndRecordState eRecState, VxGUID& feedId, const char* fileName ) override;
    bool           				fromGuiVideoRecord( enum EVideoRecordState eRecState, VxGUID& feedId, const char* fileName ) override;
    bool           				fromGuiPlayLocalMedia( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, uint8_t fileType, int pos0to100000 = 0 ) override;
    bool           				fromGuiPlayLocalMedia( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, uint8_t fileType, VxGUID assetId, int pos0to100000 = 0 ) override;

    bool           				fromGuiAssetAction( enum EAssetAction assetAction, AssetBaseInfo& assetInfo, int pos0to100000 = 0 ) override;
    bool				        fromGuiQueueAssetAction( enum EAssetAction assetAction, AssetBaseInfo& assetInfo, int pos0to100000 = 0 ) override;
    bool           				fromGuiAssetAction( enum EPluginType pluginType, enum EAssetAction assetAction, VxGUID& assetId, int pos0to100000 = 0 ) override;
    bool           				fromGuiSendAsset( AssetBaseInfo& assetInfo ) override;

    void           				fromGuiWantMediaInput( VxGUID& onlineId, enum EMediaInputType mediaType, MediaCallbackInterface * callback, enum EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput ) override;
    void           				fromGuiWantMediaInput( VxGUID& onlineId, enum EMediaInputType mediaType, enum EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput ) override;

    void           				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl, bool fromThread = false ) override;
    void           				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl, bool fromThread = false ) override;
    void           				fromGuiLeaveHost( HostedId& adminId, bool fromThread = false ) override;
    void           				fromGuiUnJoinHost( HostedId& adminId, bool fromThread = false ) override;

    void           				fromGuiSearchHost( enum EHostType hostType, SearchParams& searchParams, bool enable, bool fromThread = false ) override;

    void				        fromGuiSendAnnouncedList( enum EHostType hostType, VxGUID& sessionId ) override;

    void				        fromGuiDisconnectFromUser( VxGUID& onlineId ) override;

    void           				fromGuiRunIsPortOpenTest( uint16_t port ) override;
    void           				fromGuiRunUrlAction( VxGUID& sessionId, const char* myUrl, const char* ptopUrl, ENetCmdType testType ) override;

	void           				fromGuiUpdateWebPageProfile(	const char*	pProfileDir,	// directory containing user profile
																const char*	strGreeting,	// greeting text
																const char*	aboutMe,		// about me text
																const char*	url1,			// favorite url 1
																const char*	url2,			// favorite url 2
                                                                const char*	url3,           // favorite url 3
                                                                const char*	donation ) override;	// donation		

    void           				fromGuiApplyNetHostSettings( NetHostSetting& netSettings ) override;
    void           				fromGuiSetNetSettings( NetSettings& netSettings ) override;
    void           				fromGuiGetNetSettings( NetSettings& netSettings ) override;
    void           				fromGuiSetRelaySettings( int userRelayMaxCnt, int systemRelayMaxCnt ) override;

    void           				fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings ) override;
    void           				fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings ) override;

    void           				fromGuiSetPluginPermission( enum EPluginType pluginType, enum EFriendState eFriendState ) override;
    int            				fromGuiGetPluginPermission( enum EPluginType pluginType ) override;
    EPluginServerState	        fromGuiGetPluginServerState( enum EPluginType pluginType ) override;

    bool           				fromGuiStartPluginSession( enum EPluginType pluginType, VxGUID onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;
    void           				fromGuiStopPluginSession( enum EPluginType pluginType, VxGUID onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID()  ) override;
    bool           				fromGuiIsPluginInSession( enum EPluginType pluginType, VxGUID& onlineId = VxGUID::nullVxGUID(), VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

	bool           				fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;
    bool           				fromGuiToPluginOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo ) override;

    virtual EXferError			fromGuiFileXferControl( enum EPluginType pluginType, enum EXferAction xferAction, FileInfo& fileInfo ) override;

	bool           				fromGuiInstMsg(	enum EPluginType	pluginType, VxGUID&	onlineId, const char* pMsg ) override;
                                                   
    bool           				fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk ) override;

	bool           				fromGuiChangeMyFriendshipToHim(	VxGUID&	onlineId, enum EFriendState myFriendshipToHim, enum EFriendState hisFriendshipToMe ) override;															
                                                                
    void           				fromGuiSendContactList( enum EFriendViewType eFriendView, int maxContactsToSend ) override;
    void           				fromGuiRefreshContactList( int maxContactsToSend ) override;

    void           				fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing ) override;

    void           				fromGuiRelayPermissionCount( int userPermittedCount, int anonymousCount );

    void           				fromGuiStartScan( enum EScanType eScanType, uint8_t searchFlags, uint8_t fileTypeFlags, const char* pSearchPattern = "" ) override;
    void           				fromGuiNextScan( enum EScanType eScanType ) override;
    void           				fromGuiStopScan( enum EScanType eScanType ) override;

    InetAddress			        fromGuiGetMyIpAddress( void ) override;
    InetAddress			        fromGuiGetMyIPv4Address( void ) override;
    InetAddress			        fromGuiGetMyIPv6Address( void ) override;

    void           				fromGuiCancelDownload( VxGUID& fileInstance ) override;
    void           				fromGuiCancelUpload( VxGUID& fileInstance ) override;

	bool           				fromGuiTodGameActionSend( enum EPluginType	pluginType, VxGUID& onlineId, ETodGameAction todGameAction ) override;

	bool           				fromGuiTestCmd(	enum ETestParam1 testParam1, int	testParam2 = 0, const char* testParam3 = nullptr ) override;                                            

    virtual uint16_t			fromGuiGetRandomTcpPort( void ) override;
    /// Get url for this node
    void                        fromGuiGetNodeUrl( std::string& nodeUrl ) override;
    /// Get internet status
    virtual EInternetStatus     fromGuiGetInternetStatus( void ) override;
    /// Get network status
    virtual ENetAvailStatus     fromGuiGetNetAvailStatus( void ) override;

    bool           				fromGuiBrowseFiles( VxGUID& appInstId, std::string& folderName, uint8_t fileFilterMask = VXFILE_TYPE_ALLNOTEXE | VXFILE_TYPE_DIRECTORY ) override;

    // returns -1 if unknown else percent downloaded
    int            			    fromGuiGetFileDownloadState( uint8_t* fileHashId ) override;

    bool           				fromGuiSetFileIsShared( FileInfo& fileInfo, bool isShared ) override;
    bool           				fromGuiGetIsFileShared( FileInfo& fileInfo ) override;

    bool           				fromGuiSetFileIsInLibrary( FileInfo& fileInfo, bool isInLibrary ) override;
    bool           				fromGuiGetFileIsInLibrary( FileInfo& fileInfo ) override;

    void           				fromGuiGetFileLibraryList( VxGUID& appInstId, uint8_t fileTypeFilter ) override;

    void           				fromGuiScanFolderForMedia( VxGUID& appInstId, std::string dirToScan, uint8_t fileTypeFilter, bool fromThread = false ) override;
    void				        fromGuiScanItemReceived( VxGUID& appInstId ) override;
    void           				fromGuiScanFolderCancel( VxGUID& appInstId ) override;

    bool           				fromGuiIsNoLimitVideoFile( const char* fileName ) override;
    bool           				fromGuiIsNoLimitAudioFile( const char* fileName ) override;

    int            				fromGuiDeleteFile( std::string fileName, bool shredFile ) override;

    void           				fromGuiQuerySessionHistory( GroupieId& groupieId ) override;
    bool           				fromGuiMultiSessionAction( enum EMSessionAction mSessionAction, VxGUID& onlineId, int pos0to100000, VxGUID lclSessionId = VxGUID::nullVxGUID() ) override;

    int					        fromGuiGetAnnouncedHostCount( enum EHostType hostType ) override;
    int            				fromGuiGetJoinedListCount( enum EPluginType pluginType ) override;
    void                        fromGuiListAction( enum EListAction listAction ) override;
    std::string			        fromGuiQueryDefaultUrl( enum EHostType hostType, bool ignoreMyself = false ) override;
    bool                        fromGuiSetDefaultUrl( enum EHostType hostType, std::string& hostUrl ) override;
    bool           				fromGuiQueryIdentity( std::string& url, VxNetIdent& retNetIdent, bool requestIdentityIfUnknown ) override;
    bool           				fromGuiQueryIdentity( VxGUID onlineId, VxNetIdent& retNetIdent ) override;
    bool           				fromGuiQueryHosts( std::string& netHostUrl, enum EHostType hostType, std::vector<HostedInfo>& hostedInfoList, VxGUID& hostIdIfNullThenAll ) override;
    bool           				fromGuiQueryMyHostedInfo( enum EHostType hostType, std::vector<HostedInfo>& hostedInfoList ) override;
    bool           				fromGuiQueryHostListFromNetworkHost( VxPtopUrl& netHostUrl, enum EHostType hostType, VxGUID& hostIdIfNullThenAll ) override;
    bool           				fromGuiQueryGroupiesFromHosted( VxPtopUrl& hostedUrl, enum EHostType hostType, VxGUID& onlineIdIfNullThenAll ) override;

    bool           				fromGuiDownloadWebPage( enum EWebPageType webPageType, VxGUID& onlineId ) override;
    bool           				fromGuiCancelWebPage( enum EWebPageType webPageType, VxGUID& onlineId ) override;

    bool           				fromGuiDownloadFileList( enum EPluginType pluginType, VxGUID& onlineId, VxGUID& sessionId, uint8_t fileTypes = 0 ) override;
    bool           				fromGuiDownloadFileListCancel( enum EPluginType pluginType, VxGUID& onlineId, VxGUID& sessionId ) override;

    EJoinState		            fromGuiQueryJoinState( enum EHostType hostType, VxNetIdent& netIdent ) override;

    void           				fromGuiUpdatePluginPermission( enum EPluginType pluginType, enum EFriendState pluginPermission ) override;

    bool           				fromGuiQueryFileHash( FileInfo& fileInfo ) override;
    void           				fromGuiFileHashGenerated( std::string& fileNameAndPath, int64_t fileLen, VxSha1Hash& fileHash );

    bool				        fromGuiDeleteDatabase( enum EDatabaseType databaseType ) override;

    void				        fromGuiSetIsAutomatedHost( bool automatedHost ) override;

    bool                        fromGuiSendRandConnectSelected( VxGUID& onlineId, bool isSelected ) override;
    bool                        fromGuiSendRandConnectAction( VxGUID& onlineId,
                                                              enum ERandAction randAction,
                                                              VxGUID sessionId = VxGUID::nullVxGUID(),
                                                              uint64_t timeRequestedMs = 0,
                                                              EOfferType offerType = eOfferTypeUnknown );

    bool                        fromGuiQueryFriendRequest( std::vector<std::shared_ptr<FriendRequestInfo>>& friendRequestList, VxGUID& onlineIdIfNullThenAll ) override;
    bool                        fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestText, EFriendState myFriendshipToHim ) override;

	//========================================================================
	// to gui
	//========================================================================
    void						sendToGuiStatusMessage( const char* statusMsg, ... );

	void						toGuiContactAnythingChange( VxNetIdent* netIdent );

	int 						toGuiSendAdministratorList( int iSentCnt, int iMaxSendCnt );
	int 						toGuiSendFriendList( int iSentCnt, int iMaxSendCnt );
	int 						toGuiSendGuestList( int iSentCnt, int iMaxSendCnt );
	int							toGuiSendAnonymousList( int iSentCnt, int iMaxSendCnt );
	int							toGuiSendIgnoreList( int iSentCnt, int iMaxSendCnt );

	//========================================================================
	// asset mgr callbacks
	//========================================================================
    void           				callbackFileWasShredded( std::string& fileName ) override;
    void           				callbackAssetAdded( AssetBaseInfo* assetInfo ) override;
    void           				callbackAssetRemoved( AssetBaseInfo* assetInfo ) override;

	void           				callbackSharedFileTypesChanged( uint16_t fileTypes );
	void           				callbackSharedPktFileListUpdated( void );

    void           				callbackAssetHistory( void * userData, AssetBaseInfo* assetInfo ) override;
    //========================================================================
    // host list mgr callbacks
    //========================================================================
    void           				callbackBlobAdded( BlobInfo* blobInfo ) override;
    void           				callbackBlobRemoved( BlobInfo* blobInfo ) override;
    void           				callbackBlobHistory( BlobInfo* blobInfo ) override;

	//========================================================================
	//========================================================================
    void                        enableTimerThread( bool enable );
    void                        executeTimerThreadFunctions( void );

    void						fromGuiOncePerSecond( void ); // from qt gui thread triggers release of engine thread timed events.. allows better gui performance
	void						onOncePerSecond( void );
	void						onOncePer30Seconds( void );
	void						onOncePerMinute( void );
    void						onOncePer5Minutes( void );
    void						onOncePer10Minutes( void );
    void						onOncePer15Minutes( void );
    void						onOncePer30Minutes( void );
	void						onOncePerHour( void );

	void						onBigListInfoRestored( BigListInfo * poInfo ); 
	void						onBigListLoadComplete( int32_t rc );
	void						onBigListInfoDelete( BigListInfo * poInfo );

	virtual	void				onContactConnected		( RcConnectInfo * poInfo, bool connectionListLocked, bool newContact = false );
	virtual	void				onContactDisconnected	( RcConnectInfo * poInfo, bool connectionListLocked );

    void                        onConnectionClosing( std::shared_ptr<VxSktBase>& sktBase );
	void						onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );

	void						onSessionStart( enum EPluginType pluginType, VxGUID& onlineId );
	//========================================================================
	//========================================================================

	void						handleTcpData( std::shared_ptr<VxSktBase>& sktBase );

	std::string					describeContact( BigListInfo * bigListInfo );
	std::string					describeContact( VxConnectInfo& connectInfo );
	std::string					describeContact( ConnectRequest& connectRequest );

	void						broadcastSystemPkt( VxPktHdr* pkt, bool onlyIncludeMyContacts );
	void						broadcastSystemPkt( VxPktHdr* pkt, VxGUIDList& retIdsSentPktTo );

    bool           				txSystemPkt( const VxGUID&                  destOnlineId,
                                             std::shared_ptr<VxSktBase>&    sktBase,
                                             VxPktHdr*                      poPkt );

	void           				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );

	bool						connectToContact(	VxConnectInfo&		        connectInfo, 
													std::shared_ptr<VxSktBase>&	ppoRetSkt,
													bool&				        retIsNewConnection,
													EConnectReason		        connectReason = eConnectReasonStayConnected );

	void           				doPktAnnHasChanged( bool connectionListIsLocked );

	bool						shouldInfoBeInDatabase( BigListInfo * poInfo );

    void                        sktMgrStatusCallback( std::string& sktAction, SOCKET sktHandle );

	//! called if hacker offense is detected
    void						hackerOffense(  enum EHackerLevel	hackerLevel,
                                                enum EHackerReason   hackerReason,
                                                InetAddress		ipAddr,					// ip address 
                                                VxGUID          signatureGuid,			// users identity info ( may be null if not known then use ipAddress )
                                                const char*     pMsg, ... );			// message about the offense

	void						hackerOffense(	enum EHackerLevel	hackerLevel,			    
                                                enum EHackerReason   hackerReason,
                                                VxNetIdent*	    poContactIdent,			// users identity info ( may be null if not known then use ipAddress )
												InetAddress		IpAddr,					// ip address if identity not known
												const char*	    pMsg, ... );			// message about the offense

    void                        hackerOffense(  enum EHackerLevel	hackerLevel,			    
                                                enum EHackerReason  hackerReason,
                                                VxPktHdr*	        pktHdr,			// users identity info ( may be null if not known then use ipAddress )
                                                std::shared_ptr<VxSktBase>&      sktBase,
                                                const char*	        pMsg, ... );			// message about the offense

	//========================================================================
	// pkt handlers
	//========================================================================

    //=== packet handlers ===//
    void                        handlePkt                   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktUnhandled              ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktInvalid				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktAnnounce				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAnnList				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktScanReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktScanReply			    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktPluginOfferReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktPluginOfferReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktChatReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktChatReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktVoiceReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktVoiceReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktVideoFeedReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktVideoFeedStatus		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktVideoFeedPic			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktVideoFeedPicChunk		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktVideoFeedPicAck		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFileGetReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFindFileReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFindFileReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileListReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileListReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFileInfoReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFileChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileChunkReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileGetCompleteReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileXferCancel         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileShareErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktAssetGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetChunkReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktAssetXferErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktMultiSessionReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktMultiSessionReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktSessionStartReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktSessionStartReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktSessionStopReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktSessionStopReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktMyPicSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktMyPicSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerPicChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerPicChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerGetChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerGetChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerPutChunkTx	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktWebServerPutChunkAck	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktTodGameStats			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktTodGameAction			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktTodGameValue			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktTcpPunch				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktPingReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktPingReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktImAliveReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktImAliveReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktBlobSendReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobSendReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobChunkReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobChunkReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobSendCompleteReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobSendCompleteReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktBlobXferErr            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostJoinReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostJoinReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostLeaveReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostLeaveReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostUnJoinReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUnJoinReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostSearchReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostSearchReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostOfferReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostOfferReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFriendOfferReq         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFriendOfferReply       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktThumbGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktThumbXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktOfferSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktOfferXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktPushToTalkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktPushToTalkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktPushToTalkStart        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktPushToTalkStop         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktMembershipReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktMembershipReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostInfoReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInfoReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostInviteAnnReq       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInviteAnnReply     ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInviteSearchReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInviteSearchReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInviteMoreReq      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostInviteMoreReply    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktGroupieInfoReq         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieInfoReply       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktGroupieAnnReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieAnnReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieSearchReq       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieSearchReply     ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieMoreReq         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktGroupieMoreReply       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFileInfoInfoReq        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoInfoReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFileInfoAnnReq         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoAnnReply       ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoSearchReq      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoSearchReply    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoMoreReq        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktFileInfoMoreReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktRelayUserDisconnect    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostUserInfoReq        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserInfoReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserStatusReq      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserStatusReply    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktHostUserListReq        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserListReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserListMoreReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    void           				onPktHostUserListMoreReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
    
	void           				onPktTestConnTestReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktTestConnTestReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktTestConnPingReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktTestConnPingReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

	void           				onPktQueryHostUrlReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktQueryHostUrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktStreamCtrlReq		    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktStreamCtrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktRandConnectReq		    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktRandConnectReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktFriendRequestReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;
	void           				onPktFriendRequestReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    void           				onPktAdminAvail             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) override;

    bool                        validateIdent( VxNetIdent* netIdent ); // extra validatation for at risk connections like multicast

    void                        onNetworkConnectionReady( bool requiresRelay, std::string& ipAddr, uint16_t ipPort );

    /// extract online id from url if url is valid
    static VxGUID               getOnlineIdFromUrl( std::string& ptopUrl );

    std::string                 describeConnectId( ConnectId& connectionId );
    std::string                 describeGroupieId( GroupieId& groupieId );
    std::string                 describeHostedId( HostedId& hostedId );
    std::string                 describeUser( VxGUID& onlineId );

    void                        onStreamStop( VxGUID& streamId );

    bool                        isMyAccessAllowedFromHim( VxGUID& onlineId, EPluginType pluginType );
    bool                        isHisAccessAllowedFromMe( VxGUID& onlineId, EPluginType pluginType );

    void                        disconnectFromHostIfNotNeeded( HostedId& adminId );

    bool                        isTestGuid( VxGUID& onlineId ) { return m_TestGuid == onlineId; }

protected:
    //========================================================================
    //========================================================================
    void						iniitializePtoPEngine( void );

	bool           				txPluginPkt( 	enum EPluginType			pluginType, 
												VxNetIdent*	                netIdent, 
												std::shared_ptr<VxSktBase>&	sktBase, 
												VxPktHdr*			        poPkt );

	void           				doAppStateChange( enum EAppState eAppState );
	bool           				shouldNotifyGui( VxNetIdent* netIdent );

	// pkt ann has changed and needs to be re announced
	void						doPktAnnConnectionInfoChanged( bool connectionListIsLocked );
	void				        attemptConnectionToRelayService( BigListInfo * poInfo );
	void						handleIncommingRelayData( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	void						sendToGuiTheContactList( int maxContactsToSend );
    void                        updateIdentLists( BigListInfo* bigListInfo, int64_t timestampMs = 0 );

    // called when connected to or recieve pkt announce
    bool                        updateOnFirstConnect( std::shared_ptr<VxSktBase>& sktBase, BigListInfo* bigListInfo, bool nearbyLanConnected );

    bool						onFirstPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo, ConnectId& connectId );
    bool						onConnectionPktAnnounceUpdated( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo );
    bool						onHostedUserPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo, ConnectId& connectId );
    bool                        onRelayedUserPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo );
    bool						onUnexpectedPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo );

    bool						onPktAnnounceCommonHandler( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo );

    EMembershipState            getMembershipState( PktAnnounce& myPktAnn, VxNetIdent* netIdent, enum EPluginType pluginType, enum EFriendState myFriendshipToHim );

    void                        updateMyPktAnnIpAddress( std::string ipAddr );
    void                        updateMyNetworkServiceUrl( EHostType hostType );

	//=== vars ===//
	VxPeerMgr&					m_PeerMgr;
    FromGuiMgr                  m_FromGuiMgr;
    ConnectIdListMgr            m_ConnectIdListMgr;
    IgnoreListMgr               m_IgnoreListMgr;
    FileMgr&                    m_FileMgr;
    FriendListMgr               m_FriendListMgr;
    FriendRequestMgr&           m_FriendRequestMgr;
    GroupieListMgr              m_GroupieListMgr;
    HostUrlListMgr              m_HostUrlListMgr;
    HostedListMgr               m_HostedListMgr;
    BigListMgr					m_BigListMgr;

	PktAnnounce					m_PktAnn;
    int64_t                     m_PktAnnLastModTime{ 0 };
    VxGUID                      m_MyOnlineId;
	VxMutex						m_AnnouncePktMutex;
	EngineSettings				m_EngineSettings;
	EngineParams				m_EngineParams;
    NetStatusAccum              m_NetStatusAccum;
	AssetMgr&					m_AssetMgr;
    BlobMgr&				    m_BlobMgr;
    OfferMgr&				    m_OfferMgr;
    PushToTalkMgr&				m_PushToTalkMgr;
    ThumbMgr&					m_ThumbMgr;
    ConnectionMgr&              m_ConnectionMgr;
    ConnectMgr&                 m_ConnectMgr;
	P2PConnectList				m_ConnectionList;
    MediaProcessor&				m_MediaProcessor;
    MemberActiveMgr&            m_MemberActiveMgr;

    NetworkMgr&					m_NetworkMgr;
	NetworkMonitor&				m_NetworkMonitor;
	NetServicesMgr&				m_NetServicesMgr;
	StayConnected&				m_StayConnected;

	PluginMgr&					m_PluginMgr;
    PluginSettingMgr			m_PluginSettingMgr;

	PluginFileShareServer*	    m_PluginFileShareServer;
    PluginLibraryServer*        m_PluginLibraryServer;
	PluginNetServices*			m_PluginNetServices;

	IsPortOpenTest&				m_IsPortOpenTest;
    RandConnectMgr&             m_RandConnectMgr;
    RelayMgr                    m_RelayMgr;
    RunUrlAction&			    m_RunUrlAction;
    SendQueueMgr&			    m_SendQueueMgr;

    HostServerJoinMgr&			m_HostJoinMgr;
    UserJoinMgr&				m_UserJoinMgr;
    WebPageMgr&                 m_WebPageMgr;
    std::shared_ptr<VxSktBase>  m_SktLoopback;

	RcScan						m_RcScan;

    enum EAppState				m_eAppState{ eAppStateInvalid };

    enum EFriendViewType		m_eFriendView{ eFriendViewEverybody };
	unsigned int				m_iCurPreferredRelayConnectIdx{ 0 };
	VxGUID						m_NextFileInstance;
    bool						m_AppStartupCalled{ false };

	bool						m_IsUserSpecificDirSet{ false };
    bool                        m_EngineInitialized{ false };
    bool                        m_IsEngineCreated{ false };
    bool                        m_IsEngineReady{ false };
    bool                        m_NetworkConnectionReady{ false };
    bool                        m_PktMgrNetworkReadyWasCalled{ false };

	PktImAliveReq				m_PktImAliveReq;

    VxSemaphore                 m_TimerThreadSemaphore;
    VxThread                    m_TimerThread;

    VxGUID                      m_TestGuid;
};

extern P2PEngine& GetPtoPEngine();
