//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseService.h"
#include "PluginMgr.h"
#include "P2PSession.h"
#include "RxSession.h"
#include "TxSession.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
PluginBaseService::PluginBaseService( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBase( engine, pluginMgr, myIdent, pluginType )
, m_HostType( PluginTypeToHostType( pluginType ) )
, m_HostedId( myIdent->getMyOnlineId(), m_HostType )
{
}

//============================================================================
void PluginBaseService::broadcastToClients( VxPktHdr* pktHdr, VxGUID& requesterOnlineId, std::shared_ptr<VxSktBase>& sktBaseRequester, bool includeRequester )
{
    if( pktHdr && pktHdr->isValidPktPrefix() )
    {
        bool sentToRequestor{ false };
        VxGUID requestorSktConnectionId;
        if( sktBaseRequester )
        {
            requestorSktConnectionId = sktBaseRequester->getSocketId();
        }

        std::set<ConnectId> connectIdSet;
        std::set<ConnectId> relayedIdSet;
        if( m_Engine.getConnectIdListMgr().getConnections( getHostedId(), connectIdSet, relayedIdSet ) )
        {
            for( auto& connectId : connectIdSet )
            {
                VxGUID memberOnlineId = const_cast<ConnectId&>(connectId).getUserOnlineId();
                VxGUID socketId = const_cast<ConnectId&>(connectId).getSocketId();
                GroupieId groupieId = const_cast<ConnectId&>(connectId).getGroupieId();
                #if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseService::%s lockSktBaseMgr", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().lockSktBaseMgr();
                #if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseService::%s lockSktBaseMgr locked", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                std::shared_ptr<VxSktBase> sktBase = m_Engine.getPeerMgr().findSktBase( socketId, true );
                if( sktBase && sktBase->isConnected() )
                {
                    VxGUID peerOnlineId = sktBase->getPeerOnlineId();
                    if( sktBase->getPeerOnlineId() != memberOnlineId )
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer %s id %s does NOT match user id %s for pkt %s",
                                sktBase->getPeerOnlineName().c_str(), peerOnlineId.toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str(),
                                pktHdr->describePktHdr().c_str() );
                        #if defined(DEBUG_SKT_MGR_LOCK)
							LogMsg( LOG_DEBUG, "PluginBaseService::%s unlockSktBaseMgr", __func__ );
						#endif // defined(DEBUG_SKT_MGR_LOCK)
                        m_Engine.getPeerMgr().unlockSktBaseMgr();
                        continue;
                    }

                    if( peerOnlineId == requesterOnlineId )
                    {
                        if( !includeRequester )
                        {
                            LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients excluding requestor %s id %s for pkt %s",
                                    sktBase->getPeerOnlineName().c_str(), sktBase->getPeerOnlineId().toOnlineIdString().c_str(), 
                                    pktHdr->describePktHdr().c_str() );
                            #if defined(DEBUG_SKT_MGR_LOCK)
							    LogMsg( LOG_DEBUG, "PluginBaseService::%s unlockSktBaseMgr", __func__ );
						    #endif // defined(DEBUG_SKT_MGR_LOCK)
                            m_Engine.getPeerMgr().unlockSktBaseMgr();
                            continue;
                        }

                        sentToRequestor = true;
                    }


                    LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseService::broadcastToClients pkt %s to %s peer %s", pktHdr->describePktHdr().c_str(),
                               m_Engine.describeGroupieId(groupieId).c_str(), sktBase->getPeerPktAnn().describeUser().c_str() );
                    if(  txPacket( memberOnlineId, sktBase, pktHdr, getClientPluginType() ) )
                    {
                        // should we log fail to send ?
                    }
                }

                #if defined(DEBUG_SKT_MGR_LOCK)
					LogMsg( LOG_DEBUG, "PluginBaseService::%s unlockSktBaseMgr", __func__ );
				#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().unlockSktBaseMgr();
            }
        }

        if( !sentToRequestor && includeRequester && sktBaseRequester && requesterOnlineId.isValid() )
        {
            // allways send to requester even if not still joined
            txPacket( requesterOnlineId, sktBaseRequester, pktHdr, getClientPluginType() );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseService::broadcastToHostClients invalid pkt %s host %s", pktHdr->describePktHdr().c_str(), DescribeHostType( getHostType() ) );
    }
}

//============================================================================
void PluginBaseService::broadcastToClients( VxPktHdr* pktHdr, VxGUID& excludedOnlineId )
{
    if( pktHdr && pktHdr->isValidPktPrefix() )
    {
        std::set<ConnectId> connectIdSet;
        std::set<ConnectId> relayedIdSet;
        if( m_Engine.getConnectIdListMgr().getConnections( getHostedId(), connectIdSet, relayedIdSet ) )
        {
            for( auto& connectId : connectIdSet )
            {
                VxGUID memberOnlineId = const_cast<ConnectId&>(connectId).getUserOnlineId();
                VxGUID socketId = const_cast<ConnectId&>(connectId).getSocketId();
                GroupieId groupieId = const_cast<ConnectId&>(connectId).getGroupieId();

                m_Engine.getPeerMgr().lockSktBaseMgr();
                std::shared_ptr<VxSktBase> sktBase = m_Engine.getPeerMgr().findSktBase( socketId, true );
#if defined(DEBUG_SKT_MGR_LOCK)
                LogMsg( LOG_DEBUG, "PluginBaseService::%s unlockSktBaseMgr", __func__ );
#endif // defined(DEBUG_SKT_MGR_LOCK)
                m_Engine.getPeerMgr().unlockSktBaseMgr();
                if( sktBase && sktBase->isConnected() )
                {
                    if( sktBase->getPeerOnlineId() != memberOnlineId )
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer id %s does NOT match user id %s",
                                sktBase->getPeerOnlineId().toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str() );
                    }
                    else
                    {
                        LogMsg( LOG_VERBOSE, "PluginBaseService::broadcastToClients peer id %s does match user id %s",
                                sktBase->getPeerOnlineId().toOnlineIdString().c_str(), memberOnlineId.toOnlineIdString().c_str() );
                    }

                    bool isExcludeId = excludedOnlineId == sktBase->getPeerOnlineId();
                    if( isExcludeId )
                    {
                        continue;
                    }

                    LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseService::broadcastToClients pkt %s to %s peer %s", pktHdr->describePktHdr().c_str(),
                               m_Engine.describeGroupieId(groupieId).c_str(), sktBase->getPeerPktAnn().describeUser().c_str() );
                   
                    if( !txPacket( memberOnlineId, sktBase, pktHdr, getClientPluginType() ) )
                    {
                        // logging ?
                    }
                }
            }
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseService::broadcastToHostClients invalid pkt %s host %s", pktHdr->describePktHdr().c_str(),  DescribeHostType( getHostType() ) );
    }
}

//============================================================================
EConnectReason PluginBaseService::getHostAnnounceConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomAnnounce;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupAnnounce;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectAnnounce;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
EConnectReason PluginBaseService::getHostJoinConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomJoin;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupJoin;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectJoin;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
EConnectReason PluginBaseService::getHostSearchConnectReason( void )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        connectReason = eConnectReasonChatRoomSearch;
        break;
    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        connectReason = eConnectReasonGroupSearch;
        break;
    case ePluginTypeHostRandomConnect:
    case ePluginTypeClientRandomConnect:
        connectReason = eConnectReasonRandomConnectSearch;
        break;
    default:
        break;
    }

    return connectReason;
}
