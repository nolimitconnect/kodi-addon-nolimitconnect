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

#include "FileInfoDb.h"
#include "FileInfoXferMgr.h"
#include "FileInfoBaseMgr.h"

#include <PktLib/VxCommon.h>

#include <CoreLib/VxThread.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/Sha1GeneratorCallback.h>

class FileInfo;
class IToGui;
class PluginBase;
class P2PEngine;
class PktFileListReply;
class PktFileInfoSearchReply;
class PktFileInfoMoreReply;
class SearchParams;
class SharedFilesMgr;
class VxSha1Hash;
class VxFileShredder;

class FileInfoClientFilesMgr : public FileInfoDb, public FileInfoBaseMgr
{
public:
	FileInfoClientFilesMgr() = delete;
	FileInfoClientFilesMgr( const FileInfoClientFilesMgr& rhs ) = delete;
	FileInfoClientFilesMgr( P2PEngine& engine, PluginBase& plugin, std::string clientdFilesDbName );
	virtual ~FileInfoClientFilesMgr() = default;

	FileInfoClientFilesMgr& operator=( const FileInfoClientFilesMgr& rhs ) = delete;
	//=== vars ===//
};

