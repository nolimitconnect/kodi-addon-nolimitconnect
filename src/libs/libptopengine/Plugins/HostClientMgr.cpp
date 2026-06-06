//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostClientMgr.h"
#include "PluginBase.h"

#include <GuiInterface/IToGui.h>
#include <BigListLib/BigListInfo.h>
#include <P2PEngine/P2PEngine.h>
#include <BaseInfo/BaseSessionInfo.h>
#include <Membership/MemberActiveMgr.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsHostSearch.h>
#include <PktLib/PluginIdList.h>
#include <PktLib/PktsHostUser.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>
#include <NetLib/VxSktBase.h>

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
HostClientMgr::HostClientMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase )
    : HostClientSearchMgr( engine, pluginMgr, myIdent, pluginBase )
{
}

//============================================================================
void HostClientMgr::onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostJoinReply* hostReply = (PktHostJoinReply*)pktHdr;
    if( hostReply->isValidPktPrefix() )
    {
        if( eHostTypeUnknown == hostReply->getHostType() || hostReply->getHostType() != getHostType() )
        {
            LogMsg( LOG_ERROR, "HostClientMgr::onPktHostJoinReply invalid host type" );
            return;
        }

        GroupieId groupieId( hostReply->getGroupieId() );
        if( !m_Engine.getConnectIdListMgr().addConnection( sktBase, groupieId, isRelayedFromHost( groupieId, netIdent ) ) )
        {
            LogMsg( LOG_ERROR, "HostClientMgr::onPktHostJoinReply addConnection failed" );
            return;
        }

        HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, hostReply->getSessionId() );

        if( ePluginAccessOk == hostReply->getAccessState() )
        {
            if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
            {
                m_Engine.getHostedListMgr().joinedToHostSuccess( groupieId.getHostedId() );
            }

            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );

            BaseSessionInfo sessionInfo( hostUserSessionId );
            onUserJoinHostGranted( groupieId, sktBase, netIdent, sessionInfo );
        }
        else if( ePluginAccessLocked == hostReply->getAccessState() )
        {
            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );

            m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFailPermission );
            if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
            {
                m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
            }
        }
        else
        {
            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );

            m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFail, DescribePluginAccess( hostReply->getAccessState() ) );
            if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
            {
                m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
            }
        }
    }
    else
    {
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );     
    }
}

//============================================================================
void HostClientMgr::onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if( !pktHdr->isValidPktPrefix() )
    {
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );
        return;
    }

    PktHostLeaveReply* hostReply = ( PktHostLeaveReply* )pktHdr;
    if( eHostTypeUnknown == hostReply->getHostType() || hostReply->getHostType() != getHostType() )
    {
        LogMsg( LOG_ERROR, "HostClientMgr::%s invalid host type", __func__ );
    }

    GroupieId groupieId( hostReply->getGroupieId() );

    if( LogEnabled( eLogMembership ) )LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseHostService::%s rxed groupie %s ", __func__,
                    m_Engine.describeGroupieId( groupieId ).c_str() );

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, hostReply->getSessionId() );

    if( ePluginAccessOk == hostReply->getAccessState() )
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinUnJoinSuccess );
        BaseSessionInfo sessionInfo( hostUserSessionId );
        onUserLeftHost( groupieId, sktBase, netIdent, sessionInfo );
    }
    else if( ePluginAccessLocked == hostReply->getAccessState() )
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFailPermission );
        if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
        {
            m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
        }
    }
    else
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFail, DescribePluginAccess( hostReply->getAccessState() ) );
        if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
        {
            m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
        }
    }

    m_Engine.getConnectIdListMgr().removeConnection( sktBase->getSocketId(), groupieId );
}

//============================================================================
void HostClientMgr::onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostUnJoinReply* hostReply = ( PktHostUnJoinReply* )pktHdr;
    if( hostReply->isValidPktPrefix() )
    {
        if( eHostTypeUnknown == hostReply->getHostType() || hostReply->getHostType() != getHostType() )
        {
            LogMsg( LOG_ERROR, "HostClientMgr::onPktHostUnJoinReply invalid host type" );
        }

        GroupieId groupieId( hostReply->getGroupieId() );
        m_Engine.getConnectIdListMgr().removeConnection( sktBase->getSocketId(), groupieId );
        HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, hostReply->getSessionId() );

        if( ePluginAccessOk == hostReply->getAccessState() )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinUnJoinSuccess );
            BaseSessionInfo sessionInfo( hostUserSessionId );
            onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
        }
        else if( ePluginAccessLocked == hostReply->getAccessState() )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFailPermission );
            if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
            {
                m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
            }
        }
        else
        {
            m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinFail, DescribePluginAccess( hostReply->getAccessState() ) );
            if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
            {
                m_Engine.getConnectionMgr().doneWithConnection( hostReply->getSessionId(), groupieId.getUserOnlineId(), this, HostTypeToConnectJoinReason( hostReply->getHostType() ) );
            }
        }
    }
    else
    {
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );
    }
}

//============================================================================
void HostClientMgr::onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostSearchReply* hostReply = ( PktHostSearchReply* )pktHdr;
    ECommErr commErr = hostReply->getCommError();
    if( 0 == hostReply->getTotalMatches() )
    {
        if( eCommErrNone != commErr )
        {
            if( eCommErrPluginNotEnabled == commErr )
            {
                m_Engine.getToGui().toGuiHostSearchStatus( hostReply->getHostType(), netIdent->getMyOnlineId(), eHostSearchPluginDisabled, hostReply->getCommError() );
            }
            else if( eCommErrPluginPermission == commErr )
            {
                m_Engine.getToGui().toGuiHostSearchStatus( hostReply->getHostType(), netIdent->getMyOnlineId(), eHostSearchFailPermission, hostReply->getCommError() );
            }
            else
            {
                m_Engine.getToGui().toGuiHostSearchStatus( hostReply->getHostType(), netIdent->getMyOnlineId(), eHostSearchFail, hostReply->getCommError() );
            }
        }
        else
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostReply->getHostType(), netIdent->getMyOnlineId(), eHostSearchNoMatches, hostReply->getCommError() );
        }

        LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::onPktHostSearchReply no matches" );
        stopHostSearch( hostReply->getHostType(), hostReply->getSearchSessionId(), sktBase, netIdent->getMyOnlineId() );
    }
    else
    {
        startHostDetailSession( hostReply, sktBase, netIdent );
    }
}

//============================================================================
void HostClientMgr::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    if( !netIdent->getIsJoined( groupieId.getHostType() ) )
    {
        netIdent->setIsJoined( groupieId.getHostType(), true );
        m_Engine.toGuiContactAnythingChange( netIdent );
    }

    ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogHostJoin );
    if(LogEnabled(hostLogModule))LogModule( hostLogModule, LOG_INFO, "HostClientMgr::%s joined %s", __func__,
              groupieId.describeGroupieId().c_str() );

    if(groupieId.getUserOnlineId() == m_Engine.getMyOnlineId())
    {
        setHostId( groupieId.getHostedId() );
    }

    m_ServerListMutex.lock();
    m_ServerList.insert( groupieId );
    m_ServerListMutex.unlock();

    m_Engine.getUserJoinMgr().onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
    //m_Engine.getUserOnlineMgr().onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
    m_Engine.getThumbMgr().queryThumbIfNeeded( sktBase, netIdent, sessionInfo.getHostPluginType() );

    if( sessionInfo.getJoineState() == eJoinStateJoinIsGranted )
    {
        m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );
    }
}

//============================================================================
void HostClientMgr::onUserJoinHostGranted( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    if( sktBase->isTempConnection() )
    {
        LogModule( eLogHostConnect, LOG_INFO, "HostClientMgr::%s temp connection", __func__ );
    }

    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );

    m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinSuccess );
    if( groupieId.getUserOnlineId() == m_Engine.getMyOnlineId() )
    {
        // for the case of host is also the network host add connect reason so is not treated as temp connection
        enum EConnectReason connectReason = hostTypeToConnectReason( groupieId.getHostType() );
        if( connectReason != eConnectReasonUnknown )
        {
            // tell gui to add online status so we know when disconnects
            sktBase->addConnectReason( connectReason ); 
            m_Engine.getConnectIdListMgr().addHostConnection( sktBase, groupieId );
        }       

        // request a list of everybody because we just joined
        clearUserInfoRequests();

        PktHostUserListReq pktReq;
        pktReq.setHostType( sessionInfo.getHostType() );
        pktReq.setHostOnlineId( sessionInfo.getHostOnlineId() );
        pktReq.setSearchSessionId( sessionInfo.getSessionId() );

        LogModule( eLogHostConnect, LOG_INFO, "HostClientMgr::%s to me", __func__ );
        if( !m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, &pktReq) )
        {
            LogModule( eLogHostConnect, LOG_INFO, "HostClientMgr::txPkt PktHostUserListReq failed" );
        }
    }
    else
    {
        // request info about the person who just joined
        PktHostUserInfoReq pktReq;
        pktReq.setGroupieId( groupieId );

        LogModule( eLogHostConnect, LOG_INFO, "HostClientMgr::onUserJoinHostGranted to other user" );
        if( !m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, &pktReq ) )
        {
            LogModule( eLogHostConnect, LOG_INFO, "HostClientMgr::txPkt PktHostUserInfoReq failed" );
        }
    }

    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );
}

//============================================================================
enum EConnectReason HostClientMgr::hostTypeToConnectReason( EHostType hostType )
{
    enum EConnectReason connectReason{ eConnectReasonUnknown };
    switch( hostType )
    {
    case eHostTypeChatRoom:
        connectReason = eConnectReasonChatRoomJoin;
        break;
    case eHostTypeGroup:
        connectReason = eConnectReasonGroupJoin;
        break;
    case eHostTypeRandomConnect:
        connectReason = eConnectReasonRandomConnectJoin;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
void HostClientMgr::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    m_ServerListMutex.lock();
    m_ServerList.erase( groupieId );
    m_ServerListMutex.unlock();

    m_Engine.getUserJoinMgr().onUserLeftHost( groupieId, sktBase, netIdent, sessionInfo );
    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );

    enum EConnectReason connectReason = hostTypeToConnectReason( groupieId.getHostType() );
    if( connectReason != eConnectReasonUnknown )
    {
        sktBase->removeConnectReason( connectReason );
    }   

    m_Engine.getConnectIdListMgr().removeConnection( sktBase->getSocketId(), groupieId );
}

//============================================================================
void HostClientMgr::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
{
    m_ServerListMutex.lock();
    m_ServerList.erase( groupieId );
    m_ServerListMutex.unlock();

    m_Engine.getUserJoinMgr().onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
    //m_Engine.getUserOnlineMgr().onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );

    enum EConnectReason connectReason = hostTypeToConnectReason( groupieId.getHostType() );
    if( connectReason != eConnectReasonUnknown )
    {
        sktBase->removeConnectReason( connectReason );
    }  
}

//============================================================================
void HostClientMgr::sendHostSearchToNetworkHost( VxGUID& sessionId, SearchParams& searchParams, EConnectReason connectReason )
{
    // save announce pkt in announce session list
    std::string url = m_ConnectionMgr.getDefaultHostUrl( eHostTypeNetwork );
    if( url.empty() )
    {
        LogMsg( LOG_VERBOSE, "HostServerMgr network host url is empty" );
        return;
    }

    addSearchSession( sessionId, searchParams );
    connectToHost( eHostTypeNetwork, sessionId, url, connectReason );
}

//============================================================================
void HostClientMgr::addPluginRxSession( VxGUID& sessionId, PluginIdList& pluginIdList )
{
    removePluginRxSession( sessionId );
    m_PluginRxListMutex.lock();
    m_PluginRxList[sessionId] = pluginIdList;
    m_PluginRxListMutex.unlock();
}

//============================================================================
void HostClientMgr::removePluginRxSession( VxGUID& sessionId )
{
    m_PluginRxListMutex.lock();
    auto iter = m_PluginRxList.find( sessionId );
    if( iter != m_PluginRxList.end() )
    {
        m_PluginRxList.erase( iter );
    }

    m_PluginRxListMutex.unlock();
}

//============================================================================
void HostClientMgr::removeSession( VxGUID& sessionId, EConnectReason connectReason )
{
    if( isSearchConnectReason( connectReason ) )
    {
        removeSearchSession( sessionId );
    }

    HostClientSearchMgr::removeSession( sessionId, connectReason );
}

//============================================================================
void HostClientMgr::onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason )
{
    GroupieId groupieId( m_Engine.getMyOnlineId(), onlineId, getHostType() );
    m_ServerList.erase( groupieId );
    removeContact( onlineId );
}

//============================================================================
bool HostClientMgr::onConnectToHostSuccess( EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason )
{
    bool result{ false };
    if( isSearchConnectReason( connectReason ) )
    {
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchConnectSuccess );
    }

    if( hostType == eHostTypeNetwork &&
        ( connectReason == eConnectReasonChatRoomSearch ||
            connectReason == eConnectReasonGroupSearch ||
            connectReason == eConnectReasonRandomConnectSearch ) )
    {
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchSendingSearchRequest );

        m_SearchParamsMutex.lock();
        auto iter = m_SearchParamsList.find( sessionId );
        if( iter != m_SearchParamsList.end() )
        {
            SearchParams& searchParams = iter->second;
            PktHostSearchReq searchReq;
            searchReq.setHostType( searchParams.getHostType() );
            searchReq.setPluginType( m_Plugin.getPluginType() );
            searchReq.setSearchSessionId( sessionId );
            PktBlobEntry& blobEntry = searchReq.getBlobEntry();
            bool result = searchParams.addToBlob( blobEntry );
            searchReq.calcPktLen();
            // unlock before txPacket else in looback mode can cause a deadlock
            m_SearchParamsList.erase( iter );
            m_SearchParamsMutex.unlock();

            if( result && searchReq.isValidPktPrefix() )
            {
                if( !m_Plugin.txPacket( onlineId, sktBase, &searchReq, m_Plugin.getDestinationPluginOverride( hostType ) ) )
                {
                    LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::onConnectToHostSuccess failed send PktHostSearchReq" );
                    stopHostSearch( hostType, sessionId, sktBase, onlineId );
                    return false;
                }
            }
            else
            {
                LogMsg( LOG_ERROR, "HostServerMgr PktHostSearchReq is invalid" );
                stopHostSearch( hostType, sessionId, sktBase, onlineId );
                return false;
            }

            // not done with connection.. wait for search results
        }
        else
        {
            m_SearchParamsMutex.unlock();
            LogMsg( LOG_ERROR, "HostServerMgr Search Params Not Found" );
            stopHostSearch( hostType, sessionId, sktBase, onlineId );
        }     
    }
    else
    {
        HostBaseMgr::onConnectToHostSuccess( hostType, sessionId, sktBase, onlineId, connectReason );
    }

    return result;
}

//============================================================================
void HostClientMgr::startHostDetailSession( PktHostSearchReply* hostReply, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    EHostType hostType = hostReply->getHostType();
    VxGUID sessionId = hostReply->getSearchSessionId();
    int pluginIdCnt = hostReply->getTotalMatches();
    ECommErr commErr = hostReply->getCommError();
    if( eCommErrNone != commErr )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::startHostDetailSession comm error %s", DescribeCommError( commErr ) );
        if( eCommErrPluginNotEnabled == commErr )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, netIdent->getMyOnlineId(), eHostSearchPluginDisabled );
        }

        stopHostSearch( hostReply->getHostType(), hostReply->getSearchSessionId(), sktBase, netIdent->getMyOnlineId() );
        return;
    }

    if( !sessionId.isValid() )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::startHostDetailSession session id invalid");
    }

    bool result = sessionId.isValid() && pluginIdCnt > 0 && eCommErrNone == commErr;
    if( result )
    {
        // insert ids and send first request for plugin settings
        PluginIdList pluginIdList;
        PktBlobEntry& blobEntry = hostReply->getBlobEntry();
        blobEntry.resetRead();
        for( int i = 0; i < pluginIdCnt; i++ )
        {
            PluginId pluginId;
            if( blobEntry.getValue( pluginId ) )
            {
                pluginIdList.addPluginId( pluginId );
            }
            else
            {
                LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::startHostDetailSession error getting plug id at index %d", i);
                result = false;
                break;
            }
        }

        if( result )
        {
            addPluginRxSession( sessionId, pluginIdList );
        }
    }

    if( !result )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostClientMgr::startHostDetailSession failed");
        stopHostSearch( hostReply->getHostType(), hostReply->getSearchSessionId(), sktBase, netIdent->getMyOnlineId() );
    }
}

//============================================================================
bool HostClientMgr::stopHostSearch( EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    m_Engine.getToGui().toGuiHostSearchStatus( hostType, onlineId, eHostSearchCompleted );
    m_Engine.getToGui().toGuiHostSearchComplete( hostType, onlineId );

    removeSearchSession( sessionId );
    EConnectReason connectReason = getSearchConnectReason(hostType);
    m_Engine.getConnectionMgr().doneWithConnection( sessionId, onlineId, this, connectReason );
    return true;
}

//============================================================================
void HostClientMgr::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostClientMgr::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    onUserLeftHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostClientMgr::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    onUserUnJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostClientMgr::onGroupRelayedUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    LogMsg( LOG_VERBOSE, "HostClientMgr::onGroupRelayedUserAnnounce from %s id %s hosted by %s id %s groupieId %s",
            netIdent->getOnlineName(), netIdent->getMyOnlineId().describeVxGUID().c_str(), sktBase->getPeerOnlineName().c_str(),
        sktBase->getPeerOnlineId().describeVxGUID().c_str(), groupieId.describeGroupieId().c_str());

    m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), groupieId.getUserOnlineId(), eHostJoinSuccess );
    VxGUID sessionId;
    sessionId.initializeWithNewVxGUID();

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    onUserJoinedHost( groupieId, sktBase, netIdent, sessionInfo );
}

//============================================================================
void HostClientMgr::onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_ERROR, "HostClientMgr::onPktHostUserInfoReq" ); // should not recieve this.. should we do hack attempt?
}

//============================================================================
void HostClientMgr::onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::onPktHostUserStatusReq" );
}

//============================================================================
void HostClientMgr::onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::onPktHostUserStatusReply" );
}

//============================================================================
void HostClientMgr::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::onPktHostUserListReq" );
}

//============================================================================
void HostClientMgr::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::%s", __func__ );
    PktHostUserListReply* pktReply = (PktHostUserListReply*)pktHdr;
    std::set<VxGUID> onlineIds;
    if( !pktReply->isValidPktPrefix() )
    {
        LogMsg( LOG_ERROR, "HostClientMgr::%s invalid pktReply->isValidPktPrefix()", __func__ );
        return;
    }

    PktBlobEntry& blobEntry = pktReply->getBlobEntry();
    blobEntry.resetRead();

    int userCnt = pktReply->getHostUserCountThisPkt();
    ELogModule hostLogModule = HostTypeLogModule( pktReply->getHostType(), eLogHostJoin );
    if(LogEnabled(hostLogModule))LogModule( hostLogModule, LOG_VERBOSE, "HostClientMgr::%s member cnt %d host type %s", __func__, userCnt, DescribeHostType( pktReply->getHostType() ) );

    for( int i = 0; i < userCnt; i++ )
    {
        VxGUID onlineId;
        if( !blobEntry.getValue( onlineId ) )
        {
            LogMsg( LOG_ERROR, "HostClientMgr::%s failed read online id", __func__ );
            return;
        }

        if(LogEnabled(hostLogModule))LogModule( hostLogModule, LOG_VERBOSE, "HostClientMgr::%s list item %d - user %s", __func__,
                        i + 1, m_Engine.describeUser( onlineId ).c_str() );

        if( !onlineId.isValid() )
        {
            LogMsg( LOG_ERROR, "HostClientMgr::%s invalid online id", __func__ );
            continue;
        }

        if( onlineId == netIdent->getMyOnlineId() )
        {
            LogMsg( LOG_VERBOSE, "HostClientMgr::%s host cannot also be a member", __func__ );
            continue;
        }

        if( onlineId == m_Engine.getMyOnlineId() )
        {
            continue;
        }

        onlineIds.insert( onlineId );
    }

    GroupieId groupieId( netIdent->getMyOnlineId(), netIdent->getMyOnlineId(), getHostType());
    ConnectId connectId( sktBase->getSocketId(), groupieId );
    connectId.setIsRelayed( true );

    bool firstOnlineIdIsSet{ false };
    VxGUID firstOnlineId;
    for( auto onlineId : onlineIds )
    {
        connectId.setUserOnlineId( onlineId );
        if( m_Engine.getConnectIdListMgr().addUnconfirmedConnection( connectId ) )
        {
            if( !firstOnlineIdIsSet )
            {
                firstOnlineIdIsSet = true;
                firstOnlineId = onlineId;
            }

            m_ClientMutex.lock();
            m_UserInfoReqList.addGuidIfDoesntExist( onlineId );
            m_ClientMutex.unlock();
        }
    }

    if( firstOnlineIdIsSet )
    {
        sendNextUserInfoRequest( sktBase, netIdent, firstOnlineId, pktReply->getSearchSessionId() );
    }
}

//============================================================================
void HostClientMgr::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::$s", __func__ );
}

//============================================================================
void HostClientMgr::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::%s", __func__ );
}

//============================================================================
void HostClientMgr::sendNextUserInfoRequest( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdentHost, VxGUID& onlineId, VxGUID& sessionId )
{
    PktHostUserInfoReq pktReq;
    GroupieId groupieId( onlineId, netIdentHost->getMyOnlineId(), getHostType() );
    if(LogEnabled(eLogOnline))LogModule( eLogOnline, LOG_VERBOSE, "HostClientMgr::%s requesting info for user %s", __func__,
                      m_Engine.describeUser( onlineId ).c_str() );

    pktReq.setGroupieId( groupieId );
    pktReq.setSessionId( sessionId );

    if( !m_Plugin.txPacket( netIdentHost->getMyOnlineId(), sktBase, &pktReq) )
    {
        clearLastHostUserInfoRequestTime();
        LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s failed send", __func__ );
    }
    else
    {
        ELogModule hostLogModule = HostTypeLogModule( groupieId.getHostType(), eLogHostJoin );
        if(LogEnabled(hostLogModule))LogModule( hostLogModule, LOG_VERBOSE, "HostClientMgr::%s sent user %s groupie %s", __func__,
                      m_Engine.describeUser( onlineId ).c_str(), groupieId.describeGroupieId().c_str() );
    }
}

//============================================================================
void HostClientMgr::clearUserInfoRequests( void )
{
    LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s", __func__ );
    m_ClientMutex.lock();
    m_UserInfoReqList.clearList();
    m_SentUserInfoReqTime = 0;
    m_ClientMutex.unlock();
}

//============================================================================
void HostClientMgr::onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "HostClientMgr::%s", __func__ );
    PktHostUserInfoReply* pktReply = (PktHostUserInfoReply*)pktHdr;

    if( !pktReply->isValidPktPrefix() )
    {
        LogMsg( LOG_ERROR, "HostClientMgr::%s invalid packet", __func__ );
        return;
    }

    VxGUID srcOnlineId = pktReply->getSrcOnlineId();
    clearLastHostUserInfoRequestTime();

    VxGUID groupieUserId = pktReply->getGroupieId().getUserOnlineId();

    m_ClientMutex.lock();
    m_UserInfoReqList.removeGuid( groupieUserId );
    bool userInfoReqListEmpty = m_UserInfoReqList.isEmpty();
    VxGUID nextOnlineId = m_UserInfoReqList.getAnyGuid();
    m_ClientMutex.unlock();

    if(!userInfoReqListEmpty)
    {
        if(LogEnabled(eLogHostedUser))LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s next online id %s", __func__,
                        m_Engine.describeUser( nextOnlineId ).c_str() );
    }

    PktBlobEntry& blobEntry = pktReply->getBlobEntry();
    blobEntry.resetRead();
    if( blobEntry.getBlobLen() && !pktReply->getCommError() )
    {
        PktAnnounce pktAnn;
        bool readResult = pktAnn.extractFromBlob( blobEntry );
        if( readResult )
        {
            VxGUID memberId = pktAnn.getMyOnlineId();
            if(LogEnabled(eLogHostedUser))LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s extracted user %s, next user %s", __func__,
                            m_Engine.describeUser( memberId ).c_str(),   m_Engine.describeUser( nextOnlineId ).c_str() );

            if( memberId != groupieUserId )
            {
                LogMsg( LOG_ERROR, "HostClientMgr::%s groupieUserId is %s but pktAnn id %s name %s ", __func__,
                        groupieUserId.toOnlineIdString().c_str(), memberId.toOnlineIdString().c_str(), pktAnn.getOnlineName() );
            }

            // just to make sure has low friendship privilege until we get a real announce packet from user
            pktAnn.setHisFriendshipToMe( eFriendStateGuest );
            pktAnn.setMyFriendshipToHim( eFriendStateGuest );

            announceUserInfo( sktBase, &pktAnn, pktReply->getSessionId(), srcOnlineId, pktReply->getHostType() );
        }
        else
        {
            LogMsg( LOG_ERROR, "HostClientMgr::%s failed extract PktAnn", __func__ );
        }
    }
    else
    {
        if( pktReply->getCommError() )
        {
            LogMsg( LOG_INFO, "HostClientMgr::%s comm error %s", __func__,
                    DescribeCommError( pktReply->getCommError() ) );
        }
        else
        {
            LogMsg( LOG_ERROR, "HostClientMgr::%s 0 length blobEntry", __func__ );
        }
    }

    if( !userInfoReqListEmpty )
    {
        sendNextUserInfoRequest( sktBase, netIdent, nextOnlineId, pktReply->getSessionId() );
    }
    else if(LogEnabled(eLogHostedUser))
    {
        LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s list is empty", __func__ );
    }
}

//============================================================================
void HostClientMgr::announceUserInfo( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, VxGUID& sessionId, VxGUID& hostOnlineId, EHostType hostType )
{
    VxGUID memberOnlineId = pktAnn->getMyOnlineId();
    GroupieId groupieId( memberOnlineId, hostOnlineId, hostType );
    if(LogEnabled(eLogHostedUser))LogModule( eLogHostedUser, LOG_VERBOSE, "HostClientMgr::%s for user %s", __func__,
                  m_Engine.describeUser( memberOnlineId ).c_str() );
    if( m_Engine.getMyOnlineId() == memberOnlineId )
    {
        LogMsg( LOG_VERBOSE, "HostClientMgr::%s got myself.. ERROR ?", __func__ );
        return;
    }

    if( m_Engine.getBigListMgr().isUserIgnored( memberOnlineId ) )
    {
        LogMsg( LOG_VERBOSE, "HostClientMgr::%s ignored user %s %s", __func__, 
            pktAnn->getOnlineName(), memberOnlineId.toOnlineIdString().c_str() );
        return;
    }

    if( IsHostARelayForUsers( hostType ) )
    {
        // this is someone not in our database so start with guest friendships because is a member of the host we joined to  
        BigListInfo* bigListInfo = 0;
        // the pktAnn does not have the correct friendship .. from host
        bigListInfo = m_Engine.getBigListMgr().findBigListInfo( pktAnn->getMyOnlineId() );
        if( !bigListInfo )
        {
            // this user has not been encountered. add to big list as temporary until we get a actual pkt announce from user
            EPktAnnUpdateType updateType = m_Engine.getBigListMgr().updatePktAnn( pktAnn, &bigListInfo, hostType );
            if( !bigListInfo || !bigListInfo->isValidNetIdent() )
            {
                LogMsg( LOG_ERROR, "HostClientMgr::%s INVALID", __func__ );
                return;
            }
        }


        if( bigListInfo->isIgnored() )
        {
            LogMsg( LOG_INFO, "HostClientMgr::%s ignored user %s", __func__,
                   m_Engine.describeUser( memberOnlineId ).c_str() );
            return;
        }

        bigListInfo->setIsJoined( getHostType(), true );

        if( !m_Engine.getConnectIdListMgr().isUserOnline( pktAnn->getMyOnlineId() ) )
        {

            // we need to exchange PktAnn to get current friendship from peer user
            // also when pktAnn is recieved it will confirm the connection
            ConnectId connectId( sktBase->getSocketId(), groupieId );
            bool isRelayed = groupieId.getUserOnlineId() != sktBase->getPeerOnlineId();
            connectId.setIsRelayed( isRelayed );
            m_Engine.getConnectIdListMgr().addUnconfirmedConnection( connectId );

               if(LogEnabled(eLogUsers))LogModule( eLogUsers, LOG_VERBOSE, "HostClientMgr::%s sending pktann to %s", __func__,
                          m_Engine.describeUser( memberOnlineId ).c_str() );

            // send our pkt ann through host to peer user and request response pkt ann
            m_Engine.getConnectionMgr().sendMyPktAnnounce( memberOnlineId, sktBase, true, false, false, false );
        }

        pktAnn = bigListInfo;
    }
    else
    {
        LogMsg( LOG_ERROR, "HostClientMgr::%s NOT a relay Host %s", __func__, DescribeHostType( hostType ) );
    }

    HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
    BaseSessionInfo sessionInfo( hostUserSessionId );
    sessionInfo.setOnlineState( eOnlineStateOnline );
    sessionInfo.setJoinState( eJoinStateJoinIsGranted );

    onUserJoinedHost( groupieId, sktBase, pktAnn, sessionInfo );
}

