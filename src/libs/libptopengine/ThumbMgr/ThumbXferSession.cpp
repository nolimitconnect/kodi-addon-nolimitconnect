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
#include "ThumbXferSession.h"
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxGlobals.h>

//============================================================================
ThumbXferSession::ThumbXferSession( P2PEngine& engine )
: AssetBaseXferSession( engine )
{
	initLclSessionId();
}

//============================================================================
ThumbXferSession::ThumbXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId )
: AssetBaseXferSession( engine, sktBase, sendToId )
{
	initLclSessionId();
}

//============================================================================
ThumbXferSession::ThumbXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId )
: AssetBaseXferSession( engine, sktBase, sendToId )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.setLclSessionId( lclSessionId );
	}
}

//============================================================================
ThumbXferSession::~ThumbXferSession()
{
}

//============================================================================
void ThumbXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void ThumbXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool ThumbXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}

	return false;
}

//============================================================================
void ThumbXferSession::setThumbStateSendBegin( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxBegin, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxBegin, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void ThumbXferSession::setThumbStateSendCanceled( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxCancel, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxCancel, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void ThumbXferSession::setThumbStateSendFail( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxError, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxError, m_ThumbInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void ThumbXferSession::setThumbStateSendProgress( int progress )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxProgress, m_ThumbInfo.getAssetUniqueId(), progress );
	}
	else
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxProgress, m_ThumbInfo.getAssetUniqueId(), progress );
	}
}

//============================================================================
void ThumbXferSession::setThumbStateSendSuccess( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxSuccess, m_ThumbInfo.getAssetUniqueId(), 100 );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxNotifyNewMsg, m_ThumbInfo.getCreatorId(), 100 );
	}
	else
	{
		m_ThumbInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxSuccess, m_ThumbInfo.getAssetUniqueId(), 100 );
	}
}
