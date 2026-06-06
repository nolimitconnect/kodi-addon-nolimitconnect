#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktBaseMgr.h"

class VxSktConnect;

// implements a manager to manage connect sockets
class VxClientMgr : public VxSktBaseMgr
{
public:
	VxClientMgr();
	virtual ~VxClientMgr() = default;

	void								sktMgrStartup( bool ipv6 ) override;

    virtual std::shared_ptr<VxSktBase>	makeNewSkt( void ) override;

	//! Connect to ip or URL and return socket.. if cannot connect return NULL
	virtual std::shared_ptr<VxSktBase>	connectTo(	const char*	pIpOrUrl,			// remote ip or url 
										uint16_t	u16Port,						// port to connect to
										int			iTimeoutMilliSeconds = 1000 );	// seconds before connect attempt times out
};

