//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileShareXferSession.h"

#include "OfferBase/OfferBaseInfo.h"

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsFileShare.h>

#include <stdio.h>

//============================================================================
FileShareXferSession::FileShareXferSession()
: m_FilesToXferList()
, m_FilesXferedList()
, m_FileXferInfo()
, m_iPercentComplete(0)
, m_Skt(nullptr)
, m_SendToId()
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
FileShareXferSession::FileShareXferSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_FilesToXferList()
, m_FilesXferedList()
, m_FileXferInfo()
, m_iPercentComplete(0)
, m_Skt( sktBase )
, m_SendToId( sendToId )
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
FileShareXferSession::FileShareXferSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_FilesToXferList()
, m_FilesXferedList()
, m_FileXferInfo( lclSessionId )
, m_iPercentComplete(0)
, m_Skt( sktBase )
, m_SendToId( sendToId )
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
void FileShareXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void FileShareXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool FileShareXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}
	return false;
}

//============================================================================
bool FileShareXferSession::offerAccepted( VxGUID& lclSessionId )
{
	bool nextFileAvail{ false };
	if( !m_FileXferInfo.m_hFile )
	{
		for( auto iter = m_FilesToXferList.begin(); iter != m_FilesToXferList.end(); ++iter )
		{
			if( ( *iter ).getLclSessionId() == lclSessionId )
			{
				nextFileAvail  = fillXferFile( *iter );
				m_FilesToXferList.erase( iter );
				break;
			}
		}
	}

	return nextFileAvail;
}

//============================================================================
bool FileShareXferSession::setupNextFile( void )
{
	bool nextFileAvail{ false };
	if( m_FilesToXferList.size() )
	{
		nextFileAvail = fillXferFile( m_FilesToXferList[0] );
		m_FilesToXferList.erase( m_FilesToXferList.begin() );
	}

	return nextFileAvail;
}

//============================================================================
bool FileShareXferSession::fillXferFile( FileToXfer& fileToXfer )
{
	bool nextFileAvail{ false };
	if( !m_FileXferInfo.m_hFile )
	{

		nextFileAvail = fileToXfer.fillFileXferInfo( m_FileXferInfo, getXferDirection() );
		if( nextFileAvail )
		{
		}

	}

	return nextFileAvail;
}
