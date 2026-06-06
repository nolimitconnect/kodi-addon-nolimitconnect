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
#include "AssetXferSession.h"
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxGlobals.h>

//============================================================================
AssetXferSession::AssetXferSession( P2PEngine& engine )
: m_Engine( engine )
, m_FileXferInfo()
{
	initLclSessionId();
}

//============================================================================
AssetXferSession::AssetXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: m_Engine( engine )
, m_FileXferInfo()
, m_Skt( sktBase )
, m_Ident( netIdent )
{
	initLclSessionId();
}

//============================================================================
AssetXferSession::AssetXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: m_Engine( engine )
, m_FileXferInfo( lclSessionId )
, m_Skt( sktBase )
, m_Ident( netIdent )
{
	initLclSessionId();
}

//============================================================================
AssetXferSession::~AssetXferSession()
{
}

//============================================================================
void AssetXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void AssetXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool AssetXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}

	return false;
}

//============================================================================
void AssetXferSession::setAssetStateSendBegin( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxBegin, m_AssetInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxBegin, m_AssetInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetXferSession::setAssetStateSendCanceled( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxCancel, m_AssetInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxCancel, m_AssetInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetXferSession::setAssetStateSendFail( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxError, m_AssetInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxError, m_AssetInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetXferSession::setAssetStateSendProgress( int progress )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxProgress, m_AssetInfo.getAssetUniqueId(), progress );
	}
	else
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxProgress, m_AssetInfo.getAssetUniqueId(), progress );
	}
}

//============================================================================
void AssetXferSession::setAssetStateSendSuccess( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxSuccess, m_AssetInfo.getAssetUniqueId(), 100 );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxNotifyNewMsg, m_AssetInfo.getCreatorId(), 100 );
	}
	else
	{
		m_AssetInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxSuccess, m_AssetInfo.getAssetUniqueId(), 100 );
	}
}
