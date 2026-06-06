#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>

#include "OfferBaseXferSession.h"

class OfferBaseRxSession : public OfferBaseXferSession
{
public:
	OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr );
	OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	OfferBaseRxSession( P2PEngine& engine, OfferBaseMgr& offerMgr, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	virtual ~OfferBaseRxSession() = default;

	void cancelDownload( VxGUID& lclSessionId );

};
