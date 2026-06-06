#pragma once
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

class AssetBaseRxSession : public AssetBaseXferSession
{
public:
	AssetBaseRxSession( P2PEngine& engine );
	AssetBaseRxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	AssetBaseRxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	virtual ~AssetBaseRxSession();

	void cancelDownload( VxGUID& lclSessionId );

};
