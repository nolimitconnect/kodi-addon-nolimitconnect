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

#include "NetworkDefs.h"

#include <CoreLib/InetAddress.h>
#include <PktLib/VxFriendMatch.h>

#include <GuiInterface/IDefs.h>

#include <memory>

class P2PEngine;
class VxPeerMgr;
class BigListMgr;
class EngineSettings;
class VxNetIdent;
class VxSktBase;
class VxGUID;
class InetAddress;
class PktAnnounce;

class NetworkMgr
{
public:
	NetworkMgr( P2PEngine&		engine, 
				VxPeerMgr&		peerMgr,
				BigListMgr&		bigListMgr );
	virtual ~NetworkMgr() = default;

	P2PEngine&					getEngine( void )								{ return m_Engine; }
	VxPeerMgr&					getPeerMgr( void )								{ return m_PeerMgr; }

	void						setNetworkKey( std::string networkName )		{ m_NetworkName = networkName; }
	std::string					getNetworkKey( void )							{ return m_NetworkName; }

	void						updateFromEngineSettings( EngineSettings& engineSettings );

	void						handleTcpSktCallback( std::shared_ptr<VxSktBase>& sktBase );
	void						handleSktMgrStatusCallback( const char* sktAction, SOCKET sktHandle );

protected:
	P2PEngine&					m_Engine;
    PktAnnounce&				m_PktAnn;
    VxPeerMgr&					m_PeerMgr;
	BigListMgr&					m_BigListMgr;

	std::string					m_NetworkName;

};

