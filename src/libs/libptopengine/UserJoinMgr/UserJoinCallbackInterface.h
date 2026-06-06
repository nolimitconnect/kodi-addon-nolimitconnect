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

#include <GuiInterface/IDefs.h>

class UserJoinInfo;
class VxGUID;

class UserJoinCallbackInterface
{
public:
    virtual void				callbackUserJoinAdded( UserJoinInfo* hostJoinInfo ) {};
    virtual void				callbackUserJoinUpdated( UserJoinInfo* hostJoinInfo ) {};
    virtual void				callbackUserUnJoin( UserJoinInfo* hostJoinInfo ) {};
    virtual void				callbackUserJoinRemoved( GroupieId& groupieId ) {};

    virtual void				callbackUserJoinOfferState( GroupieId& groupieId, EJoinState userOfferState ) {};
    virtual void				callbackUserJoinOnlineState( GroupieId& groupieId, EOnlineState onlineState, VxGUID& connectionId ) {};

    virtual void				callbackUserJoinAHostStatus( EHostType hostType, VxGUID& sessionId, EConnectStatus connectStatus ) {};
};

