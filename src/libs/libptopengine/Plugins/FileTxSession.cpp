//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileTxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
FileTxSession::FileTxSession()
: FileShareXferSession()
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
FileTxSession::FileTxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: FileShareXferSession( sktBase, sendToId )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
FileTxSession::FileTxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: FileShareXferSession( lclSessionId, sktBase, sendToId )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
void FileTxSession::reset( void )
{
	FileShareXferSession::reset();
	m_strViewDirectory.clear();
}

//============================================================================
void FileTxSession::cancelUpload( VxGUID& lclSessionId )
{
	if( m_FileXferInfo.m_hFile )
	{
		VFileClose( m_FileXferInfo.m_hFile );
		m_FileXferInfo.m_hFile = nullptr;
	}

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
