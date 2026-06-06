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

#include "PluginBaseFilesServer.h"

class AssetBaseInfo;

class PluginFileShareServer : public PluginBaseFilesServer
{
public:
	PluginFileShareServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* netIdent, EPluginType pluginType );
	virtual ~PluginFileShareServer() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

	virtual void				onNetworkConnectionReady( bool requiresRelay ) override;

	virtual void				updateSharedFilesInfo( void );

	void						deleteFile( std::string fileNameAndPath, bool shredFile ) override;

	void						onPktStreamCtrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	void						fileAboutToBeDeleted( std::string fileNameAndPath );

	void						fileShareEnable( AssetBaseInfo* assetInfo, bool shareFile );

protected:
	virtual void				onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) override;
};


