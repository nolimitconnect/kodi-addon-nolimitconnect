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

class VxGUID;

class SktListCallbackInterface
{
public:
    virtual void				callbackUserListUpdated( EUserViewType listType, VxGUID& onlineId, uint64_t timestamp ) {};
    virtual void				callbackUserListRemoved( EUserViewType listType, VxGUID& onlineId ) {};
};

