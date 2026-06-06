//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferBaseTxSession.h"

#include <OfferBase/OfferMgr.h>

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
OfferBaseTxSession::OfferBaseTxSession( P2PEngine& engine, OfferBaseMgr& offerMgr )
: OfferBaseXferSession( engine, offerMgr )
, m_iOutstandingAckCnt( 0 )
, m_bSendingPkts( false )
, m_bViewingFileList( false )
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
OfferBaseTxSession::OfferBaseTxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: OfferBaseXferSession( engine, offerMgr, sktBase, sendToId )
, m_iOutstandingAckCnt( 0 )
, m_bSendingPkts( false )
, m_bViewingFileList( false )
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
OfferBaseTxSession::OfferBaseTxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: OfferBaseXferSession( engine, offerMgr, lclSessionId, sktBase, sendToId )
, m_iOutstandingAckCnt(0)
, m_bSendingPkts(false)
, m_bViewingFileList(false)
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
void OfferBaseTxSession::reset( void )
{
	OfferBaseXferSession::reset();
	m_iOutstandingAckCnt = 0;
	m_bSendingPkts = false;
	m_bViewingFileList = false;
	m_strOfferFile = "";
	m_strViewDirectory = "";
}

//============================================================================
void OfferBaseTxSession::cancelUpload( VxGUID& lclSessionId )
{
	if( m_FileXferInfo.m_hFile )
	{
		VFileClose( m_FileXferInfo.m_hFile );
		m_FileXferInfo.m_hFile = nullptr;
	}

	setOfferBaseStateSendFail();
}
