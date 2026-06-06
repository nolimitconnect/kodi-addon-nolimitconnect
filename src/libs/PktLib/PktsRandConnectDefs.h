#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

enum ERandAction
{
    eRandActionNone,
    eRandActionSelectUser,
    eRandActionDeselectUser,

    // One-on-one offer lifecycle actions for random connect members.
    eRandActionOfferRequest,
    eRandActionOfferAccept,
    eRandActionOfferReject,
    eRandActionOfferCancel,
    eRandActionOfferNoResponse,
    eRandActionOfferMissed,

    eMaxRandAction
};
