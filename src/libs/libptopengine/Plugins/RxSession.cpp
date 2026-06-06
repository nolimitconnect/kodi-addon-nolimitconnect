//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RxSession.h"

//============================================================================
RxSession::RxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: PluginSessionBase( sktBase, sendToId, pluginType )
, m_PktVideoFeedPic( 0 )
{
	setSessionType(ePluginSessionTypeRx);
	setIsSessionStarted( true );
}

//============================================================================
RxSession::RxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: PluginSessionBase( lclSessionId, sktBase, sendToId, pluginType )
, m_PktVideoFeedPic( 0 )
{
	setSessionType(ePluginSessionTypeRx);
	setIsSessionStarted( true );
}

//============================================================================
RxSession::~RxSession()
{
}

//============================================================================
bool RxSession::waitForResponse( int iTimeoutMs )
{
	return m_Semaphore.wait( iTimeoutMs );
}
