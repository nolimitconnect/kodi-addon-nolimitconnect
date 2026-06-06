#pragma once
//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <vector>
#include <memory>

class VxSktBase;

class IConnectedCallback
{
public:
    /// return true if have use for this connection
    virtual bool                onContactConnected( std::shared_ptr<VxSktBase>& sktBase ) = 0;
    virtual void                onContactDisconnected( std::shared_ptr<VxSktBase>& sktBase ) = 0;
};
