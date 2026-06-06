//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginPersonFileXfer.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PluginPersonFileXfer::PluginPersonFileXfer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseFiles( engine, pluginMgr, myIdent, pluginType, m_FileInfoPersonFileXferMgr )
, m_FileInfoPersonFileXferMgr( engine, *this, "PersonFileXfer.db3" )
{
    setPluginType( ePluginTypePersonFileXfer );
}


//============================================================================
//! packet with remote users reply to offer
void PluginPersonFileXfer::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "PluginPersonFileXfer::onPktPluginOfferReply" );
	m_FileInfoXferMgr.onPktPluginOfferReply( sktBase, pktHdr, netIdent );
}
