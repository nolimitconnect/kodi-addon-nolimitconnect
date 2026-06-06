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
#include "ThumbTxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
ThumbTxSession::ThumbTxSession( P2PEngine& engine )
: ThumbXferSession( engine )
, m_iOutstandingAckCnt( 0 )
, m_bSendingPkts( false )
, m_bViewingFileList( false )
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
ThumbTxSession::ThumbTxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: ThumbXferSession( engine, sktBase, sendToId )
, m_iOutstandingAckCnt( 0 )
, m_bSendingPkts( false )
, m_bViewingFileList( false )
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
ThumbTxSession::ThumbTxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: ThumbXferSession( engine, lclSessionId, sktBase, sendToId )
, m_iOutstandingAckCnt(0)
, m_bSendingPkts(false)
, m_bViewingFileList(false)
, m_QuePosition( 0 )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
void ThumbTxSession::reset( void )
{
	ThumbXferSession::reset();
	m_iOutstandingAckCnt = 0;
	m_bSendingPkts = false;
	m_bViewingFileList = false;
	m_strOfferFile = "";
	m_strViewDirectory = "";
}

//============================================================================
void ThumbTxSession::cancelUpload( VxGUID& lclSessionId )
{
	if( m_FileXferInfo.m_hFile )
	{
		VFileClose( m_FileXferInfo.m_hFile );
		m_FileXferInfo.m_hFile = nullptr;
	}

	setThumbStateSendFail();
}
