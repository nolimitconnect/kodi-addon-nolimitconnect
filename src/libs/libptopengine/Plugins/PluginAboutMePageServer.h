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

class PluginAboutMePageServer : public PluginBaseFilesServer
{
public:
	PluginAboutMePageServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginAboutMePageServer() = default;

	EMediaModule					getMediaModule( void ) override { return eMediaModuleInvalid; }

    void						onNetworkConnectionReady( bool requiresRelay ) override;

protected:
	virtual void				onAfterUserLogOnThreaded( void ) override;
	virtual void				onLoadedFilesReady( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) override;
	virtual void				onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes ) override;

	void						setIsWebPageServerReady( bool isReady );
	bool						getIsWebPageServerReady( void ) { return m_WebPageServerReady; }

	void						checkIsWebPageServerReady( void );
	void						onWebPageServerReady( bool isReady );

	void						updateHasProfilePictureChanged( void );

	std::string					m_RootFileFolder{""};
	bool						m_WebPageServerReady{ false };
};



