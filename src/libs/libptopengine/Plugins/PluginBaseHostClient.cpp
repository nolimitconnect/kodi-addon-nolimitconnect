//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseHostClient.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <UserJoinMgr/UserJoinMgr.h>
#include <NetLib/VxPeerMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/SearchParams.h>
#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsGroupie.h>

namespace
{
ELogModule HostTypeJoinLogModule( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeGroup:
        return eLogGroup;
    case eHostTypeChatRoom:
        return eLogChatRoom;
    case eHostTypeRandomConnect:
        return eLogRandomConnect;
    default:
        return eLogHostJoin;
    }
}

ELogModule HostTypeMembershipLogModule( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeGroup:
        return eLogGroup;
    case eHostTypeChatRoom:
        return eLogChatRoom;
    case eHostTypeRandomConnect:
        return eLogRandomConnect;
    default:
        return eLogMembership;
    }
}
}

//============================================================================
PluginBaseHostClient::PluginBaseHostClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
    : PluginBaseMultimedia( engine, pluginMgr, myIdent, pluginType )
    , m_ConnectionMgr(engine.getConnectionMgr())
    , m_HostClientMgr(engine, pluginMgr, myIdent, *this)
{
}

//============================================================================
void PluginBaseHostClient::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    if( !ptopUrl.empty() )
    {
        //VxGUID hostGuid;
        //EHostJoinStatus joinStatus = m_ConnectionMgr.lookupOrQueryId( hostType, url.c_str(), hostGuid, this);
    }
    else
    {
        m_Engine.getToGui().toGuiHostAnnounceStatus( adminId.getHostType(), sessionId, eHostAnnounceInvalidUrl );
    }
}

//============================================================================
void PluginBaseHostClient::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    if(  !ptopUrl.empty() )
    {
        //VxGUID hostGuid;
        //EHostJoinStatus joinStatus = m_ConnectionMgr.lookupOrQueryId( hostType, url.c_str(), hostGuid, this);
    }
    else
    {
        m_Engine.getToGui().toGuiHostJoinStatus( adminId.getHostType(), sessionId, eHostJoinInvalidUrl );
    }
}

//============================================================================
void PluginBaseHostClient::fromGuiLeaveHost( HostedId& adminId )
{
    sendLeaveHost( adminId );
    m_HostClientMgr.fromGuiLeaveHost( adminId );
}

//============================================================================
void PluginBaseHostClient::fromGuiUnJoinHost( HostedId& adminId )
{
    sendUnJoinHost( adminId );
    m_HostClientMgr.fromGuiUnJoinHost( adminId );
}

//============================================================================
void PluginBaseHostClient::fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable )
{
    std::string url = searchParams.getSearchUrl();
    if( !url.empty() )
    {
        //VxGUID hostGuid; // TODO
        //EHostJoinStatus joinStatus = m_ConnectionMgr.lookupOrQueryId( hostType, url.c_str(), hostGuid, this);
    }
    else
    {
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, searchParams.getSearchSessionId(), eHostSearchInvalidUrl );
        m_Engine.getToGui().toGuiHostSearchComplete( hostType, searchParams.getSearchSessionId() );
    }
}

//============================================================================
bool PluginBaseHostClient::fromGuiRequestPluginThumb( VxNetIdent* netIdent, VxGUID& thumbId )
{
    if( netIdent && thumbId.isValid() )
    {
        std::shared_ptr<VxSktBase> sktBase( nullptr );
        m_PluginMgr.pluginApiSktConnectTo( getPluginType(), netIdent, 0, sktBase );
        if( sktBase && sktBase->isConnected() )
        {
            // the netIdent from gui is not the same one as in big list
            BigListInfo* bigListInfo = m_Engine.getBigListMgr().findBigListInfo( netIdent->getMyOnlineId() );
            if( bigListInfo )
            {
                return ptopEngineRequestPluginThumb( sktBase, bigListInfo->getVxNetIdent(), thumbId );
            }
        }
    }

    return false;
}

//============================================================================
bool PluginBaseHostClient::ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb )
{
    return m_ThumbXferMgr.requestPluginThumb( sktBase, netIdent, thumbId, tmpThumb );
}

//============================================================================
void PluginBaseHostClient::sendLeaveHost( HostedId& adminId )
{
    VxGUID hostOnlineId = adminId.getHostOnlineId();
    if( hostOnlineId.isValid() )
    {
        GroupieId groupieId( m_Engine.getMyOnlineId(), adminId );
        sendLeaveHost( groupieId );
    }
}

//============================================================================
bool PluginBaseHostClient::sendLeaveHost( GroupieId& groupieId )
{
    ELogModule logModule = HostTypeJoinLogModule( groupieId.getHostType() );
    if( LogEnabled( logModule ) )LogModule( logModule, LOG_VERBOSE, "PluginBaseHostClient::sendLeaveHost groupie %s my online id %s",
               groupieId.describeGroupieId().c_str(), m_Engine.getMyOnlineId().describeVxGUID().c_str());
    bool pktSent{ false };
    std::shared_ptr<VxSktBase> sktBase =  m_Engine.getConnectIdListMgr().findHostConnection( groupieId );
    if( sktBase )
    {
        PktHostLeaveReq leaveReq;
        leaveReq.setGroupieId( groupieId );
        leaveReq.setPluginType( HostTypeToClientPlugin( groupieId.getHostType() ) );
        if( m_Engine.getMyOnlineId() == groupieId.getHostOnlineId() )
        {
            // is ourself
            pktSent = txPacket( m_Engine.getMyOnlineId(), m_Engine.getSktLoopback(), &leaveReq );
        }
        else
        {
            pktSent = txPacket( groupieId.getHostOnlineId(), sktBase, &leaveReq );
            if( pktSent )
            {
                m_Engine.getUserJoinMgr().onUserLeftHost( groupieId );
            }
        }
    }

    return pktSent;
}

//============================================================================
void PluginBaseHostClient::sendUnJoinHost( HostedId& adminId )
{
    VxGUID hostOnlineId = adminId.getHostOnlineId();
    if( hostOnlineId.isValid() )
    {
        GroupieId groupieId( m_Engine.getMyOnlineId(), adminId );
        sendUnJoinHost( groupieId );
    }
}

//============================================================================
bool PluginBaseHostClient::sendUnJoinHost( GroupieId& groupieId )
{
    bool pktSent{ false };
    std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findHostConnection( groupieId );
    if( sktBase )
    {
        PktHostUnJoinReq leaveReq;
        leaveReq.setHostType( groupieId.getHostType() );
        leaveReq.setPluginType( HostTypeToClientPlugin( groupieId.getHostType() ) );
        if( m_Engine.getMyOnlineId() == groupieId.getHostOnlineId() )
        {
            // is ourself
            pktSent = txPacket( m_Engine.getMyOnlineId(), m_Engine.getSktLoopback(), &leaveReq );
        }
        else
        {
            pktSent = txPacket( groupieId.getHostOnlineId(), sktBase, &leaveReq );
            if( pktSent )
            {
                m_Engine.getUserJoinMgr().onUserLeftHost( groupieId );
            }
        }
    }

    return pktSent;
}

//============================================================================
bool PluginBaseHostClient::queryUserListFromHost( GroupieId& groupieId )
{
    bool pktSent{ false };
    std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findHostConnection( groupieId );
    if( sktBase )
    {
        PktGroupieSearchReq pktReq;
        VxGUID sessionId;
        sessionId.initializeWithNewVxGUID();

        pktReq.setSearchSessionId( sessionId );
        pktReq.setHostType( groupieId.getHostType() );
        if( m_Engine.getMyOnlineId() == groupieId.getHostOnlineId() )
        {
            // is ourself
            pktSent = txPacket( m_Engine.getMyOnlineId(), m_Engine.getSktLoopback(), &pktReq );
        }
        else
        {
            pktSent = txPacket( groupieId.getHostOnlineId(), sktBase, &pktReq );
        }
    }

    return pktSent;
}

//============================================================================
void PluginBaseHostClient::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{

    m_HostClientMgr.onUserJoinedHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseHostClient::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostClientMgr.onUserLeftHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseHostClient::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostClientMgr.onUserUnJoinedHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseHostClient::onGroupRelayedUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostClientMgr.onGroupRelayedUserAnnounce( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseHostClient::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::onPktHostJoinReply %s", pktHdr->describePktHdr().c_str() );
    PktHostJoinReply* pktReply = (PktHostJoinReply*)pktHdr;
    GroupieId userGroupieId = pktReply->getGroupieId();
    ELogModule logModule = HostTypeMembershipLogModule( userGroupieId.getHostType() );
    if( LogEnabled( logModule ) )LogModule( logModule, LOG_VERBOSE, "PluginBaseHostClient::onPktHostJoinReply %s", m_Engine.describeGroupieId(userGroupieId).c_str() );
    m_Engine.getConnectIdListMgr().addConnection( sktBase, userGroupieId, netIdent->getMyOnlineId() != userGroupieId.getUserOnlineId() );
}

//============================================================================
void PluginBaseHostClient::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::onPktHostLeaveReply %s", pktHdr->describePktHdr().c_str() );
    PktHostLeaveReply* pktReply = (PktHostLeaveReply*)pktHdr;
    GroupieId userGroupieId = pktReply->getGroupieId();
    ELogModule logModule = HostTypeMembershipLogModule( userGroupieId.getHostType() );
    if( LogEnabled( logModule ) )LogModule( logModule, LOG_VERBOSE, "PluginBaseHostClient::onPktHostLeaveReply %s", m_Engine.describeGroupieId(userGroupieId).c_str() );
    m_Engine.getConnectIdListMgr().removeConnection( sktBase->getSocketId(), userGroupieId );
}

//============================================================================
void PluginBaseHostClient::onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
}

//============================================================================
void PluginBaseHostClient::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
}

//============================================================================
void PluginBaseHostClient::onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
    m_Engine.getHostedListMgr().onPktHostInfoReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseHostClient::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient:%s", __func__ );
}

//============================================================================
void PluginBaseHostClient::onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
    m_Engine.getGroupieListMgr().onPktGroupieInfoReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseHostClient::onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
    m_Engine.getGroupieListMgr().onPktGroupieAnnReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseHostClient::onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
     m_Engine.getGroupieListMgr().onPktGroupieSearchReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseHostClient::onPktGroupieMoreReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
    m_Engine.getGroupieListMgr().onPktGroupieMoreReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseHostClient::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
}

//============================================================================
void PluginBaseHostClient::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "PluginBaseHostClient::%s", __func__ );
}

