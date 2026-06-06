//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "TxSession.h"

//============================================================================
TxSession::TxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: PluginSessionBase( sktBase, sendToId, pluginType )
{
	setSessionType(ePluginSessionTypeTx);
}

//============================================================================
TxSession::TxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: PluginSessionBase( lclSessionId, sktBase, sendToId, pluginType )
{
	setSessionType(ePluginSessionTypeTx);
}
