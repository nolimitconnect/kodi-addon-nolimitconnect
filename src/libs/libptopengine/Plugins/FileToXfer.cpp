//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileToXfer.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
FileToXfer::FileToXfer( FileInfo& fileInfo, VxGUID& lclSessionId, VxGUID& rmtSessionId )
: FileInfo( fileInfo )
, m_LclSessionId( lclSessionId )
, m_RmtSessionId( rmtSessionId )
{
}

//============================================================================
FileToXfer::FileToXfer( const FileToXfer& rhs )
    : FileInfo( rhs )
    , m_LclSessionId( rhs.m_LclSessionId )
    , m_RmtSessionId( rhs.m_RmtSessionId )
    , m_FileOffset( rhs.m_FileOffset )
{

}

//============================================================================
FileToXfer& FileToXfer::operator=(const FileToXfer& rhs) 
{	
	if( &rhs != this )
	{
		*((FileInfo*)this) = rhs;

		m_LclSessionId = rhs.m_LclSessionId;
		m_RmtSessionId = rhs.m_RmtSessionId;
		m_FileOffset = rhs.m_FileOffset;
	}

	return *this;
}

//============================================================================
bool FileToXfer::fillFileXferInfo( VxFileXferInfo& xferInfo, EXferDirection dir )
{
	xferInfo.clear();
	xferInfo.setXferDirection( dir );
	xferInfo.setLclSessionId( m_LclSessionId );
	xferInfo.setRmtSessionId( m_RmtSessionId );

	xferInfo.setAssetId( getAssetId() );
	xferInfo.setFileHashId( getFileHashId() );
	xferInfo.setFileLength( getFileLength() );

	xferInfo.setRmtFileName( getFileName().c_str());
	xferInfo.setLclFileName( getFileName().c_str() );
	if( dir == eXferDirectionTx && !VxFileUtil::fileExists( getFileNameAndPath().c_str() ) )
	{
		LogMsg( LOG_ERROR, "FileToXfer::fillFileXferInfo file no longer exists %s", getFileNameAndPath().c_str() );
		return false;
	}

	xferInfo.setLclFileNameAndPath( getFileNameAndPath().c_str() );
	return true;
}