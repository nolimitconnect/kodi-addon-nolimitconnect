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

#include <PktLib/PktsRandConnectDefs.h>
#include <stdint.h>

class GroupieId;
class VxGUID;

class RandConnectCallback
{
public:
    virtual void				callbackRandConnect( GroupieId& onlineId, enum ERandAction randAction ) {};
    virtual void                callbackRandConnectOffer( GroupieId& groupieId,
                                                           VxGUID& toUserOnlineId,
                                                           VxGUID& sessionId,
                                                           enum ERandAction randAction,
                                                           uint64_t timeRequestedMs,
                                                           EOfferType offerType ) {};
};

