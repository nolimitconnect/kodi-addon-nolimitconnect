//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileRxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
FileRxSession::FileRxSession()
: FileShareXferSession()
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
FileRxSession::FileRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: FileShareXferSession( sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
FileRxSession::FileRxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: FileShareXferSession( lclSessionId, sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
void FileRxSession::cancelDownload( VxGUID& lclSessionId )
{
	VxFileXferInfo& xferInfo = getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = nullptr;
	}

	VxFileUtil::deleteFile( xferInfo.getLclFileNameAndPath().c_str() );

	for( auto iter = m_FilesToXferList.begin(); iter != m_FilesToXferList.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_FilesToXferList.erase(iter);
			break;
		}
	}

	for( auto iter = m_FilesXferedList.begin(); iter != m_FilesXferedList.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_FilesToXferList.erase(iter);
			break;
		}
	}
}
