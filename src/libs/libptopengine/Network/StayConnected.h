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
#include "ConnectRequest.h"

#include <PktLib/VxFriendMatch.h>
#include <CoreLib/VxThread.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxSemaphore.h>

#include <vector>
#include <memory>

class BigListInfo;
class BigListMgr;
class P2PConnectList;
class P2PEngine;
class PktAnnounce;
class NetworkMgr;
class VxGUID;
class VxPeerMgr;
class VxPktHdr;
class VxSktBase;

class StayConnected
{
public:
	StayConnected( P2PEngine& engine );
	virtual ~StayConnected();

	void						stayConnectedStartup( void );
	void						stayConnectedShutdown( void );

	void						doStayConnectedThread( void );

protected:

	//=== vars ===//
	VxThread					m_StayConnectedThread;
	P2PEngine&					m_Engine;
	BigListMgr&					m_BigListMgr;
	PktAnnounce&				m_PktAnn;
};

