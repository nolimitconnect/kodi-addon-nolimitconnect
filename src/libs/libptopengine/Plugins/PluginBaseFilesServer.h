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
#include "FileInfoSharedFilesMgr.h"

class PluginBaseFilesServer : public PluginBaseFiles
{
public:
	PluginBaseFilesServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, std::string fileInfoDbName );
	virtual ~PluginBaseFilesServer() = default;

	EMediaModule				getMediaModule( void ) override { return eMediaModuleInvalid; }

protected:
	FileInfoSharedFilesMgr		m_FileInfoSharedFilesMgr;
};


