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

#include "PluginBaseFilesServer.h"

class PluginLibraryServer : public PluginBase
{
public:
	PluginLibraryServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* netIdent, EPluginType pluginType );
	virtual ~PluginLibraryServer() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

	void						onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override {};
	void						onConnectionLost(std::shared_ptr<VxSktBase> &) override {};
	void						onContactOnlineStatusChange( ConnectId&, bool ) override {};
	void						replaceConnection(VxNetIdent *,std::shared_ptr<VxSktBase> &,std::shared_ptr<VxSktBase> &) override {};	

	virtual bool				fromGuiSetFileIsInLibrary( FileInfo& fileInfo, bool inLibrary );
	virtual bool				fromGuiSetFileIsInLibrary( std::string& fileNameAndPath, bool inLibrary );

	virtual bool				fromGuiGetFileIsInLibrary( FileInfo& fileInfo );
	virtual bool				fromGuiGetFileIsInLibrary( std::string& fileNameAndPath );

	virtual void				fromGuiGetFileLibraryList( VxGUID& appInstId, uint8_t fileTypeFilter );

	bool						isFileInLibrary( std::string& fileNameAndPath );

	void						deleteFile( std::string fileNameAndPath, bool shredFile );

protected:

	AssetMgr&					m_AssetMgr;
};


