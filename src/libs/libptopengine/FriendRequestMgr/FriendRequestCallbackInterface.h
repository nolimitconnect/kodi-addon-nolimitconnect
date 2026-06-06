#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

class FriendRequestInfo;
class VxGUID;

class FriendRequestCallbackInterface
{
public:
    virtual void				callbackFriendRequestUpdated( std::shared_ptr<FriendRequestInfo>& friendRequest ){};
    virtual void				callbackFriendRequestRemoved( VxGUID& friendOnlineId, VxGUID& requestId ){};
};

