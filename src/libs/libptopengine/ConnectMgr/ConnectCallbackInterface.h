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

class ConnectInfo;
class VxGUID;

class ConnectCallbackInterface
{
public:
    virtual void				callbackConnectAdded( ConnectInfo * userHostInfo ){};
    virtual void				callbackConnectUpdated( ConnectInfo * userHostInfo ){};
    virtual void				callbackConnectRemoved( VxGUID& userHostId ){};
    virtual void				callbackConnectOfferState( VxGUID& hostOnlineId, enum EOfferState userHostOfferState ) {};
    virtual void				callbackConnectOnlineState( VxGUID& hostOnlineId, enum EOnlineState onlineState, VxGUID& connectionId ) {};
};

