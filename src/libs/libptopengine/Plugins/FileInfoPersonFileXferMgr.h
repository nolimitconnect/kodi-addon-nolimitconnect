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

class FileInfo;
class PluginBase;
class P2PEngine;

class FileInfoPersonFileXferMgr : public FileInfoDb, public FileInfoBaseMgr
{
public:
	FileInfoPersonFileXferMgr() = delete;
	FileInfoPersonFileXferMgr( const FileInfoPersonFileXferMgr& rhs ) = delete;
	FileInfoPersonFileXferMgr( P2PEngine& engine, PluginBase& plugin, std::string sharedFilesDbName );
	virtual ~FileInfoPersonFileXferMgr();

	FileInfoPersonFileXferMgr& operator=( const FileInfoPersonFileXferMgr& rhs ) = delete;
};

