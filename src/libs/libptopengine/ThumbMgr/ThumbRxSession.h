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

#include "ThumbXferSession.h"

class ThumbRxSession : public ThumbXferSession
{
public:
	ThumbRxSession( P2PEngine& engine );
	ThumbRxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	ThumbRxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	virtual ~ThumbRxSession() = default;

	void						cancelDownload( VxGUID& lclSessionId );
};
