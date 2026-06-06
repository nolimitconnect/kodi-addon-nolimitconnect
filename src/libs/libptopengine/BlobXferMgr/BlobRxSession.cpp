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
#include "BlobRxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
BlobRxSession::BlobRxSession( P2PEngine& engine )
: BlobXferSession( engine )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
BlobRxSession::BlobRxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: BlobXferSession( engine, sktBase, netIdent )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
BlobRxSession::BlobRxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
: BlobXferSession( engine, lclSessionId, sktBase, netIdent )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
BlobRxSession::~BlobRxSession()
{
}

//============================================================================
void BlobRxSession::cancelDownload( VxGUID& lclSessionId )
{
	VxFileXferInfo& xferInfo = getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
	}

	VxFileUtil::deleteFile( xferInfo.getLclFileNameAndPath().c_str() );

}
