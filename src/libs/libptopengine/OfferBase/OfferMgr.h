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

#include <OfferBase/OfferBaseMgr.h>

#include <CoreLib/VxThread.h>
#include <CoreLib/VxSemaphore.h>
#include <CoreLib/VxMutex.h>

class OfferCallback;

class OfferMgr : public OfferBaseMgr
{
public:
	OfferMgr();
	~OfferMgr() override = default;

    void                        fromGuiUserLoggedOn( void ) override;

    bool				        fromGuiOfferCreated( OfferBaseInfo& offerInfo );
    bool				        fromGuiOfferUpdated( OfferBaseInfo& offerInfo );
};

