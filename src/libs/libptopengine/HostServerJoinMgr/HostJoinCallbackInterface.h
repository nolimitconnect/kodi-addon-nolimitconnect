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

class HostJoinInfo;
class VxGUID;
class GroupieId;

class HostJoinCallbackInterface
{
public:
    virtual void				callbackHostJoinRequested( HostJoinInfo* hostJoinInfo ) {};
    virtual void				callbackHostJoinUpdated( HostJoinInfo* hostJoinInfo ){};
    virtual void				callbackHostUnJoin( GroupieId& groupieId ) {};
    virtual void				callbackHostJoinRemoved( GroupieId& groupieId ){};

    virtual void				callbackHostJoinOfferState( GroupieId& groupieId, enum EJoinState userHostOfferState ) {};
    virtual void				callbackHostJoinOnlineState( GroupieId& groupieId, enum EOnlineState onlineState, VxGUID& connectionId ) {};
};

