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

class GroupieId;

class MemberActiveCallback
{
public:
    virtual void				callbackMemberActive( GroupieId& onlineId, bool isActive ) {};
};

