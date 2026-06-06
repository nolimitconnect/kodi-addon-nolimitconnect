//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferBaseRxSession.h"

#include <OfferBase/OfferMgr.h>

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
OfferBaseRxSession::OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr )
: OfferBaseXferSession( engine, offerMgr )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
OfferBaseRxSession::OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: OfferBaseXferSession( engine, offerMgr, sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
OfferBaseRxSession::OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: OfferBaseXferSession( engine, offerMgr, lclSessionId, sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
void OfferBaseRxSession::cancelDownload( VxGUID& lclSessionId )
{
	VxFileXferInfo& xferInfo = getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
	}

	VxFileUtil::deleteFile( xferInfo.getLclFileNameAndPath().c_str() );

}
