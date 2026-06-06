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
#include <PktLib/PktsFileInfo.h>

//============================================================================
PluginBaseFilesServer::PluginBaseFilesServer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, std::string fileInfoDbName )
    : PluginBaseFiles( engine, pluginMgr, myIdent, pluginType, m_FileInfoSharedFilesMgr )
    , m_FileInfoSharedFilesMgr( engine, *this, fileInfoDbName )
{
}
