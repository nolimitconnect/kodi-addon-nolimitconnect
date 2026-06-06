//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferBaseXferSession.h"

#include <OfferBase/OfferMgr.h>

//============================================================================
OfferBaseXferSession::OfferBaseXferSession( P2PEngine& engine, OfferBaseMgr& offerMgr )
: m_Engine( engine )
, m_OfferMgr( offerMgr )
, m_FileXferInfo()
, m_iPercentComplete(0)
, m_Skt(nullptr)
, m_SendToId()
, m_Error( 0 )
{
	initLclSessionId();
}

//============================================================================
OfferBaseXferSession::OfferBaseXferSession( P2PEngine& engine, OfferBaseMgr& offerMgr, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_Engine( engine )
, m_OfferMgr( offerMgr )
, m_FileXferInfo()
, m_Skt( sktBase )
, m_SendToId( sendToId )
{
	initLclSessionId();
}

//============================================================================
OfferBaseXferSession::OfferBaseXferSession( P2PEngine& engine, OfferBaseMgr& offerMgr, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: m_Engine( engine )
, m_OfferMgr( offerMgr )
, m_FileXferInfo( lclSessionId )
, m_Skt( sktBase )
, m_SendToId( sendToId )
{
	initLclSessionId();
}

//============================================================================
void OfferBaseXferSession::reset( void )
{
	m_iPercentComplete = 0;
}

//============================================================================
void OfferBaseXferSession::initLclSessionId( void )
{
	if( false == m_FileXferInfo.getLclSessionId().isValid() )
	{
		m_FileXferInfo.getLclSessionId().initializeWithNewVxGUID();
	}
}

//============================================================================
bool OfferBaseXferSession::isXferingFile( void )
{
	if( m_FileXferInfo.m_hFile )
	{
		return true;
	}

	return false;
}

//============================================================================
void OfferBaseXferSession::setOfferBaseStateSendBegin( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxProgress );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionRxBegin, 0 );
	}
	else
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateTxProgress );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionTxBegin, 0 );
	}
}

//============================================================================
void OfferBaseXferSession::setOfferBaseStateSendCanceled( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxFail );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionRxCancel, 0 );
	}
	else
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateTxFail );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionTxCancel, 0 );
	}
}

//============================================================================
void OfferBaseXferSession::setOfferBaseStateSendFail( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxFail );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionRxError, 0 );
	}
	else
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateTxFail );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionTxError, 0 );
	}
}

//============================================================================
void OfferBaseXferSession::setOfferBaseStateSendProgress( int progress )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxProgress );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionRxProgress, progress );
	}
	else
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateTxProgress );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionTxProgress, progress );
	}
}

//============================================================================
void OfferBaseXferSession::setOfferBaseStateSendSuccess( void )
{
	if( eXferDirectionRx == getXferDirection() )
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxSuccess );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionRxSuccess, 100 );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getCreatorId(), eOfferActionRxNotifyNewMsg, 100 );
	}
	else
	{
		m_OfferBaseInfo.setOfferSendState( eOfferSendStateRxSuccess );
		m_OfferMgr.announceOfferAction( m_OfferBaseInfo.getOfferId(), eOfferActionTxSuccess, 100 );
	}
}
