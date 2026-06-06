//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxMutex.h>

#include <map>

class ConnectedInfo;
class VxSktBase;

class ConnectedListBase
{
public:
    ConnectedListBase() = default;
    virtual ~ConnectedListBase() = default;

    void                        lockConnectedList( void )   { m_ConnectedListMutex.lock(); }
    void                        unlockConnectedList( void ) { m_ConnectedListMutex.unlock(); }

    std::map<std::pair<VxGUID, VxGUID>, ConnectedInfo *>&	getConnectedList( void )		{ return m_ConnectList; }

protected:
    VxMutex                     m_ConnectedListMutex;
    std::map<std::pair<VxGUID, VxGUID>, ConnectedInfo *>    m_ConnectList; // pair is socketId, onlineId
};
