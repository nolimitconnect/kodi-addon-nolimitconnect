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

#include "PluginBaseFiles.h"
#include "FileInfoClientFilesMgr.h"

class PluginBaseFilesClient : public PluginBaseFiles
{
public:
	enum EFileClientState
	{
		eFileClientStateInitializing,
		eFileClientStateIdle,
		eFileClientStateRetrieveInfo,
		eFileClientStateDownloading,
		eFileClientStateDownloadComplete,
		eFileClientStateCommError,

		eMaxFileClientState
	};

	PluginBaseFilesClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, std::string fileInfoDbName );
	virtual ~PluginBaseFilesClient() = default;

	EMediaModule				getMediaModule( void ) override { return eMediaModuleInvalid; }

	bool						connectForWebPageDownload( VxGUID& onlineId );
	bool						onConnectForWebPageDownload( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	virtual bool				connectForFileListDownload( VxGUID& onlineId );
	virtual bool				onConnectForFileListDownload( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	void						setSearchFileTypes( uint8_t fileTypes )				{ m_FileTypes = fileTypes; }
	uint8_t						getSearchFileTypes( void )							{ return m_FileTypes; }

protected:
	
	FileInfoClientFilesMgr      m_FileInfoClientFilesMgr;

	VxGUID						m_HisOnlineId;
	VxGUID						m_SearchSessionId;
	VxGUID						m_LclSessionId;
	EFileClientState			m_FileClientState{ eFileClientStateInitializing };
	uint8_t						m_FileTypes{ 0 };
	VxGUID						m_SktConnectionId;
};


