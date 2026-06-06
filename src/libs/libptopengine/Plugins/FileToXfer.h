#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfo.h"

#include <NetLib/VxFileXferInfo.h>

class FileToXfer : public FileInfo
{
public:
	FileToXfer( FileInfo& fileInfo, VxGUID&	lclSessionId, VxGUID& rmtSessionId );				
    FileToXfer( const FileToXfer& fileXferInfo );

	FileToXfer& operator=( const FileToXfer& fileXferInfo );

	void						setLclSessionId( VxGUID& lclId )			{ m_LclSessionId = lclId; }
	VxGUID&						getLclSessionId( void )						{ return m_LclSessionId; }
	void						setRmtSessionId( VxGUID& rmtId )			{ m_RmtSessionId = rmtId; }
	VxGUID&						getRmtSessionId( void )						{ return m_RmtSessionId; }

	void						setFileOffset( uint64_t fileOffset )		{ m_FileOffset = fileOffset; }
	uint64_t					getFileOffset( void )						{ return m_FileOffset; }

	bool						fillFileXferInfo( VxFileXferInfo& xferInfo, EXferDirection dir );

protected:
	//=== vars ===//
	VxGUID						m_LclSessionId;
	VxGUID						m_RmtSessionId;
	uint64_t					m_FileOffset{ 0 };
};
