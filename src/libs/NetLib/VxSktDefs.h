#pragma once
//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <memory>

class VxSktBase;

#define MAX_DISCONNECTED_SKT_TO_KEEP_AROUND				50

enum ESktMgrType
{
	eSktMgrTypeNone				= 0,
	eSktMgrTypeTcpConnect		= 1,
	eSktMgrTypeTcpAccept		= 2,

	eMaxSktMgrType			// always last
};

enum ESktCallbackReason
{
	eSktCallbackReasonUnknown		= 0,
	eSktCallbackReasonConnecting	= 1,
	eSktCallbackReasonConnectError	= 2,
	eSktCallbackReasonConnected		= 3,
	eSktCallbackReasonData			= 4,
	eSktCallbackReasonClosing		= 5,
	eSktCallbackReasonClosed		= 6,
	eSktCallbackReasonError			= 7,
	eSktCallbackReasonNewMgr		= 8,	// socket is being transfered to new manager

	eMaxSktCallbackReason			// always last
};

enum EIpProtocolType
{
	eIpProtocolTypeInvalid	= 0,
	eIpProtocolTypeUdp		= 1,
	eIpProtocolTypeTcp		= 2,
	eMaxIpProtocolType		= 3
};
