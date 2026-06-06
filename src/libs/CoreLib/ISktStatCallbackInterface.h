#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>
#include "VxDefs.h"

#include <string>

class ISktStatCallbackInterface
{
public:
	virtual void                sktConnected( SOCKET skt ) = 0;
	virtual void                sktConnected2( SOCKET skt, std::string ipAddr ) = 0;
	virtual void                sktConnected4( SOCKET skt, std::string ipAddr, ESktType sktType, EConnectReason connectReason ) = 0;
	virtual void                sktSetRemoteAddr( SOCKET skt, std::string ipAddr ) = 0;
	virtual void                sktSetType( SOCKET skt, ESktType sktType ) = 0;
	virtual void                sktClosed( SOCKET skt ) = 0;
};