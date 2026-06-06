//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxClientMgr.h"
#include "VxSktConnect.h"

#include <CoreLib/VxDebug.h>

//============================================================================
VxClientMgr::VxClientMgr()
{
	m_eSktMgrType = eSktMgrTypeTcpConnect;
}

//============================================================================
void VxClientMgr::sktMgrStartup( bool ipv6 )
{
	// empty
}

//============================================================================
//! make a new socket... give derived classes a chance to override
std::shared_ptr<VxSktBase> VxClientMgr::makeNewSkt( void )
{ 
	std::shared_ptr<VxSktBase> sharedSkt( new VxSktConnect() );
	sharedSkt->setThisSkt( sharedSkt ); // so skt can do callbacks without look up in manager
	return sharedSkt;
}

//============================================================================
//! Connect to ip or url and return socket.. if cannot connect return NULL
std::shared_ptr<VxSktBase> VxClientMgr::connectTo(	const char*		pIpOrUrl,				// remote ip or url 
													uint16_t		u16Port,				// port to connect to
													int				iTimeoutMilliSeconds )	// milli seconds before connect attempt times out
{
	if( NULL ==  m_pfnUserReceive )
	{
		LogMsg( LOG_INFO, "VxClientMgr::VxConnectTo: you must call setReceiveCallback first" );
		vx_assert( m_pfnUserReceive );
	}

	std::shared_ptr<VxSktBase> sktBase = makeNewSkt();
	sktBase->m_SktMgr		= this;
	int32_t rc = sktBase->connectTo(	m_LclIp,
									pIpOrUrl, 
									u16Port, 
									iTimeoutMilliSeconds );
	if( rc )
	{
		LogModule( eLogSkt, LOG_INFO, "VxClientMgr::VxConnectTo: error %d\n", rc );

		sktBase.reset();
	}
	else
	{
		addSkt( sktBase );
	}

	return sktBase;
}
