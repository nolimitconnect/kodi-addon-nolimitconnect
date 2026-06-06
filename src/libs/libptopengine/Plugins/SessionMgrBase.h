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

#include <CoreLib/VxMutex.h>

#include <memory>

class ConnectId;
class P2PEngine;
class PluginBase;
class PluginMgr;
class IToGui;
class VxGUID;
class VxNetIdent;
class VxSktBase;

class SessionMgrBase
{
public:
	class AutoSessionMgrLock
	{
	public:
		AutoSessionMgrLock( SessionMgrBase * mgr ) : m_SessionMgrMutex(mgr->getSessionMgrMutex())	{ m_SessionMgrMutex.lock(); }
		~AutoSessionMgrLock()																		{ m_SessionMgrMutex.unlock(); }
		VxMutex&				m_SessionMgrMutex;
	};

	SessionMgrBase( P2PEngine& engine, PluginBase& plugin, PluginMgr& pluginMgr );
	virtual ~SessionMgrBase() = default;

	PluginBase&					getPlugin( void )							{ return m_Plugin; }
	PluginMgr&					getPluginMgr( void )						{ return m_PluginMgr; }
    IToGui&						getToGui( void );
	VxMutex&					getSessionMgrMutex( void )					{ return m_SessionMgrMutex; }

    EPluginType                 getPluginType( void );

	bool						isPluginSingleSession( void );

	virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) = 0;
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) = 0;
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase ) = 0;

	virtual void				onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) = 0;

protected:
	//=== vars ===//
    P2PEngine&                  m_Engine;
	PluginBase&					m_Plugin;
	PluginMgr&					m_PluginMgr;
	
	VxMutex						m_SessionMgrMutex;

private:
	SessionMgrBase(); // do not allow
};
