//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginStoryboardServer.h"
#include "PluginMgr.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

//============================================================================
PluginStoryboardServer::PluginStoryboardServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
	: PluginBaseFilesServer( engine, pluginMgr, myIdent, pluginType, "StoryboardFilesServer.db3" )
{
	setPluginType( ePluginTypeStoryboardServer );
}

//============================================================================
void PluginStoryboardServer::onAfterUserLogOnThreaded( void )
{
	m_RootFileFolder = VxGetAppDirectory( eAppDirStoryboardPageServer );
	getFileInfoMgr().setRootFolder( m_RootFileFolder );

	getFileInfoMgr().onAfterUserLogOnThreaded();
}

//============================================================================
void PluginStoryboardServer::onLoadedFilesReady( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes )
{
	if( !getFileInfoMgr().loadStoryboardPageFileAssets() )
	{
		LogMsg( LOG_ERROR, "PluginAboutMePageServer::onLoadedFilesReady failed or missing web files" );
	}
	else
	{
		setIsWebPageServerReady( true );
	}
}

//============================================================================
void PluginStoryboardServer::onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes )
{
	checkIsWebPageServerReady();
}

//============================================================================
void PluginStoryboardServer::checkIsWebPageServerReady( void )
{
}

//============================================================================
void PluginStoryboardServer::setIsWebPageServerReady( bool isReady )
{
	if( m_WebPageServerReady != isReady )
	{
		m_WebPageServerReady = isReady;
		onWebPageServerReady( isReady );
	}
}

//============================================================================
void PluginStoryboardServer::onWebPageServerReady( bool isReady )
{
	m_WebPageServerReady = isReady;
	// do stuff
}
