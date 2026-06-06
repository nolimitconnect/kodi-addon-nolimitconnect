//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostServerMgr.h"
#include "PluginBase.h"

#include <GuiInterface/IToGui.h>
#include <BigListLib/BigListInfo.h>
#include <P2PEngine/P2PEngine.h>
#include <HostServerJoinMgr/HostServerJoinMgr.h>
#include <Membership/MemberActiveMgr.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <UrlMgr/UrlMgr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostInvite.h>
#include <PktLib/PktsHostUser.h>
#include <PktLib/VxCommon.h>

namespace
{
ELogModule HostTypeLogModule( EHostType hostType, ELogModule defaultModule )
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
        return defaultModule;
    }
}
}

//============================================================================
HostServerMgr::HostServerMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase )
    : HostServerSearchMgr( engine, pluginMgr, myIdent, pluginBase )
{
}

//============================================================================
void HostServerMgr::onClientJoined( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_ClientListMutex.lock();
    addClient( sktBase, netIdent );
    m_ClientListMutex.unlock();

    addContact(sktBase, netIdent );
}

//============================================================================
void HostServerMgr::sendHostAnnounceToNetworkHost( VxGUID& sessionId, PktHostInviteAnnounceReq& hostAnnounce, EConnectReason connectReason )
{
    if( ePluginTypeHostNetwork == getPluginType() && m_Engine.isNetworkHostEnabled() )
    {
        // dont announce network host to network host if we have network host plugin enabled
        LogMsg( LOG_VERBOSE, "HostServerMgr attempted announce network host when we are the network host" );
        return;
    }

    if( !m_Engine.getNetStatusAccum().isNetHostAvailable() )
    {
        LogMsg( LOG_ERROR, "HostServerMgr::%s network host UNAVAILABLE %s", __func__, DescribePluginType( m_Plugin.getPluginType() ) );
        return;
    }

    // save announce pkt in announce session list
    std::string url = m_ConnectionMgr.getDefaultHostUrl( eHostTypeNetwork );
    if( url.empty() )
    {
        LogMsg( LOG_VERBOSE, "HostServerMgr network host url is empty" );
        return;
    }
    
    url = m_Engine.getUrlMgr().resolveUrl(url);

    if( !m_Engine.getNetStatusAccum().isRxPortOpen() )
    {
        m_Engine.getToGui().toGuiHostAnnounceStatus( hostAnnounce.getHostType(), sessionId, eHostAnnounceFailRequiresOpenPort, "Announce Host Requires An Open Port");
        return;
    }

    ELogModule hostLogModule = HostTypeLogModule( hostAnnounce.getHostType(), eLogHostJoin );
    if(LogEnabled( hostLogModule ))LogModule( hostLogModule, LOG_DEBUG, "HostServerMgr::%s %s", __func__, DescribePluginType( m_Plugin.getPluginType() ) );
    addAnnounceSession( sessionId, hostAnnounce.makeHostAnnCopy() );
    connectToHost( eHostTypeNetwork, sessionId, url, connectReason );
}

//============================================================================
void HostServerMgr::addAnnounceSession( VxGUID& sessionId, PktHostInviteAnnounceReq* hostAnn )
{
    removeAnnounceSession( sessionId );
    m_AnnListMutex.lock();
    m_AnnList[sessionId] = hostAnn;
    m_AnnListMutex.unlock();
}

//============================================================================
void HostServerMgr::removeAnnounceSession( VxGUID& sessionId )
{
    m_AnnListMutex.lock();
    auto iter = m_AnnList.find( sessionId );
    if( iter != m_AnnList.end() )
    {
        delete iter->second;
        m_AnnList.erase( iter );
    }

    m_AnnListMutex.unlock();
}

//============================================================================
void HostServerMgr::removeSession( VxGUID& sessionId, EConnectReason connectReason )
{
    if( isAnnounceConnectReason( connectReason ) )
    {
        removeAnnounceSession( sessionId );
    }

    HostServerSearchMgr::removeSession( sessionId, connectReason );
}

//============================================================================
void HostServerMgr::onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason )
{
    removeSession( sessionId, connectReason );

    HostBaseMgr::onContactDisconnected( sessionId, sktBase, onlineId, connectReason );
}

//============================================================================
bool HostServerMgr::onConnectToHostSuccess( EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason )
{
    bool result{ false };
    if( hostType == eHostTypeNetwork &&
        ( connectReason == eConnectReasonChatRoomAnnounce ||
            connectReason == eConnectReasonGroupAnnounce ||
            connectReason == eConnectReasonRandomConnectAnnounce ) )
    {
        m_AnnListMutex.lock();
        auto iter = m_AnnList.find( sessionId );
        if( iter != m_AnnList.end() )
        {
            if( iter->second->isValidPktPrefix() )
            {
                result = m_Plugin.txPacket( onlineId, sktBase, iter->second, ePluginTypeHostNetwork );
            }
            else
            {
                LogMsg( LOG_VERBOSE, "HostServerMgr m_PktHostInviteAnnounceReq is invalid" );
            }

            delete iter->second;
            m_AnnList.erase( iter );
        }

        m_AnnListMutex.unlock();
        if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "HostServerMgr::%s done with connection %d %s hostType %s connectReason %s", __func__,
            sktBase->getSktNumber(), sktBase->getPeerOnlineName().c_str(), DescribeHostType( hostType ), DescribeConnectReason( connectReason ) );
        m_Engine.getConnectionMgr().doneWithConnection( sessionId, onlineId, this, connectReason );
    }
    else
    {
        if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "HostServerMgr::%s success connection %d %s hostType %s connectReason %s", __func__,
            sktBase->getSktNumber(), sktBase->getPeerOnlineName().c_str(), DescribeHostType( hostType ), DescribeConnectReason( connectReason ) );
        HostBaseMgr::onConnectToHostSuccess( hostType, sessionId, sktBase, onlineId, connectReason );
        result = true;
    }

    return result;
}

//============================================================================
bool HostServerMgr::addClient( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    bool wasAdded = m_ContactList.addGuidIfDoesntExist( netIdent->getMyOnlineId() );
    if( wasAdded )
    {
        // TODO implement contact added
    }

    return wasAdded;
}

//============================================================================
bool HostServerMgr::removeClient( VxGUID& onlineId )
{
    bool wasRemoved = m_ContactList.removeGuid( onlineId );
    if( wasRemoved )
    {
        // TODO implement contact removed
    }

    return wasRemoved;
}

//============================================================================
void HostServerMgr::onJoinRequested( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID sessionId, EHostType hostType )
{
    ELogModule hostLogModule = HostTypeLogModule( hostType, eLogHostJoin );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_DEBUG, "onJoinRequested %s user %s", DescribePluginType( m_Plugin.getPluginType() ), netIdent->getOnlineName() );
    GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), hostType );
    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinRequested );
    m_Engine.getHostJoinMgr().onHostJoinRequestedByUser( sktBase, netIdent, sessionInfo );
}

//============================================================================
bool HostServerMgr::onUserJoined( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID sessionId, GroupieId& groupieId )
{
    if( groupieId.getUserOnlineId() != netIdent->getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, " HostServerMgr::onUserJoined onlineId does not match groupie user id" );
        return false;
    }

    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogHostJoin );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_DEBUG, "onUserJoined %s user %s", DescribePluginType( m_Plugin.getPluginType() ), netIdent->getOnlineName() );
    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinWasGranted );
    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
    return sktBase->isConnected();
}

//============================================================================
void HostServerMgr::onUserLeftHost( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID sessionId, EHostType hostType )
{
    ELogModule hostLogModule = HostTypeLogModule( hostType, eLogHostJoin );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_DEBUG, "onUserJoined %s user %s", DescribePluginType( m_Plugin.getPluginType() ), netIdent->getOnlineName() );
    GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), hostType );
    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    onUserLeftHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onUserUnJoined( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID sessionId, GroupieId& groupieId )
{
    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogHostJoin );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_DEBUG, "onUserJoined %s user %s", DescribePluginType( m_Plugin.getPluginType() ), netIdent->getOnlineName() );
    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
EJoinState HostServerMgr::getJoinState( VxNetIdent* netIdent, EHostType hostType )
{
    return m_Engine.getHostJoinMgr().fromGuiQueryJoinState( hostType, *netIdent );
}

//============================================================================
EMembershipState HostServerMgr::getMembershipState( VxNetIdent* netIdent, EHostType hostType )
{
    return m_Engine.getHostJoinMgr().fromGuiQueryMembership( hostType, *netIdent );
}

//============================================================================
void HostServerMgr::fromGuiListAction( EListAction listAction )
{
    HostServerSearchMgr::fromGuiListAction( listAction );
}

//============================================================================
void HostServerMgr::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );

    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    sessionInfo.setJoinState( eJoinStateJoinIsGranted );
    m_Engine.getHostJoinMgr().onHostJoinedByUser( sktBase, netIdent, sessionInfo );
    m_Engine.getGroupieListMgr().onHostJoinedByUser( sktBase, netIdent, sessionInfo );
    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );

    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogMembership );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_VERBOSE, "HostServerMgr::onUserJoinedHost %s", m_Engine.describeGroupieId(groupieId).c_str() );

    // broadcast users PktAnn to all users
    BigListInfo* bigListInfo = m_Engine.getBigListMgr().findBigListInfo( groupieId.getUserOnlineId() );
    if( bigListInfo )
    {
        m_ClientListMutex.lock();
        m_ClientList.addGuidIfDoesntExist( groupieId.getUserOnlineId() );
        m_ClientListMutex.unlock();

        PktAnnounce* pktAnn = bigListInfo->makeAnnCopy();
        if( pktAnn )
        {
            // send users pkt ann to all clients except the user who joined
            pktAnn->clearTempValues();
            pktAnn->setHostType( getHostType() );
            pktAnn->setHostOnlineId( m_Engine.getMyOnlineId() );
            pktAnn->setMyFriendshipToHim( eFriendStateGuest );
            pktAnn->setHisFriendshipToMe( eFriendStateGuest );
            m_Plugin.broadcastToClients( pktAnn, groupieId.getUserOnlineId(), sktBase, false);
            delete pktAnn;
        }
        else
        {
            LogMsg( LOG_ERROR, "HostServerMgr::onUserJoinedHost null pktAnn for user %s %s",
                    netIdent->getOnlineName(), groupieId.getUserOnlineId().toOnlineIdString().c_str() );
            vx_assert( false );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "HostServerMgr::onUserJoinedHost null bigListInfo for user %s %s",
                netIdent->getOnlineName(), groupieId.getUserOnlineId().toOnlineIdString().c_str() );
        vx_assert( false );
    }   
}

//============================================================================
void HostServerMgr::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    onUserLeftHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogMembership );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_VERBOSE, "HostServerMgr::onUserLeftHost %s", m_Engine.describeGroupieId(groupieId).c_str() );

    m_ClientListMutex.lock();
    m_ClientList.removeGuid( groupieId.getUserOnlineId() );
    m_ClientListMutex.unlock();
    
    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    m_Engine.getHostJoinMgr().onHostLeftByUser( sktBase, netIdent, sessionInfo );
    m_Engine.getGroupieListMgr().onHostLeftByUser( sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_ClientListMutex.lock();
    m_ClientList.removeGuid( groupieId.getUserOnlineId() );
    m_ClientListMutex.lock();

    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogMembership );
    if( LogEnabled( hostLogModule ) )LogModule( hostLogModule, LOG_VERBOSE, "HostServerMgr::onUserUnJoinedHost %s", m_Engine.describeGroupieId(groupieId).c_str() );

    sessionInfo.setJoinState( eJoinStateJoinLeaveHost );
    m_Engine.getHostJoinMgr().onHostLeftByUser( sktBase, netIdent, sessionInfo );
    m_Engine.getGroupieListMgr().onHostLeftByUser( sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onGroupDirectUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostServerMgr::onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::%s", __func__ );
    PktHostUserInfoReq* pktReq = (PktHostUserInfoReq*)pktHdr;
    VxGUID srcOnlineId = pktReq->getSrcOnlineId();
    if( pktReq->isValidPktPrefix() )
    {
        bool userOnline{ false };
        PktHostUserInfoReply pktReply;
        pktReply.setSessionId( pktReq->getSessionId() );
        pktReply.setGroupieId( pktReq->getGroupieId() );
        VxGUID onlineId = pktReq->getGroupieId().getUserOnlineId();
        if( isMemberOnline( onlineId ) )
        {        
            BigListInfo* bigListInfo = m_Engine.getBigListMgr().findBigListInfo( pktReq->getGroupieId().getUserOnlineId() );
            if( bigListInfo )
            {
                PktAnnounce* userAnn = bigListInfo->makeAnnCopy();
                if( userAnn )
                {
                    userAnn->setHostId( getHostId() );
                    userOnline = userAnn->addToBlob( pktReply.getBlobEntry() );
                    if( LogEnabled( eLogHostedUser ) )LogModule( eLogHostedUser, LOG_ERROR, "HostServerMgr::%s send ann %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
                }
            }
        }

        if( !userOnline )
        {
            if( LogEnabled( eLogHostedUser ) )LogModule( eLogHostedUser, LOG_ERROR, "HostServerMgr::%s not online %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
            pktReply.setCommError( eCommErrNotFound );
        }

        pktReply.calcPktLen();   
        m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply);
    }
}

//============================================================================
void HostServerMgr::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserInfoReply" );
}

//============================================================================
void HostServerMgr::onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserStatusReq" );
}

//============================================================================
void HostServerMgr::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserStatusReply" );
}

//============================================================================
void HostServerMgr::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserListReq" );
    VxGUID requestorOnlineId = netIdent->getMyOnlineId();
    PktHostUserListReq* pktReq = ( PktHostUserListReq * )pktHdr;
    if( pktReq->isValidPktPrefix() )
    {
        PktHostUserListReply pktReply;
        if( pktReq->getHostType() != getHostType() )
        {
            LogMsg( LOG_ERROR, "HostServerMgr::onPktHostUserListReq invalid host type" );
            pktReply.setCommError( eCommErrInvalidHostType );
        }
        else
        {
            pktReply.setHostType( getHostType() );
            pktReply.setHostOnlineId( pktReq->getHostOnlineId() );
            pktReply.setSearchSessionId( pktReq->getSearchSessionId() );
            std::vector<VxGUID> onlineIdList;
            HostedId hostId( m_Engine.getMyOnlineId(), getHostType() );
            m_Engine.getConnectIdListMgr().getOnlineMembers( hostId, onlineIdList );
            PktBlobEntry& blobEntry = pktReply.getBlobEntry();
            blobEntry.resetWrite();
            bool writeResult{ true };
            for( auto onlineId : onlineIdList )
            {
                if( onlineId == requestorOnlineId )
                {
                    continue;
                }

                if(LogEnabled( eLogHostedUser ))LogModule( eLogHostedUser, LOG_ERROR, "HostServerMgr::%s adding %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
                if( blobEntry.getRemainingStorageLen() >= sizeof( VxGUID ) )
                {
                    writeResult &= blobEntry.setValue( onlineId );
                    if( writeResult )
                    {
                        pktReply.incrementHostUserCount();
                    }
                    else
                    {
                        LogMsg( LOG_ERROR, "HostServerMgr::onPktHostUserListReq failed blob write" );
                        pktReply.setCommError( eCommErrInvalidParam );
                        break;
                    }
                }
                else
                {
                    if( LogEnabled( eLogHostedUser ) )LogModule( eLogHostedUser, LOG_ERROR, "HostServerMgr::%s no room for %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
                    pktReply.setMoreHostUsersExist( true );
                    pktReply.setNextSearchOnlineId( onlineId );
                    break;
                }
            }
        }

        pktReply.calcPktLen();
        if( !m_Plugin.txPacket( requestorOnlineId, sktBase, &pktReply ) )
        {
            LogMsg( LOG_ERROR, "HostServerMgr::onPktHostUserListReq failed to send" );
        }
    }
}

//============================================================================
void HostServerMgr::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserListReply" );
}

//============================================================================
void HostServerMgr::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::onPktHostUserListMoreReq" );
    VxGUID requestorOnlineid = netIdent->getMyOnlineId();
    PktHostUserListMoreReq* pktReq = (PktHostUserListMoreReq*)pktHdr;
    if( pktReq->isValidPktPrefix() )
    {
        PktHostUserListMoreReply pktReply;
        if( pktReq->getHostType() != getHostType() )
        {
            LogMsg( LOG_ERROR, "HostServerMgr::%s invalid host type", __func__ );
            pktReply.setCommError( eCommErrInvalidHostType );
        }
        else
        {
            pktReply.setHostType( getHostType() );
            pktReply.setHostOnlineId( pktReq->getHostOnlineId() );
            pktReply.setSearchSessionId( pktReq->getSearchSessionId() );
            std::vector<VxGUID> onlineIdList;
            HostedId hostId( m_Engine.getMyOnlineId(), getHostType() );
            m_Engine.getConnectIdListMgr().getOnlineMembers( hostId, onlineIdList );
            PktBlobEntry& blobEntry = pktReply.getBlobEntry();
            blobEntry.resetWrite();
            bool writeResult{ true };
            bool foundSearchId{ false };
            VxGUID searchId = pktReq->getNextSearchOnlineId();
            for( auto onlineId : onlineIdList )
            {
                if( onlineId == searchId )
                {
                    foundSearchId = true;
                }

                if( onlineId == requestorOnlineid )
                {
                    continue;
                }

                if( foundSearchId )
                {
                    if( blobEntry.getRemainingStorageLen() >= sizeof( VxGUID ) )
                    {
                        if( LogEnabled( eLogHostedUser ) )LogModule( eLogHostedUser, LOG_ERROR, "HostServerMgr::%s adding %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
                        writeResult &= blobEntry.setValue( onlineId );
                        if( writeResult )
                        {
                            pktReply.incrementHostUserCount();
                        }
                        else
                        {
                            LogMsg( LOG_ERROR, "HostServerMgr::%s failed blob write", __func__ );
                            pktReply.setCommError( eCommErrInvalidParam );
                            break;
                        }
                    }
                    else
                    {
                        pktReply.setMoreHostUsersExist( true );
                        pktReply.setNextSearchOnlineId( onlineId );
                        break;
                    }
                }
            }
        }

        pktReply.calcPktLen();
        if( !m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply) )
        {
            LogMsg( LOG_ERROR, "HostServerMgr::%s failed to send", __func__ );
        }
    }
}

//============================================================================
void HostServerMgr::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostServerMgr::%s", __func__ );
}

//============================================================================
bool HostServerMgr::isMemberOnline( VxGUID& onlineId )
{
    return m_Engine.getConnectIdListMgr().isMemberOnline( getHostId(), onlineId );
}

//============================================================================
bool HostServerMgr::sendMemberListToClient( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_ClientListMutex.lock();
    std::vector<VxGUID> memberList = m_ClientList.getGuidList();
    m_ClientListMutex.unlock();

    for( auto& onlineId : memberList )
    {
        if( onlineId == netIdent->getMyOnlineId() )
        {
            continue;
        }

        if( m_Engine.getConnectIdListMgr().isUserOnline( onlineId ) )
        {
            BigListInfo* bigListInfo = m_Engine.getBigListMgr().findBigListInfo( onlineId );
            if( bigListInfo )
            {
                PktAnnounce* pktAnn = bigListInfo->makeAnnCopy();
                if( pktAnn )
                {
                    // send users pkt ann to all clients
                    pktAnn->clearTempValues();
                    pktAnn->setHostType( getHostType() );
                    pktAnn->setHostOnlineId( m_Engine.getMyOnlineId() );
                    pktAnn->setMyFriendshipToHim( eFriendStateGuest );
                    pktAnn->setHisFriendshipToMe( eFriendStateGuest );
                    bool wasSent = m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, pktAnn);

                    delete pktAnn;
                    if( !wasSent )
                    {
                        break;
                    }
                }
                else
                {
                    LogMsg( LOG_ERROR, "HostServerMgr::onUserJoinedHost null pktAnn for user %s %s",
                            netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str() );
                    vx_assert( false );
                }
            }
            else
            {
                LogMsg( LOG_ERROR, "HostServerMgr::sendMemberListToClient null bigListInfo for online id %s",
                        onlineId.toOnlineIdString().c_str() );
                vx_assert( false );
            }
        }

        if( !sktBase->isConnected() )
        {
            break;
        }
    }  

    return sktBase->isConnected();
}

//============================================================================
bool HostServerMgr::isMember( VxGUID& onlineId, bool removeFromMembers )
{
    bool isMember{ false };
    m_ClientListMutex.lock();
    if( removeFromMembers )
    {
        isMember = m_ClientList.removeGuid( onlineId );
    }
    else
    {
        if( m_ClientList.doesGuidExist( onlineId ) )
        {
            isMember = true;
        }
    }

    m_ClientListMutex.unlock();;
 
    return isMember;
}
