//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginRandomConnectHost.h"

#include "PluginMgr.h"
#include "P2PSession.h"
#include "RxSession.h"
#include "TxSession.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxFileUtil.h>

#include <PktLib/PktAdminAvail.h>
#include <PktLib/PktsRandConnect.h>

//============================================================================
PluginRandomConnectHost::PluginRandomConnectHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseHostService( engine, pluginMgr, myIdent, pluginType )
{
    setPluginType( ePluginTypeHostRandomConnect );
}

//============================================================================
void PluginRandomConnectHost::pluginStartup( void )
{
    PluginBaseHostService::pluginStartup();
}

//============================================================================
void PluginRandomConnectHost::onPktRandConnectReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktRandConnectReq* pktReq = (PktRandConnectReq*)pktHdr;
    PktRandConnectReply pktReply;
    GroupieId groupieId = pktReq->getGroupieId();
    pktReply.setGroupieId( groupieId );
    pktReply.setToUserOnlineId( pktReq->getToUserOnlineId() );
    pktReply.setSessionId( pktReq->getSessionId() );
    pktReply.setTimeRequestedMs( pktReq->getTimeRequestedMs() );
    pktReply.setOfferType( pktReq->getOfferType() );
    pktReply.setAccessState( pktReq->getAccessState() );
    pktReply.setRandAction( pktReq->getRandAction() );

    VxGUID excludeId;
    broadcastToClients( &pktReply, excludeId );
}

//============================================================================
void PluginRandomConnectHost::fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing )
{
    if( pluginType != getPluginType() )
    {
        return;
    }

    GroupieId groupieId( m_Engine.getMyOnlineId(), m_Engine.getMyOnlineId(), eHostTypeRandomConnect );
    PktAdminAvail pktAdminAvail;
    pktAdminAvail.setAdminAvailable( adminIsViewing );
    pktAdminAvail.setAdminGroupieId( groupieId );

    VxGUID excludeId;
    broadcastToClients( &pktAdminAvail, excludeId );
}
