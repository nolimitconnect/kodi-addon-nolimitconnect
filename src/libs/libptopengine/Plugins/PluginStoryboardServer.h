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

class PluginStoryboardServer : public PluginBaseFilesServer
{
public:
	PluginStoryboardServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginStoryboardServer() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

protected:
	virtual void				onAfterUserLogOnThreaded( void ) override;
	virtual void				onLoadedFilesReady( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) override;
	virtual void				onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) override;

	void						setIsWebPageServerReady( bool isReady );
	bool						getIsWebPageServerReady( void ) { return m_WebPageServerReady; }

	void						checkIsWebPageServerReady( void );
	void						onWebPageServerReady( bool isReady );

	std::string					m_RootFileFolder{ "" };
	bool						m_WebPageServerReady{ false };
};



