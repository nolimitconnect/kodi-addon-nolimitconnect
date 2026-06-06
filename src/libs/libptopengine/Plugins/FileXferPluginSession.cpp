//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileXferPluginSession.h"
#include "PluginBase.h"

#include <CoreLib/VirtFileMgr.h>
#include <PktLib/PktsFileShare.h>

#include <stdio.h>

//============================================================================
FileXferPluginSession::FileXferPluginSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: P2PSession( sktBase, sendToId, pluginType )
, m_Error( 0 )
{
}

//============================================================================
FileXferPluginSession::FileXferPluginSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: P2PSession( lclSessionId, sktBase, sendToId, pluginType )
, m_Error( 0 )
{
}

//============================================================================
FileXferPluginSession::~FileXferPluginSession()
{
}

//============================================================================
//! return true if file is already in que to be sent or has been sent
bool FileXferPluginSession::isFileQuedOrSent( const char* pFileName )
{
	return false;
}

//============================================================================
void FileXferPluginSession::cancelUpload( PluginBase& pluginBase, VxGUID& lclSessionId )
{
	PktFileChunkReq oPkt;
	oPkt.setDataLen( 0 );
	oPkt.setError( eXferErrorCanceled );
	oPkt.setLclSessionId( lclSessionId );
	oPkt.setRmtSessionId( m_RmtSessionId );
	pluginBase.txPacket( getSendToId(), getSkt(), &oPkt);

	if( m_TxFileInfo.m_hFile )
	{
		VFileClose( m_TxFileInfo.m_hFile );
	}

	std::vector<FileToXfer>::iterator iter;
	for( iter = m_astrFilesToSend.begin(); iter != m_astrFilesToSend.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesToSend.erase(iter);
			break;
		}
	}

	for( iter = m_astrFilesSent.begin(); iter != m_astrFilesSent.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesSent.erase(iter);
			break;
		}
	}	

	for( iter = m_astrFilesReceived.begin(); iter != m_astrFilesReceived.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesReceived.erase(iter);
			break;
		}
	}	
}

//============================================================================
void FileXferPluginSession::cancelDnload( PluginBase& pluginBase, VxGUID& lclSessionId )
{
	PktFileChunkReply oPkt;
	oPkt.setDataLen( 0 );
	oPkt.setError( eXferErrorCanceled );
	oPkt.setLclSessionId( lclSessionId );
	oPkt.setRmtSessionId( m_RmtSessionId );
	pluginBase.txPacket( getSendToId(), getSkt(), &oPkt );

	if( m_RxFileInfo.m_hFile )
	{
		VFileClose( m_RxFileInfo.m_hFile );
	}

	std::vector<FileToXfer>::iterator iter;
	for( iter = m_astrFilesToSend.begin(); iter != m_astrFilesToSend.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesToSend.erase(iter);
			break;
		}
	}

	for( iter = m_astrFilesSent.begin(); iter != m_astrFilesSent.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesSent.erase(iter);
			break;
		}
	}	

	for( iter = m_astrFilesReceived.begin(); iter != m_astrFilesReceived.end(); ++iter )
	{
		if( (*iter).getLclSessionId() == lclSessionId )
		{
			m_astrFilesReceived.erase(iter);
			break;
		}
	}	
}
