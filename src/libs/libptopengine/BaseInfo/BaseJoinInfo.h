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

class BaseJoinInfo : public BaseInfo
{
public:
    BaseJoinInfo();
    BaseJoinInfo( const BaseJoinInfo& rhs );
    virtual ~BaseJoinInfo() = default;

    BaseJoinInfo&				operator=( const BaseJoinInfo& rhs ); 

    virtual void			    setHostType( enum EHostType hostType )              { m_HostType = hostType; }
    virtual EHostType			getHostType( void ) const                           { return m_HostType; }

    virtual bool			    setJoinState( enum EJoinState joinState )           { if(m_JoinState == joinState) return false; m_JoinState = joinState; return true; }
    virtual EJoinState			getJoinState( void ) const                          { return m_JoinState; }

    virtual void				setLastConnectTime( uint64_t timestamp )            { m_LastConnectMs = timestamp; }
    virtual uint64_t			getLastConnectTime( void )  const                   { return m_LastConnectMs; }

    virtual void				setLastJoinTime( uint64_t timestamp )               { m_LastJoinMs = timestamp; }
    virtual uint64_t			getLastJoinTime( void )  const                      { return m_LastJoinMs; }

public:
	//=== vars ===//
    EHostType                   m_HostType{ eHostTypeUnknown };
    EJoinState                  m_JoinState{ eJoinStateNone };
    int64_t						m_LastConnectMs{ 0 };
    int64_t						m_LastJoinMs{ 0 };
};
