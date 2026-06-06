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
#include "AssetBaseTxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
AssetBaseTxSession::AssetBaseTxSession( P2PEngine& engine )
: AssetBaseXferSession( engine )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
AssetBaseTxSession::AssetBaseTxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: AssetBaseXferSession( engine, sktBase, sendToId )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
AssetBaseTxSession::AssetBaseTxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: AssetBaseXferSession( engine, lclSessionId, sktBase, sendToId )
{
	setXferDirection( eXferDirectionTx );
}

//============================================================================
void AssetBaseTxSession::reset( void )
{
	AssetBaseXferSession::reset();
	m_iOutstandingAckCnt = 0;
	m_bSendingPkts = false;
	m_strOfferFile.clear();
	m_strViewDirectory.clear();
}

//============================================================================
void AssetBaseTxSession::cancelUpload( VxGUID& lclSessionId )
{
	if( m_FileXferInfo.m_hFile )
	{
		VFileClose( m_FileXferInfo.m_hFile );
		m_FileXferInfo.m_hFile = nullptr;
	}

	setAssetBaseStateSendFail();
}
