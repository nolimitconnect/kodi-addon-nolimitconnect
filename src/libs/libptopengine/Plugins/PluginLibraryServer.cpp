//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginLibraryServer.h"
#include "PluginMgr.h"

#include <Plugins/FileInfo.h>
#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginFileShareServer.h>

#include <PktLib/PktsFileShare.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/VxSearchDefs.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxGlobals.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

//============================================================================
PluginLibraryServer::PluginLibraryServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_AssetMgr( engine.getAssetMgr() )
{
	LogMsg( LOG_VERBOSE, "PluginLibraryServer::PluginLibraryServer" );
	setPluginType( ePluginTypeLibraryServer );
}

//============================================================================
bool PluginLibraryServer::fromGuiSetFileIsInLibrary( FileInfo& fileInfo, bool inLibrary )
{
	return m_AssetMgr.fromGuiSetFileIsInLibrary( fileInfo, inLibrary );
}

//============================================================================
bool PluginLibraryServer::fromGuiSetFileIsInLibrary( std::string& fileNameAndPath, bool inLibrary )
{
	return m_AssetMgr.fromGuiSetFileIsInLibrary( fileNameAndPath, inLibrary );
}

//============================================================================
bool PluginLibraryServer::fromGuiGetFileIsInLibrary( FileInfo& fileInfo )
{
	return isFileInLibrary( fileInfo.getFileNameAndPath() );
}

//============================================================================
bool PluginLibraryServer::fromGuiGetFileIsInLibrary( std::string& fileNameAndPath )
{
	return isFileInLibrary( fileNameAndPath );
}

//============================================================================
bool PluginLibraryServer::isFileInLibrary( std::string& fileNameAndPath )
{
	AssetBaseInfo* assetInfo = m_AssetMgr.findAsset( fileNameAndPath );
	if( assetInfo )
	{
		return assetInfo->isInLibrary();
	}

	return false;
}

//============================================================================
void PluginLibraryServer::fromGuiGetFileLibraryList( VxGUID& appInstId, uint8_t fileTypeFilter )
{
	m_AssetMgr.fromGuiSendFileList( appInstId, fileTypeFilter, true, true );
}

//============================================================================
void PluginLibraryServer::deleteFile( std::string fileNameAndPath, bool shredFile )
{
	m_AssetMgr.deleteFile( fileNameAndPath, shredFile );
}
