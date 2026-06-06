//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PingResponseServer.h"
#include "IsPortOpenTest.h"

#include <NetLib/VxSktBase.h>
#include <CoreLib/VxParse.h>

namespace
{
	void PingResponseCallbackHandler( std::shared_ptr<VxSktBase>&  sktBase, void * pvUserCallbackData )
	{
		PingResponseServer * peerMgr = ( PingResponseServer * )pvUserCallbackData;
		if( peerMgr )
		{
			peerMgr->handleTcpSktCallback( sktBase );
		}
	}
}

//============================================================================
PingResponseServer::PingResponseServer( IsPortOpenTest& isPortOpenTest )
: m_IsPortOpenTest( isPortOpenTest )
{
	setReceiveCallback( PingResponseCallbackHandler, this );
}

//============================================================================
void PingResponseServer::handleTcpSktCallback( std::shared_ptr<VxSktBase>& sktBase )
{
	switch( sktBase->getCallbackReason() )
	{

	case eSktCallbackReasonData:
		m_IsPortOpenTest.handleTcpData( sktBase );
		break;
	case eSktCallbackReasonConnectError:
	case eSktCallbackReasonConnected:
	case eSktCallbackReasonClosed:
	case eSktCallbackReasonError:
	case eSktCallbackReasonClosing:
	case eSktCallbackReasonConnecting:
	default:
		break;
	}
}
