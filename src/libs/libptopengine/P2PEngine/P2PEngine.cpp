//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include "P2PConnectList.h"
#include "VxSktLoopback.h"

#include <AssetMgr/AssetMgr.h>
#include <BlobXferMgr/BlobMgr.h>
#include <BigListLib/BigListInfo.h>
#include <FriendRequestMgr/FriendRequestMgr.h>
#include <HostServerJoinMgr/HostServerJoinMgr.h>

#include <MediaProcessor/MediaProcessor.h>
#include <Membership/MemberActiveMgr.h>

#include <NetworkTest/IsPortOpenTest.h>
#include <NetworkTest/RunUrlAction.h>
#include <Network/NetworkMgr.h>
#include <Network/StayConnected.h>
#include <NetworkMonitor/NetworkMonitor.h>
#include <NetServices/NetServicesMgr.h>

#include <Plugins/PluginFileShareServer.h>
#include <Plugins/PluginLibraryServer.h>
#include <Plugins/PluginMgr.h>
#include <Plugins/PluginNetServices.h>
#include <PluginSettings/PluginSettingMgr.h>

#include <RandConnect/RandConnectMgr.h>
#include <Search/RcScan.h>
#include <SendQueue/SendQueueMgr.h>

#include <UserJoinMgr/UserJoinMgr.h>

#include <UrlMgr/UrlMgr.h>

#include <CoreLib/AppErr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxPtopUrl.h>
#include <CoreLib/VxSktUtil.h>

#include <NetLib/VxPeerMgr.h>

#include <PktLib/PktsBaseXfer.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

namespace
{
	void AppErrHandler( void * userData, EAppErr eAppErr, char * errMsg )
	{
		if( userData )
		{
			((P2PEngine *)userData)->getToGui().toGuiAppErr( eAppErr, errMsg );
		}
	}
}

//============================================================================
P2PEngine::P2PEngine( VxPeerMgr& peerMgr, 
					  MemberActiveMgr& memberActiveMgr, 
					  OfferMgr& offerMgr,
					  PushToTalkMgr& pushToTalkMgr,
					  RandConnectMgr& randConnectMgr,
					  SendQueueMgr& sendQueueMgr,
					  FriendRequestMgr& friendRequstMgr,
					  FileMgr& fileMgr )
	: m_PeerMgr( peerMgr )
	, m_FromGuiMgr( *this )
	, m_ConnectIdListMgr( *this )
	, m_IgnoreListMgr( *this )
	, m_FileMgr( fileMgr )
	, m_FriendListMgr( *this )
	, m_FriendRequestMgr( friendRequstMgr )
	, m_GroupieListMgr( *this )
	, m_HostUrlListMgr( *this )
	, m_HostedListMgr( *this )
	, m_BigListMgr( *this )
	, m_EngineSettings()
	, m_EngineParams()
	, m_NetStatusAccum( *this )
	, m_AssetMgr( *new AssetMgr( *this, "AssetMgrDb.db3", "AssetStateDb.db3" ) )
	, m_BlobMgr( *new BlobMgr( *this, "BlobAssetDb.db3", "BlobStateDb.db3" ) )
	, m_OfferMgr( offerMgr )
	, m_PushToTalkMgr( pushToTalkMgr )
	, m_ThumbMgr( *new ThumbMgr( *this, "ThumbAssetDb.db3", "ThumbStateDb.db3" ) )
	, m_ConnectionMgr( *new ConnectionMgr( *this ) )
	, m_ConnectMgr( *new ConnectMgr( *this, "ConnectMgrDb.db3", "ConnectStateDb.db3" ) )
	, m_ConnectionList( *this )
	, m_MediaProcessor( *( new MediaProcessor( *this ) ) )
	, m_MemberActiveMgr( memberActiveMgr )
	, m_NetworkMgr( *new NetworkMgr( *this, peerMgr, m_BigListMgr ) )
	, m_NetworkMonitor( *new NetworkMonitor( *this ) )
	, m_NetServicesMgr( *new NetServicesMgr( *this ) )
	, m_StayConnected( *new StayConnected( *this ) )
	, m_PluginMgr( *new PluginMgr( *this ) )
	, m_PluginSettingMgr( *this )
    , m_PluginFileShareServer( new PluginFileShareServer( *this, m_PluginMgr, &m_PktAnn, ePluginTypeFileShareServer ) )
	, m_PluginLibraryServer( new PluginLibraryServer( *this, m_PluginMgr, &m_PktAnn, ePluginTypeLibraryServer ) )
	, m_PluginNetServices( new PluginNetServices( *this, m_PluginMgr, &m_PktAnn, ePluginTypeNetServices ) )
	, m_IsPortOpenTest( *new IsPortOpenTest( *this, m_EngineSettings, m_NetServicesMgr, m_NetServicesMgr.getNetUtils() ) )
	, m_RandConnectMgr( randConnectMgr )
	, m_RelayMgr( *this )
	, m_RunUrlAction( *new RunUrlAction( *this, m_EngineSettings, m_NetServicesMgr, m_NetServicesMgr.getNetUtils() ) )
	, m_SendQueueMgr( sendQueueMgr )
	, m_HostJoinMgr( *new HostServerJoinMgr( *this, "HostJoinMgrDb.db3", "HostJoinedLastDb.db3" ) )
	, m_UserJoinMgr( *new UserJoinMgr( *this, "UserJoinMgrDb.db3", "UserJoinedLastDb.db3" ) )
	, m_WebPageMgr( *new WebPageMgr( *this ) )
    , m_SktLoopback( new VxSktLoopback( *this ) )
    , m_RcScan( *this, m_ConnectionList )
{
	m_SktLoopback->setThisSkt( m_SktLoopback ); // so skt can do callbacks without look up in manager

    m_PeerMgr.setSktLoopback( m_SktLoopback );
    m_NetStatusAccum.addNetStatusCallback( &m_ConnectionMgr );
    int maxPktType = MAX_PKT_TYPE_CNT;
    vx_assert( 156 == maxPktType ); // just to make sure our packet types are not mismatched

	// loadAccountSpecificSettings in gui calls this for get getTcpIpPort so need to be created right now 
	std::string strEngineSettingDbFileName = VxGetAppNoLimitDataDirectory();
	strEngineSettingDbFileName += "EngineSettings.db3";
	getEngineSettings().engineSettingsStartup( strEngineSettingDbFileName );

	m_IsEngineCreated = true;
    LogMsg( LOG_VERBOSE, "P2PEngine::P2PEngine created" );
}

//============================================================================
P2PEngine::~P2PEngine()
{
    //m_PluginMgr.pluginMgrShutdown();
}

//============================================================================
IToGui& P2PEngine::getToGui()
{
    return IToGui::getIToGui();
}

//============================================================================
IAudioRequests& P2PEngine::getAudioRequest()
{
    return IAudioRequests::getIAudioRequests();
}

//============================================================================
UrlMgr& P2PEngine::getUrlMgr()
{
	return GetUrlMgrInstance();
}

//============================================================================
void P2PEngine::startupEngine()
{
    LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::%s start", __func__ );
    iniitializePtoPEngine();

	m_RandConnectMgr.onEngineStartup();
	m_NetServicesMgr.netServicesStartup();
	m_StayConnected.stayConnectedStartup();
	m_PluginMgr.onAppStartup();
	m_IsPortOpenTest.isPortOpenTestStartup();
	m_FriendRequestMgr.friendRequestMgrStartup();
	m_MemberActiveMgr.memberActiveStartup();
    LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::%s done", __func__ );
}

//============================================================================
void P2PEngine::iniitializePtoPEngine( void )
{
    if( !m_EngineInitialized )
    {
        m_EngineInitialized = true;
        VxSetNetworkLoopbackAllowed( false );
        // not needed if do not need to send log to gui VxSetLogHandler( LogHandler, this );
        VxSetAppErrHandler( AppErrHandler, this );
        VxSocketsStartup();
        m_PluginMgr.pluginMgrStartup();

		// must be done after plugin manager so can request callbacks from plugins
		getSendQueueMgr().onEngineInitialized();
   }
}

//============================================================================
void P2PEngine::shutdownEngine( void )
{
	VxSetAppIsShuttingDown( true );
	m_FriendRequestMgr.friendRequestMgrShutdown();
	m_FromGuiMgr.fromGuMgrShutdown();
	
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: shutdown IsPortOpen" );
	m_IsPortOpenTest.isPortOpenTestShutdown();

	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: stop listening" );
	m_PeerMgr.stopListening( false );
	m_PeerMgr.stopListening( true );
	//LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: remove asset client" );
	//m_AssetMgr.addAssetMgrClient( this, false );
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: shutdown media processor" );
	m_MediaProcessor.shutdownMediaProcessor();
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: m_PeerMgr.sktMgrShutdown" );
	m_PeerMgr.sktMgrShutdown();

	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: m_PluginMgr.onAppShutdown" );
	m_PluginMgr.onAppShutdown();
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: m_StayConnected.stayConnectedShutdown" );
	m_StayConnected.stayConnectedShutdown();
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: m_NetServicesMgr.netServicesShutdown" );
	m_NetServicesMgr.netServicesShutdown();
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: m_PluginMgr.pluginMgrShutdown" );
	m_PluginMgr.pluginMgrShutdown();
	//m_RcScan.scanShutdown();
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: databases shutdown" );
	getEngineSettings().engineSettingsShutdown();
	getEngineParams().engineParamsShutdown();
	m_BigListMgr.bigListMgrShutdown();
	m_AssetMgr.assetInfoMgrShutdown();

	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: plugin shutdown" );
	m_PluginMgr.pluginMgrShutdown();

	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: waiting threads exit" );
	
	VxSleep( 1000 );
	for ( int i = 0; i < 8; i++ )
	{
		if ( 0 == VxThread::getThreadsRunningCount() )
		{
			break;
		}
		
		VxThread::dumpRunningThreads();
		VxSleep( 1000 );
	}
	
	m_PluginFileShareServer		= 0;
	m_PluginNetServices		= 0;
	LogMsg( LOG_VERBOSE, "P2PEngine::shutdownEngine: done" );
}

//============================================================================
void P2PEngine::onSessionStart( EPluginType pluginType, VxGUID& onlineId )
{
	bool shouldUpdateLastSessionTime = false;
	switch( pluginType )
	{
	case ePluginTypeMessenger:
	case ePluginTypeVoicePhone:
	case ePluginTypeVideoChat:
	case ePluginTypeTruthOrDare:
		shouldUpdateLastSessionTime = true;
	default:
		break;
	}

	if( shouldUpdateLastSessionTime )
	{
		int64_t sysTimeMs = GetGmtTimeMs();
		VxNetIdent* netIdent = getBigListMgr().findNetIdent( onlineId );
		if( netIdent )
		{
			netIdent->setLastSessionTimeMs( sysTimeMs );
		}
		else
		{
			vx_assert( false );
		}

		m_BigListMgr.dbUpdateSessionTime( netIdent->getMyOnlineId(), sysTimeMs );
		IToGui::getIToGui().toGuiContactLastSessionTimeChange( netIdent );
	}
}

//============================================================================
bool P2PEngine::shouldInfoBeInDatabase( BigListInfo * poInfo )
{
	if( poInfo->getMyOnlineId() == m_PktAnn.getMyOnlineId() )
	{
		return false;
	}

	EFriendState friendState =	poInfo->getMyFriendshipToHim();
	if( eFriendStateIgnore == friendState || eFriendStateGuest < friendState )
	{
	    if(LogEnabled(eLogUsers))LogModule( eLogUsers, LOG_INFO, "%s belongs in database. ", poInfo->getOnlineName() );
		return true;
	}

	//LogMsg( LOG_INFO, "%s does not belong in database", poInfo->getOnlineName() );
	return false;
}

//============================================================================
void P2PEngine::onBigListInfoRestored( BigListInfo * poInfo )
{
	updateIdentLists( poInfo, poInfo->getLastSessionTimeMs() );

	getConnectIdListMgr().updateOnlineExclusion( poInfo->getMyOnlineId(), poInfo->getMyFriendshipToHim() == eFriendStateIgnore);

    if( poInfo->canDirectConnectToUser() && ( poInfo->isFriend() || poInfo->isAdministrator() ) )
	{
        //m_StayConnected.addStayConnectedRequest( poInfo->getConnectInfo(), eConnectReasonStayConnected );
	}
}

//============================================================================
void P2PEngine::onBigListInfoDelete( BigListInfo * poInfo )
{
	LogMsg( LOG_INFO, "onBigListInfoDelete");
	poInfo->debugDumpIdent();
    getToGui().toGuiContactRemoved( poInfo->getConnectIdent().getMyOnlineId() );
    //m_StayConnected.removeStayConnectedRequest( poInfo->getConnectInfo() );
	if( poInfo->isMyRelay() )
	{
		m_ConnectionList.removeContactInfo( poInfo->getConnectInfo() );
	}

	//m_RcScan.onIdentDelete( poInfo );
	//NOTE: TODO.. there are many more places we should check for references to BigListInfo  or VxNetIdent
}

//============================================================================
//! called by big list when all friends are loaded
void P2PEngine::onBigListLoadComplete( int32_t rc )
{
	LogMsg( LOG_INFO, "onBigListLoadComplete %d", rc );
    getBigListMgr().bigListLock();
    for( auto iter = getBigListMgr().m_BigList.begin(); iter != getBigListMgr().m_BigList.end(); ++iter )
    {
        BigListInfo * info = iter->second;
        if( info && !info->isAnonymous() )
        {
            getToGui().toGuiContactAdded( info );
        }
        else
        {
            if( info )
            {
                LogMsg( LOG_ERROR, "onBigListLoadComplete anonymouse info %s %s", info->getOnlineName(), info->getMyOnlineIdHexString().c_str() );
            }
            else
            {
                LogMsg( LOG_ERROR, "onBigListLoadComplete null info" );
            }
        }
    }

    getBigListMgr().bigListUnlock();
}

//============================================================================
//! handle app state change
void P2PEngine::doAppStateChange( enum EAppState eAppState )
{
	m_PluginMgr.onAppStateChange( eAppState );
}

//============================================================================
bool P2PEngine::addMyIdentToBlob( PktBlobEntry& blobEntry )
{ 
    m_AnnouncePktMutex.lock(); 
    bool result = (*((VxNetIdent*)&m_PktAnn)).addToBlob( blobEntry ); 
    m_AnnouncePktMutex.unlock(); 
    return result;
}

//============================================================================
// true if have open port and ready to recieve
bool P2PEngine::isDirectConnectReady( void )
{
    bool directConnectReady = false;
    if( eFirewallTestAssumeNoFirewall == m_EngineSettings.getFirewallTestSetting() )
    {
        directConnectReady = m_NetStatusAccum.isInternetAvailable();
    }
    else
    {
        directConnectReady = m_NetStatusAccum.isInternetAvailable() && m_NetStatusAccum.isDirectConnectTested() && !m_NetStatusAccum.requiresRelay();
    }

    return directConnectReady;
}

//============================================================================
// true if netowrk host plugin is enabled
bool P2PEngine::isNetworkHostEnabled( void )
{
    m_AnnouncePktMutex.lock(); 
    bool netHostEnabled = ( eFriendStateIgnore != m_PktAnn.getPluginPermission( ePluginTypeHostNetwork ) ); 
    m_AnnouncePktMutex.unlock();
    return netHostEnabled;
}

//============================================================================
bool P2PEngine::setPluginSetting( PluginSetting& pluginSetting )
{
    if( ( ePluginTypeInvalid < pluginSetting.getPluginType() ) && ( eMaxPluginType > pluginSetting.getPluginType() ) )
    {
        return getPluginSettingMgr().setPluginSetting( pluginSetting );
    }

    return false;
}

//============================================================================
bool P2PEngine::getPluginSetting( enum EPluginType pluginType, PluginSetting& pluginSetting )
{
    if( ( ePluginTypeInvalid < pluginType ) && ( eMaxPluginType > pluginType ) )
    {
        return getPluginSettingMgr().getPluginSetting( pluginType, pluginSetting );
    }

    return false;
}

//============================================================================
EFriendState P2PEngine::getPluginPermission( EPluginType pluginType )
{
    EFriendState permission = eFriendStateIgnore;
	if( ( ePluginTypeInvalid < pluginType ) && 
		( eMaxPermissionPluginType > pluginType ) )
	{
		permission = m_PluginMgr.getPluginPermission(pluginType);
	}

	return permission;
}

//============================================================================
bool P2PEngine::getIsPluginInTestState( EPluginType pluginType, VxGUID& onlineId )
{
	if( ePluginTypePushToTalk == pluginType && onlineId == m_MyOnlineId )
	{
		return true;
	}

	return false;
}

//============================================================================
void P2PEngine::setPluginPermission( EPluginType pluginType, int iPluginPermission )
{
	if( ( ePluginTypeInvalid < pluginType ) && 
		( eMaxPermissionPluginType > pluginType ) )
	{
		if( ( iPluginPermission != m_PluginMgr.getPluginPermission( pluginType ) )
			|| ( iPluginPermission != m_PktAnn.getPluginPermission( pluginType ) ) )
		{
			fromGuiUpdatePluginPermission( pluginType, (EFriendState)iPluginPermission );
		}
	}
}

//============================================================================
//! if bHasPicture then user has updated profile picture		
void P2PEngine::setHasAboutMeContent( bool hasContent )
{
	lockAnnouncePktAccess();
	m_PktAnn.setHasAboutMeContent( hasContent );
	unlockAnnouncePktAccess();
	// let others handle the changed announcement packet
	doPktAnnHasChanged( false );
}

//============================================================================
//! if bHasPicture then user has updated profile picture		
void P2PEngine::setHasStoryboardContent( bool hasContent )
{
	lockAnnouncePktAccess();
	m_PktAnn.setHasStoryboardContent( hasContent );
	unlockAnnouncePktAccess();
	// let others handle the changed announcement packet
	doPktAnnHasChanged( false );
}

//============================================================================
//! if bHasShaeredWebCam then user bool has started web cam server		
void P2PEngine::setHasSharedWebCam( bool hasSharedWebCam )
{
	lockAnnouncePktAccess();
	m_PktAnn.setHasSharedWebCam( hasSharedWebCam );
	unlockAnnouncePktAccess();
	// let others handle the changed announcement packet
	doPktAnnHasChanged( false );
}

//============================================================================
void P2PEngine::updateIdentLists( BigListInfo* bigListInfo, int64_t timestampMs )
{
	int64_t timestamp;
	if( timestampMs )
	{
		timestamp = timestampMs;
	}
	else
	{
		timestamp = GetTimeStampMs();
	}

	if( bigListInfo->isIgnored() )
	{
		m_IgnoreListMgr.updateIdent( bigListInfo->getMyOnlineId(), timestamp );
		m_FriendListMgr.removeIdent( bigListInfo->getMyOnlineId() );
	}
	else
	{
		m_IgnoreListMgr.removeIdent( bigListInfo->getMyOnlineId() );
		m_FriendListMgr.removeIdent( bigListInfo->getMyOnlineId() );
		if( bigListInfo->isAdministrator() || bigListInfo->isFriend() )
		{
			m_FriendListMgr.updateIdent( bigListInfo->getMyOnlineId(), timestamp );
		}
	}
}

//============================================================================
bool validateArrayString( const char* str, int minStrLen, int maxStrLen )
{
	int strLen = 0;
	bool nullTerminatorFound = false;
	for( int i = 0; i < maxStrLen; ++i )
	{
		if( 0 == str[i] )
		{
			nullTerminatorFound = true;
			break;
		}

		if( isascii( str[i] ) )
		{
			strLen++;
		}
		else
		{
			LogMsg( LOG_ERROR, "validateArrayString invalid char 0x%X", str[i] );
			return false;
		}
	}

	return strLen >= minStrLen && nullTerminatorFound;
}

//============================================================================
bool P2PEngine::validateIdent( VxNetIdent* netIdent )
{
	// validate port is reasonable value
	if( 80 > netIdent->getOnlinePort() )
	{
		LogMsg( LOG_ERROR, "%s invalid port %d", __func__, netIdent->getOnlinePort() );
		return false;
	}

	// validate online name
	if( !validateArrayString( netIdent->getOnlineName(), 4, MAX_ONLINE_NAME_LEN ) )
	{
		LogMsg( LOG_ERROR, "%s invalid name", __func__ );
		return false;
	}

	if( !validateArrayString( netIdent->getOnlineDescription(), 4, MAX_ONLINE_DESC_LEN ) )
	{
		LogMsg( LOG_ERROR, "%s invalid description", __func__ );
		return false;
	}

	if( !netIdent->getMyOnlineId().isValid() )
	{
		LogMsg( LOG_ERROR, "%s invalid online id", __func__ );
		return false;
	}

	return true;
}

//============================================================================
void P2PEngine::onNetworkConnectionReady( bool requiresRelay, std::string& ipAddr, uint16_t ipPort )
{
	// called after network connection test
	if( !m_NetworkConnectionReady )
	{
		m_NetworkConnectionReady = true;
		if( m_PluginMgr.isPluginMgrReady() )
		{
			// fire up web cam, file share and other hosted services
			getPluginMgr().onNetworkConnectionReady( requiresRelay );
		}

		doPktAnnHasChanged( false );
	}

	// Update the m_PktAnn with the discovered external IP address so that
	// incoming connections can properly decrypt the first packet
	if( !ipAddr.empty() )
	{
		updateMyPktAnnIpAddress( ipAddr );
	}

	getToGui().toGuiNetworkIsTested( requiresRelay, ipAddr, ipPort );
}

//============================================================================
// true if user is member of same host as I am
bool P2PEngine::isMemberGuest( enum EPluginType pluginType, VxGUID& onlineId )
{
	// TODO BRJ determine if user is a guest member of either my hosted service or a hosted service I joined
	return true;
}

//============================================================================
VxGUID P2PEngine::getOnlineIdFromUrl( std::string& ptopUrl )
{
	VxGUID onlineId;
	VxPtopUrl hostUrl( ptopUrl );
    if( hostUrl.isValid() )
    {
        onlineId = hostUrl.getOnlineId();
    }

	return onlineId;
}

//============================================================================
std::string P2PEngine::describeConnectId( ConnectId& connectionId )
{
	std::string desc = "skt id ";
	desc += connectionId.getSocketId().toHexString();
	desc += connectionId.isRelayed() ? " relayed " : " direct ";
	desc += describeGroupieId( connectionId.getGroupieId() );
	return desc;
}

//============================================================================
std::string P2PEngine::describeGroupieId( GroupieId& groupieId )
{
	std::string desc = DescribeHostType( groupieId.getHostType() );
	desc += describeUser( groupieId.getHostOnlineId() );
	desc += " user ";
	desc += describeUser( groupieId.getUserOnlineId() );
	return desc;
}

//============================================================================
std::string P2PEngine::describeHostedId( HostedId& hostedId )
{
	std::string adminDesc = DescribeHostType( hostedId.getHostType() );
	adminDesc += " admin ";
	adminDesc += describeUser( hostedId.getHostOnlineId() );
	return adminDesc;
}

//============================================================================
std::string P2PEngine::describeUser( VxGUID& onlineId )
{
    std::string userDesc;
    if( !getBigListMgr().getOnlineName( onlineId, userDesc ) )
    {
		if( onlineId == getMyOnlineId() )
		{
			userDesc = getMyPktAnnounce().getOnlineName();
		}
		else
		{
			userDesc = "Unknown User";
		}
    }

	userDesc += " id ";
	userDesc += onlineId.toOnlineIdString();
	return userDesc;
}

//============================================================================
void P2PEngine::onStreamStop( VxGUID& streamId )
{
	getToGui().toGuiFileXferState( ePluginTypeFileShareClient, streamId, eXferDirectionRx, eXferStateStreamStopped, eXferErrorNone, 0 );
	fromGuiCancelDownload( streamId );
}

//============================================================================
VxGUID& P2PEngine::getMyOnlineId( void ) 
{ 
	if( m_MyOnlineId.isValid() )
	{
		return m_MyOnlineId;
	}

	m_MyOnlineId = m_PktAnn.getMyOnlineId();
	if( ! m_MyOnlineId.isValid() )
	{
		LogMsg( LOG_SEVERE, "P2PEngine::getMyOnlineId id is INVALID" );
	}

	return m_MyOnlineId;
}
