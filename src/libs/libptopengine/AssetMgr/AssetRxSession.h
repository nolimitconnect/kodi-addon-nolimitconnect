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

#include "AssetXferSession.h"

class AssetRxSession : public AssetXferSession
{
public:
	AssetRxSession( P2PEngine& engine );
	AssetRxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
	AssetRxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
	virtual ~AssetRxSession() = default;

	void cancelDownload( VxGUID& lclSessionId );

};
