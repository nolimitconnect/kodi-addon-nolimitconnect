#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <string>

class P2PEngine;
class NetServicesMgr;
class NetServiceUtils;
class VxGUID;

class NetActionBase
{
public:
	NetActionBase( NetServicesMgr& netServicesMgr );
    virtual ~NetActionBase() = default;

    virtual std::string         getNetworkKey( void );

	virtual ENetActionType		getNetActionType( void )			{ return eNetActionUnknown; }
	virtual VxGUID&			    getMyOnlineId( void );

	virtual void				enterAction( void )					{};
	virtual void				doAction( void )					{};
	virtual void				exitAction( void )					{};

protected:
	NetServicesMgr&				m_NetServicesMgr;
	NetServiceUtils&			m_NetServiceUtils;
	P2PEngine&					m_Engine;
};

