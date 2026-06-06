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

#include "BaseInfo.h"

#include <CoreLib/VxUrl.h>

class BaseHostInfo : public BaseInfo
{
public:
    BaseHostInfo();
    BaseHostInfo( const BaseHostInfo& rhs );
    virtual ~BaseHostInfo() = default;

    BaseHostInfo&				operator=( const BaseHostInfo& rhs ); 

    virtual void				setHostType( enum EHostType hostType )          { m_HostType = hostType; }
    virtual EHostType			getHostType( void )                             { return m_HostType; }
    virtual void				setConnectUrl( VxUrl& connectUrl )              { m_ConnectUrl = connectUrl; }
    virtual VxUrl&			    getConnectUrl( void )                           { return m_ConnectUrl; }

    bool                        isHostMatch( enum EHostType hostType, VxGUID& onlineId );

public:
	//=== vars ===//
    EHostType                   m_HostType{ eHostTypeUnknown };
    VxUrl                       m_ConnectUrl;
};
