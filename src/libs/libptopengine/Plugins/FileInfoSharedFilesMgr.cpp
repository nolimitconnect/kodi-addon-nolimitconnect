//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoSharedFilesMgr.h"

#include "PluginBase.h"

#include <CoreLib/VxDebug.h>

//============================================================================
FileInfoSharedFilesMgr::FileInfoSharedFilesMgr( P2PEngine& engine, PluginBase& plugin, std::string sharedFilesDbName )
	: FileInfoDb( sharedFilesDbName )
	, FileInfoBaseMgr( engine, plugin, *this )
{
	if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "FileInfoSharedFilesMgr::FileInfoSharedFilesMgr %s %p", DescribePluginType( plugin.getPluginType() ), this );
}

//============================================================================
FileInfoSharedFilesMgr::~FileInfoSharedFilesMgr()
{
	fileInfoMgrShutdown();
}
