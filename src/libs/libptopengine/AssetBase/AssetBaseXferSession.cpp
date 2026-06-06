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
#include "AssetBaseXferSession.h"
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxGlobals.h>

//============================================================================
AssetBaseXferSession::AssetBaseXferSession( P2PEngine& engine )
: m_Engine( engine )
, m_FileXferInfo()
, m_iPercentComplete(0)
, m_Skt(nullptr)
, m_SendToId()
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
AssetBaseXferSession::AssetBaseXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_Engine( engine )
, m_FileXferInfo()
, m_iPercentComplete(0)
, m_Skt( sktBase )
, m_SendToId( sendToId )
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
AssetBaseXferSession::AssetBaseXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_Engine( engine )
, m_FileXferInfo( lclSessionId )
, m_iPercentComplete(0)
, m_Skt( sktBase )
, m_SendToId( sendToId )
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
AssetBaseXferSession::~AssetBaseXferSession()
{
}

//============================================================================
void AssetBaseXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void AssetBaseXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool AssetBaseXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}

	return false;
}

//============================================================================
void AssetBaseXferSession::setAssetBaseStateSendBegin( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxBegin, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxBegin, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetBaseXferSession::setAssetBaseStateSendCanceled( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxCancel, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxCancel, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetBaseXferSession::setAssetBaseStateSendFail( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxError, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
	else
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateTxFail );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxError, m_AssetBaseInfo.getAssetUniqueId(), 0 );
	}
}

//============================================================================
void AssetBaseXferSession::setAssetBaseStateSendProgress( int progress )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxProgress, m_AssetBaseInfo.getAssetUniqueId(), progress );
	}
	else
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateTxProgress );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxProgress, m_AssetBaseInfo.getAssetUniqueId(), progress );
	}
}

//============================================================================
void AssetBaseXferSession::setAssetBaseStateSendSuccess( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxSuccess, m_AssetBaseInfo.getAssetUniqueId(), 100 );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionRxNotifyNewMsg, m_AssetBaseInfo.getCreatorId(), 100 );
	}
	else
	{
		m_AssetBaseInfo.setAssetSendState( eAssetSendStateRxSuccess );
		m_Engine.getToGui().toGuiAssetAction( eAssetActionTxSuccess, m_AssetBaseInfo.getAssetUniqueId(), 100 );
	}
}
