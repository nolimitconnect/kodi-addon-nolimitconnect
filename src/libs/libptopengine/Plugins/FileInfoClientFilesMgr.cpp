//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoClientFilesMgr.h"

#include "PluginBase.h"
#include <CoreLib/VxDebug.h>
#include <Plugins/FileInfo.h>

//============================================================================
FileInfoClientFilesMgr::FileInfoClientFilesMgr( P2PEngine& engine, PluginBase& plugin, std::string clientdFilesDbName )
	: FileInfoDb( clientdFilesDbName )
	, FileInfoBaseMgr( engine, plugin, *this )
{
	if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "FileInfoClientFilesMgr::FileInfoClientFilesMgr %s %p", DescribePluginType( plugin.getPluginType() ), this );
}
