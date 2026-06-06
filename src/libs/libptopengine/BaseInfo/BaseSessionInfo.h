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

#include <PktLib/HostUserSessionId.h>

class VxNetIdent;

class BaseSessionInfo : public HostUserSessionId
{
public:
    BaseSessionInfo();
    BaseSessionInfo( HostUserSessionId& hostUserSessionId );
	BaseSessionInfo( const BaseSessionInfo& rhs );
    virtual ~BaseSessionInfo() = default;

	BaseSessionInfo&			operator=( const BaseSessionInfo& rhs ); 
    bool			            operator==( const BaseSessionInfo& rhs ); 

    virtual HostUserSessionId&  getHostUserSessionId( void )                    { return *this; }

    virtual void				setOnlineState( EOnlineState onlineState )      { m_OnlineState = onlineState; }
    virtual EOnlineState	    getOnlineState( void )                          { return m_OnlineState; }

    virtual void				setJoinState( EJoinState joinState )            { m_JoinState = joinState; }
    virtual EJoinState	        getJoineState( void )                           { return m_JoinState; }

public:
	//=== vars ===//
    EOnlineState                m_OnlineState{ eOnlineStateUnknown };
    EJoinState                  m_JoinState{ eJoinStateNone };
};
