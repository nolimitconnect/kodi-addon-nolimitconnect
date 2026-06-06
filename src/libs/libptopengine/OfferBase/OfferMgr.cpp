//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferMgr.h"

//============================================================================
OfferMgr::OfferMgr()
: OfferBaseMgr( eOfferMgrClient )
{
}

//============================================================================
void OfferMgr::fromGuiUserLoggedOn( void )
{
    OfferBaseMgr::fromGuiUserLoggedOn();
}

//============================================================================
bool OfferMgr::fromGuiOfferCreated( OfferBaseInfo& offerInfo )
{
    return addOffer( offerInfo );
}

//============================================================================
bool OfferMgr::fromGuiOfferUpdated( OfferBaseInfo& offerInfo )
{
    return updateOffer( offerInfo );
}

