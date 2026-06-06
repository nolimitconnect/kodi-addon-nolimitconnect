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
#include "AssetBaseRxSession.h"

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdio.h>

//============================================================================
AssetBaseRxSession::AssetBaseRxSession( P2PEngine& engine )
: AssetBaseXferSession( engine )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
AssetBaseRxSession::AssetBaseRxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: AssetBaseXferSession( engine, sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
AssetBaseRxSession::AssetBaseRxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId )
: AssetBaseXferSession( engine, lclSessionId, sktBase, sendToId )
{
	getXferInfo().setXferDirection( eXferDirectionRx );
}

//============================================================================
AssetBaseRxSession::~AssetBaseRxSession()
{
}

//============================================================================
void AssetBaseRxSession::cancelDownload( VxGUID& lclSessionId )
{
	VxFileXferInfo& xferInfo = getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
	}

	VxFileUtil::deleteFile( xferInfo.getLclFileNameAndPath().c_str() );
}
