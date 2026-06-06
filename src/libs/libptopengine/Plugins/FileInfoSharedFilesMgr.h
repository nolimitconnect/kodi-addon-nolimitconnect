#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoDb.h"
#include "FileInfoBaseMgr.h"

class FileInfoSharedFilesMgr : public FileInfoDb, public FileInfoBaseMgr
{
public:
	FileInfoSharedFilesMgr() = delete;
	FileInfoSharedFilesMgr( const FileInfoSharedFilesMgr& rhs ) = delete;
	FileInfoSharedFilesMgr( P2PEngine& engine, PluginBase& plugin, std::string sharedFilesDbName );
	virtual ~FileInfoSharedFilesMgr();

	FileInfoSharedFilesMgr& operator=( const FileInfoSharedFilesMgr& rhs ) = delete;
};

