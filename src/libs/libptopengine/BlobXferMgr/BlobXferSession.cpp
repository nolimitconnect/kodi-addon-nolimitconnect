//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "BlobXferSession.h"
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxGlobals.h>

//============================================================================
BlobXferSession::BlobXferSession( P2PEngine& engine )
: m_Engine( engine )
{
	initLclSessionId();
}

//============================================================================
BlobXferSession::BlobXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: m_Engine( engine )
, m_Skt( sktBase )
, m_Ident( netIdent )
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
BlobXferSession::BlobXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: m_Engine( engine )
, m_FileXferInfo( lclSessionId )
, m_Skt( sktBase )
, m_Ident( netIdent )
{
	initLclSessionId();
}

//============================================================================
void BlobXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void BlobXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool BlobXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}

	return false;
}

//============================================================================
void BlobXferSession::setBlobStateSendBegin( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxBegin, m_BlobInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionTxBegin, m_BlobInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void BlobXferSession::setBlobStateSendCanceled( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxCancel, m_BlobInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionTxCancel, m_BlobInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void BlobXferSession::setBlobStateSendFail( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxError, m_BlobInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionTxError, m_BlobInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void BlobXferSession::setBlobStateSendProgress( int progress )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxProgress, m_BlobInfo.getAssetUniqueId(), progress );
	}
	else
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionTxProgress, m_BlobInfo.getAssetUniqueId(), progress );
	}
}

//============================================================================
void BlobXferSession::setBlobStateSendSuccess( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxSuccess, m_BlobInfo.getAssetUniqueId(), 100 );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionRxNotifyNewMsg, m_BlobInfo.getCreatorId(), 100 );
	}
	else
	{
		m_BlobInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiBlobAction( eAssetActionTxSuccess, m_BlobInfo.getAssetUniqueId(), 100 );
	}
}
