//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include <BigListLib/BigListInfo.h>

#include <HostServerJoinMgr/HostServerJoinMgr.h>

#include <OfferBase/OfferMgr.h>
#include <Plugins/PluginNetServices.h>
#include <Plugins/PluginMgr.h>
#include <SendQueue/SendQueueMgr.h>

#include <UserJoinMgr/UserJoinMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxTime.h>

//============================================================================
void P2PEngine::fromGuiAppStartup( std::string assetsDir, std::string rootDataDir, bool fromThread )
{
	VxSetAppIsShuttingDown( false );

    if( fromThread )
    {
        LogMsg( LOG_VERBOSE, "P2PEngine::fromGuiAppStartup run" );
        enableTimerThread( true );

        LogMsg( LOG_INFO, "P2PEngine::fromGuiAppStartup %s %s", assetsDir.c_str(), rootDataDir.c_str() );
    }
    else
    {
        LogMsg( LOG_VERBOSE, "P2PEngine::fromGuiAppStartup queued" );
        m_FromGuiMgr.fromGuiAppStartup( assetsDir, rootDataDir );
    }
}

//============================================================================
void P2PEngine::fromGuiSetUserXferDir( std::string userXferDir, bool fromThread )
{
    if( fromThread )
    {
	    LogMsg( LOG_INFO, "P2PEngine::fromGuiSetUserXferDir %s", userXferDir.c_str() );
	    VxSetUserXferDirectory( userXferDir );
	    std::string incompleteDir = VxGetIncompleteDirectory();
	    // delete all incomplete files from previous usage
	    std::vector<std::string>	fileList;
	    VxFileUtil::listFilesInDirectory( incompleteDir.c_str(), fileList );
	    std::vector<std::string>::iterator iter;
	    for( iter = fileList.begin(); iter != fileList.end(); ++iter )
	    {
		    GetVxFileShredder().shredFile( *iter );
	    }
    }
    else
    {
        LogMsg( LOG_VERBOSE, "P2PEngine::fromGuiSetUserXferDir queued" );
        m_FromGuiMgr.fromGuiSetUserXferDir( userXferDir );
    }
}

//============================================================================
void P2PEngine::fromGuiSetUserSpecificDir( std::string userSpecificDir, bool fromThread )
{
    if( fromThread )
    {
        const int startupStartMs = GetApplicationAliveMs();
        static std::string userDir;
        if( userDir == userSpecificDir )
        {
            LogMsg( LOG_INFO, "P2PEngine::fromGuiSetUserSpecificDir %s called twice", userSpecificDir.c_str() );
            return;
        }

        userDir = userSpecificDir;
	    LogMsg( LOG_INFO, "P2PEngine::fromGuiSetUserSpecificDir %s", userSpecificDir.c_str() );
	    VxSetUserSpecificDataDirectory( userSpecificDir.c_str() );

	    std::string strDbFileName = VxGetSettingsDirectory();
	    strDbFileName += "biglist.db3";
	    int32_t rc = m_BigListMgr.bigListMgrStartup( strDbFileName.c_str() );
	    if( rc )
	    {
		    LogMsg( LOG_ERROR, "P2PEngine::startupEngine error %d bigListMgrStartup", rc );
	    }

	    strDbFileName = VxGetSettingsDirectory();
	    strDbFileName += "HostUrlList.db3";
	    getHostUrlListMgr().hostUrlListMgrStartup( strDbFileName );

	    strDbFileName = VxGetSettingsDirectory();
	    strDbFileName += "EngineParams.db3";
	    getEngineParams().engineParamsStartup( strDbFileName );

        strDbFileName = VxGetSettingsDirectory();
        strDbFileName += "IgnoredHostsList.db3";
        getIgnoreListMgr().ignoredHostsListMgrStartup( strDbFileName );

	    strDbFileName = VxGetSettingsDirectory();
	    strDbFileName += "HostedList.db3";
	    getHostedListMgr().hostedListMgrStartup( strDbFileName );

        LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiSetUserSpecificDir db startup took %d ms",
                   GetApplicationAliveMs() - startupStartMs );

        // if application was aborted it may have left the listen socket in a state
        // that the port cannot be listened to for incomming connections
        // ANDROID crashes if attempt to close an unowned socket so is excluded
        // instead must restart the android device
        #if !defined( TARGET_OS_ANDROID )
            SOCKET listenSocket = (SOCKET)getEngineParams().getLastListenSocket();
            if( listenSocket > 0 )
            {
                VxCloseSktNow( listenSocket );
                getEngineParams().setLastListenSocket(0);
            }

        #endif //!defined( TARGET_OS_ANDROID )

	    m_IsUserSpecificDirSet = true;
	    m_AppStartupCalled = true;

        LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiSetUserSpecificDir done in %d ms at %d ms",
               GetApplicationAliveMs() - startupStartMs, GetApplicationAliveMs() );
    }
    else
    {
        LogMsg( LOG_VERBOSE, "P2PEngine::fromGuiSetUserSpecificDir queued" );
        m_FromGuiMgr.fromGuiSetUserSpecificDir( userSpecificDir );
    }
}

//============================================================================
void P2PEngine::fromGuiUserLoggedOn( VxNetIdent* netIdent, bool fromThread )
{
    if( fromThread )
    {
        const int startupStartMs = GetApplicationAliveMs();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn begin at %d ms", startupStartMs );
        memcpy( ( VxNetIdent* )&m_PktAnn, netIdent, sizeof( VxNetIdent ) );
        m_PktAnn.setSrcOnlineId( netIdent->getMyOnlineId() );
        m_MyOnlineId = netIdent->getMyOnlineId();

        m_HostJoinMgr.fromGuiUserLoggedOn();
        m_UserJoinMgr.fromGuiUserLoggedOn();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn join managers took %d ms", GetApplicationAliveMs() - startupStartMs );

        // set network settings from saved settings
        startupEngine();
        const int startupEngineDoneMs = GetApplicationAliveMs();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn startupEngine took %d ms", startupEngineDoneMs - startupStartMs );
        //updateFromEngineSettings( getEngineSettings() );
        m_PluginMgr.fromGuiUserLoggedOn();
        const int pluginLogonDoneMs = GetApplicationAliveMs();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn plugin logon took %d ms", pluginLogonDoneMs - startupEngineDoneMs );

        m_AssetMgr.onPluginsInitialized();
        m_OfferMgr.fromGuiUserLoggedOn();
        m_OfferMgr.onPluginsInitialized();
        m_ThumbMgr.onPluginsInitialized();

        m_SendQueueMgr.fromGuiUserLoggedOn();
        const int serviceReadyMs = GetApplicationAliveMs();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn post-plugin init took %d ms", serviceReadyMs - pluginLogonDoneMs );
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn done in %d ms", serviceReadyMs - startupStartMs );
        m_IsEngineReady = true;

        m_PluginMgr.onAfterUserLogOnThreaded();
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn onAfterUserLogOnThreaded complete in %d ms total", GetApplicationAliveMs() - startupStartMs );

        if( m_NetworkConnectionReady && !m_PktMgrNetworkReadyWasCalled )
        {
            // network was ready before plugin manager initialized
            m_PktMgrNetworkReadyWasCalled = true;
            // fire up web cam, file share and other hosted services
            m_PluginMgr.onNetworkConnectionReady( m_PktAnn.requiresRelay() );
        }
    }
    else
    {
        if( LogEnabled( eLogStartup ) ) LogModule( eLogStartup, LOG_VERBOSE, "P2PEngine::fromGuiUserLoggedOn queued" );
        m_FromGuiMgr.fromGuiUserLoggedOn( netIdent );
    }
}

//============================================================================
void P2PEngine::fromGuiAppShutdown( void )
{
    LogMsg( LOG_VERBOSE, " P2PEngine::%s", __func__ );
    VxSetAppIsShuttingDown( true );
    enableTimerThread( false );
    shutdownEngine();
}
