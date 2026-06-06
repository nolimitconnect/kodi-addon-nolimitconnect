//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseNetworkService.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <Membership/MemberActiveMgr.h>

#include <CoreLib/Invite.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsHostSearch.h>
#include <PktLib/PktsHostInfo.h>
#include <PktLib/PktsGroupie.h>

//============================================================================
PluginBaseNetworkService::PluginBaseNetworkService( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
    : PluginNetServices( engine, pluginMgr, myIdent, pluginType )
    , m_HostServerMgr(engine, pluginMgr, myIdent, *this)
{
}

//============================================================================
EPluginType PluginBaseNetworkService::getClientPluginType( void )
{
    return HostPluginToClientPluginType( getPluginType() );
}

//============================================================================
bool PluginBaseNetworkService::getHostedInfo( HostedInfo& hostedInfo )
{
    bool result = false;
    if( !m_HostInviteUrl.empty()  && !m_HostTitle.empty() && !m_HostDescription.empty() && m_HostInfoModifiedTime )
    {
        m_AnnMutex.lock();
        hostedInfo.setHostInfoTimestamp( m_HostInfoModifiedTime );
        hostedInfo.setHostInviteUrl( m_HostInviteUrl );
        hostedInfo.setHostTitle( m_HostTitle );
        hostedInfo.setHostDescription( m_HostDescription );
        hostedInfo.setAdminId( getHostedId() );
        VxGUID thumbId = m_Engine.getMyPktAnnounce().getHostThumbId( getHostType(), true );
        hostedInfo.setThumbId( thumbId );

        m_AnnMutex.unlock();
        result = hostedInfo.isValidForGui();
    }

    return result;
}

//============================================================================
bool PluginBaseNetworkService::setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs )
{
    bool result = PluginNetServices::setPluginSetting( pluginSetting, modifiedTimeMs );
    if( isPluginEnabled() && PluginShouldAnnounceToNetwork( getPluginType() ) )
    {
        buildHostAnnounce( pluginSetting );
        sendHostAnnounce();
    }

    return result;
}

//============================================================================
void PluginBaseNetworkService::onPluginSettingChange( PluginSetting& pluginSetting, int64_t modifiedTimeMs )
{
    if( isPluginEnabled() && PluginShouldAnnounceToNetwork( getPluginType() ) )
    {
        updateHostInvite( pluginSetting );
    }

    onPluginSettingsChanged( pluginSetting.getLastUpdateTimestamp() );
}

//============================================================================
void PluginBaseNetworkService::onThreadOncePer15Minutes( void )
{
    if( isPluginEnabled() && PluginShouldAnnounceToNetwork( getPluginType() ) )
    {
        sendHostAnnounce();
    }
}

//============================================================================
void PluginBaseNetworkService::onMyOnlineUrlIsValid( bool isValidUrl )
{
    if( isValidUrl )
    {
        sendHostAnnounce();
    }
    else
    {
        m_PktHostInviteIsValid = false;
    }
}

//============================================================================
void PluginBaseNetworkService::buildHostAnnounce( PluginSetting& pluginSetting )
{
    updateHostInvite( pluginSetting );
}

//============================================================================
void PluginBaseNetworkService::sendHostAnnounce( void )
{
    if( !isPluginEnabled() )
    {
        return;
    }

    if( getPluginType() == ePluginTypeHostNetwork ||  getPluginType() == ePluginTypeHostConnectTest )
    {
        //LogMsg( LOG_ERROR, "PluginBaseNetworkService::%s do NOT announce %s", __func__, DescribePluginType( getPluginType() ) );
        return;
    }

    if( m_Engine.getMyPktAnnounce().requiresRelay() )
    {
        VxGUID sessionId;
        m_Engine.getToGui().toGuiHostAnnounceStatus( getHostType(), sessionId, eHostAnnounceFailRequiresOpenPort, "Announce Host Requires An Open Port" );
        return;
    }

    if( !m_PktHostInviteIsValid && 
        ( m_Engine.getEngineSettings().getFirewallTestSetting() == eFirewallTestAssumeNoFirewall || // assume no firewall means extern ip should be set
            m_Engine.getNetStatusAccum().isDirectConnectTested() ) ) // isDirectConnectTested means my url should be valid
    {
        PluginSetting pluginSetting;
        if( m_Engine.getPluginSettingMgr().getPluginSetting( getPluginType(), pluginSetting ) )
        {
            buildHostAnnounce( pluginSetting );
        }
    }

    if( !m_PktHostInviteIsValid )
    {
        return;
    }

    bool sentToOurself = false;
    if( m_Engine.isNetworkHostEnabled() )
    {
        std::string netHostUrlStr = m_Engine.fromGuiQueryDefaultUrl( eHostTypeNetwork );
        VxPtopUrl netHostUrl( netHostUrlStr );
        if( netHostUrl.isValid() && netHostUrl.getOnlineId() == m_Engine.getMyOnlineId() )
        {
            // if we are also network host then send to ourself also
            PluginBase* netHostPlugin = m_PluginMgr.getPlugin( ePluginTypeHostNetwork );
            if( netHostPlugin )
            {
                m_AnnMutex.lock();
                netHostPlugin->updateHostSearchList( m_PktHostInviteAnnounceReq.getHostType(), &m_PktHostInviteAnnounceReq, m_MyIdent, m_Engine.getSktLoopback() );
                m_AnnMutex.unlock();
                sentToOurself = true;
            }
        }
    }
    
    if( m_Engine.getNetStatusAccum().canAnnounceToNlcHost( m_Engine.isNetworkHostEnabled() ) )
    {
        VxGUID::generateNewVxGUID( m_AnnounceSessionId );
        m_HostServerMgr.sendHostAnnounceToNetworkHost( m_AnnounceSessionId, m_PktHostInviteAnnounceReq, getHostAnnounceConnectReason() );
    }
}

//============================================================================
void PluginBaseNetworkService::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
//    m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginBaseNetworkService::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
//    m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
//    m_FriendGuidList.removeGuid( netIdent->getMyOnlineId() );
}

//============================================================================
void PluginBaseNetworkService::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
//    m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginBaseNetworkService::onPktHostJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogModule( eLogHostJoin, LOG_VERBOSE, "PluginBaseNetworkService PktHostJoinReq %s got PktHostJoinReq from %s %s", DescribeHostType( getHostType() ), 
               netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str() );
    PktHostJoinReq * joinReq = (PktHostJoinReq *)pktHdr;
    PktHostJoinReply joinReply;
    if( joinReq->isValidPktPrefix() )
    {
        GroupieId rxedGroupieId = joinReq->getGroupieId();
        GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), getHostType() );
        if( rxedGroupieId != groupieId )
        {
            LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService::onPktHostJoinReq %s from %s groupie %s does not match rxed %s", DescribePluginType( getPluginType() ), 
                netIdent->getOnlineName(), groupieId.describeGroupieId().c_str(), rxedGroupieId.describeGroupieId().c_str() );
        }

        joinReply.setGroupieId( groupieId );
        joinReply.setPluginType( getPluginType() );
        joinReply.setSessionId( joinReq->getSessionId() );
        joinReply.setAccessState( m_HostServerMgr.getPluginAccessState( netIdent ) );

        if( !sktBase->isConnected() )
        {
            LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService::onPktHostJoinReq %s from %s groupie %s socket was close", DescribePluginType( getPluginType() ), 
                netIdent->getOnlineName(), groupieId.describeGroupieId().c_str() );
            return;
        }

        VxGUID sktConnectionId( sktBase->getSocketId() );

        bool broadcastPkt = false;
        bool sendPkt = false;
        if( ePluginAccessOk == joinReply.getAccessState() )
        {
            broadcastPkt = true;
            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );
        }
        else if( ePluginAccessLocked == joinReply.getAccessState() )
        {
            if( !netIdent->isIgnored() )
            {
                if( m_HostServerMgr.getJoinState( netIdent, joinReq->getHostType() ) == eJoinStateJoinWasGranted ||
                    m_HostServerMgr.getJoinState( netIdent, joinReq->getHostType() ) == eJoinStateJoinIsGranted )
                {
                    broadcastPkt = true;
                    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, true );
                    // even though friendship not high enough if admin has accepted then send accepted
                    joinReply.setAccessState( ePluginAccessOk );
                    LogModule( eLogHostJoin, LOG_VERBOSE, "PluginBaseNetworkService from %s %s host %s %s join granted", 
                               netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str(), DescribeHostType( getHostType() ) );
                }
                else
                {
                    m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );
                    sendPkt = true;
                    LogModule( eLogHostJoin, LOG_VERBOSE, "PluginBaseNetworkService from %s %s host %s %s join requested", 
                               netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str(), DescribeHostType( getHostType() ) );
                }
            }
            else
            {
                // TODO .. should we drop the connection or just ignore ?
            }
        }
        else if( ePluginAccessDisabled == joinReply.getAccessState() )
        {
            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );
            // join request sent to disabled plugin.. this should not happen
            LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got join request to disabled plugin from %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
        }
        else if( ePluginAccessIgnored == joinReply.getAccessState() )
        {
            m_Engine.getMemberActiveMgr().updateMemberActive( groupieId, false );
            // TODO .. should we drop the connection of ignored person?
            LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got join request from ignored person %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
        }

        if( broadcastPkt )
        {
            if( m_Engine.getConnectIdListMgr().updateUserJoinedFriendships( groupieId, netIdent ) )
            {
                m_Engine.getConnectIdListMgr().addConnection( sktBase, groupieId, false );
                if( m_HostServerMgr.onUserJoined( sktBase, netIdent, joinReply.getSessionId(), groupieId ) )
                {
                    if( m_HostServerMgr.sendMemberListToClient( sktBase, netIdent ) )
                    {
                        if( txPacket( groupieId.getUserOnlineId(), sktBase, &joinReply ) )
                        {
                            broadcastToClients( &joinReply, groupieId.getUserOnlineId(), sktBase );    
                        }
                    }
                }         
            }
        }
        else if( sendPkt )
        {
            m_Engine.getConnectIdListMgr().addConnection( sktBase, groupieId, false );
            m_HostServerMgr.onJoinRequested( sktBase, netIdent, joinReq->getSessionId(), joinReq->getHostType() );
            txPacket( groupieId.getUserOnlineId(), sktBase, &joinReply );
        }
    }
    else
    {
        LogMsg( LOG_DEBUG, "PluginBaseNetworkService onPktHostJoinReq Invalid Packet" );
        joinReply.setCommError( eCommErrInvalidPkt );
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );   
    }
}

//============================================================================
void PluginBaseNetworkService::onPktHostLeaveReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService  %s got PktHostLeaveReq from %s", DescribeHostType( getHostType() ), netIdent->getOnlineName() );
    PktHostLeaveReq* pktReq = ( PktHostLeaveReq* )pktHdr;
    PktHostLeaveReply pktReply;
    if( !pktReq->isValidPktPrefix() )
    {
        LogMsg( LOG_DEBUG, "PluginBaseNetworkService onPktHostLeaveReq Invalid Packet" );
        pktReply.setCommError( eCommErrInvalidPkt );
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );
        return;
    }

    GroupieId rxedGroupieId = pktReq->getGroupieId();
    GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), getHostType() );
    if( rxedGroupieId != groupieId )
    {
        LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService %s from %s groupie %s does not match rxed %s", DescribePluginType( getPluginType() ),
            netIdent->getOnlineName(), groupieId.describeGroupieId().c_str(), rxedGroupieId.describeGroupieId().c_str() );
    }

    pktReply.setGroupieId( groupieId );
    pktReply.setPluginType( getPluginType() );
    pktReply.setSessionId( pktReq->getSessionId() );
    pktReply.setAccessState( m_HostServerMgr.getPluginAccessState( netIdent ) );
        
    VxGUID sktConnectionId( sktBase->getSocketId() );

    bool broadcastPkt = false;
    if( ePluginAccessOk == pktReply.getAccessState() )
    {
        broadcastPkt = true;
    }
    else if( ePluginAccessLocked == pktReply.getAccessState() )
    {
        if( !netIdent->isIgnored() )
        {
            if( m_HostServerMgr.getJoinState( netIdent, pktReq->getHostType() ) == eJoinStateJoinWasGranted )
            {
                // even though friendship not high enough if admin has accepted then send accepted
                pktReply.setAccessState( ePluginAccessOk );
                broadcastPkt = true;
            }
        }
        else
        {
            // TODO .. should we drop the connection?
        }
    }
    else if( ePluginAccessDisabled == pktReply.getAccessState() )
    {
        // join request sent to disabled plugin.. this should not happen
        LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got leave request to disabled plugin from %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
    }
    else if( ePluginAccessIgnored == pktReply.getAccessState() )
    {
        // TODO .. should we drop the connection of ignored person?
        LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got leave request from ignored person %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
    }

    if( broadcastPkt )
    {
        broadcastToClients( &pktReply, netIdent->getMyOnlineId(), sktBase );
    }
        
    m_HostServerMgr.onUserLeftHost( sktBase, netIdent, pktReply.getSessionId(), pktReply.getHostType() );
    m_Engine.getConnectIdListMgr().removeConnection( sktConnectionId, groupieId );
}

//============================================================================
void PluginBaseNetworkService::onPktHostUnJoinReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService %s got unjoin request from %s", DescribeHostType( getHostType() ), netIdent->getOnlineName() );
    PktHostUnJoinReq* joinReq = ( PktHostUnJoinReq* )pktHdr;
    PktHostUnJoinReply joinReply;
    if( joinReq->isValidPktPrefix() )
    {
        GroupieId rxedGroupieId = joinReq->getGroupieId();
        GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), getHostType() );
        if( rxedGroupieId != groupieId )
        {
            LogModule( eLogHostJoin, LOG_DEBUG, "PluginBaseNetworkService %s unjoin request from %s groupie %s does not match rxed %s", DescribePluginType( getPluginType() ),
                netIdent->getOnlineName(), groupieId.describeGroupieId().c_str(), rxedGroupieId.describeGroupieId().c_str() );
        }

        joinReply.setGroupieId( groupieId );
        joinReply.setPluginType( getPluginType() );
        joinReply.setSessionId( joinReq->getSessionId() );
        joinReply.setAccessState( m_HostServerMgr.getPluginAccessState( netIdent ) );

        VxGUID sktConnectionId( sktBase->getSocketId() );

        bool broadcastPkt = false;
        if( ePluginAccessOk == joinReply.getAccessState() )
        {
            broadcastPkt = true;
        }
        else if( ePluginAccessLocked == joinReply.getAccessState() )
        {
            if( !netIdent->isIgnored() )
            {
                if( m_HostServerMgr.getJoinState( netIdent, joinReq->getHostType() ) == eJoinStateJoinWasGranted )
                {
                    // even though friendship not high enough if admin has accepted then send accepted
                    joinReply.setAccessState( ePluginAccessOk );
                    broadcastPkt = true;
                }
            }
            else
            {
                // TODO .. should we drop the connection?
            }
        }
        else if( ePluginAccessDisabled == joinReply.getAccessState() )
        {
            // join request sent to disabled plugin.. this should not happen
            LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got unjoin request to disabled plugin from %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
        }
        else if( ePluginAccessIgnored == joinReply.getAccessState() )
        {
            // TODO .. should we drop the connection of ignored person?
            LogMsg( LOG_ERROR, "PluginBaseNetworkService %s got unjoin request from ignored person %s", DescribeHostType( getHostType() ), netIdent->getMyOnlineUrl().c_str() );
        }

        if( broadcastPkt )
        {
            broadcastToClients( &joinReply, netIdent->getMyOnlineId(), sktBase );
        }

        m_HostServerMgr.onUserUnJoined( sktBase, netIdent, joinReply.getSessionId(), groupieId );
        m_Engine.getConnectIdListMgr().removeConnection( sktConnectionId, groupieId );
    }
    else
    {
        LogMsg( LOG_DEBUG, "PluginBaseNetworkService onPktHostUnJoinReq Invalid Packet" );
        joinReply.setCommError( eCommErrInvalidPkt );
        onInvalidRxedPacket( sktBase, pktHdr, netIdent );
    }
}

//============================================================================
void PluginBaseNetworkService::onPktHostSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginBaseNetworkService onPktHostSearchReq" );
    PktHostSearchReply searchReply;
    PktHostSearchReq* searchReq = (PktHostSearchReq*)pktHdr;
    if( searchReq->isValidPktPrefix() )
    {
        EPluginAccess pluginAccess = m_HostServerMgr.getPluginAccessState( netIdent );
        searchReply.setAccessState( pluginAccess );
        if( ePluginAccessOk == pluginAccess )
        {
            PktBlobEntry& blobEntry = searchReq->getBlobEntry();
            blobEntry.resetRead();

            SearchParams searchParams;
            searchParams.extractFromBlob( blobEntry );
            searchReply.setHostType( searchParams.getHostType() );
            searchReply.setSearchSessionId( searchParams.getSearchSessionId() );
            
            std::string searchText = searchParams.getSearchText();
            if( !searchParams.getSearchListAll() && searchText.size() < MIN_SEARCH_TEXT_LEN )
            {
                LogModule( eLogHostSearch, LOG_DEBUG, "PluginBaseNetworkService search text too short" );
                searchReply.setCommError( eCommErrSearchTextToShort );
            }
            else
            {
                ECommErr searchErr = m_HostServerMgr.searchRequest( searchParams, searchReply, searchText, sktBase, netIdent );
                searchReply.setCommError( searchErr );
            }
        }
        else
        {
            LogModule( eLogHostSearch, LOG_DEBUG, "PluginBaseNetworkService host service not enabled" );
            searchReply.setCommError( eCommErrPluginNotEnabled );   
        }
    }
    else
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "PluginBaseNetworkService invalid search packet" );
        searchReply.setCommError( eCommErrInvalidPkt );
    }
    
    searchReply.setHostType( searchReq->getHostType() );
    searchReply.setSearchSessionId( searchReq->getSearchSessionId() );
    EPluginType overridePlugin = searchReq->getPluginType();

    if( !txPacket( netIdent->getMyOnlineId(), sktBase, &searchReply, overridePlugin ) )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "PluginBaseNetworkService failed send search reply" );
    }
}

//============================================================================
void PluginBaseNetworkService::onPktHostOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginChatRoomHost got join offer request" );
    PktHostOfferReply offerReply;
    offerReply.setAccessState( m_HostServerMgr.getPluginAccessState( netIdent ) );
    if( !txPacket( netIdent->getMyOnlineId(), sktBase, &offerReply) )
    {
        LogMsg( LOG_DEBUG, "PluginBaseNetworkService failed send onPktHostOfferReq" );
    }
}

//============================================================================
void PluginBaseNetworkService::onPktHostOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginBaseNetworkService got host offer reply" );
}

//============================================================================
void PluginBaseNetworkService::onPktHostInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_DEBUG, "PluginBaseNetworkService got host info request" );
    PktHostInfoReq* pktReq = ( PktHostInfoReq* )pktHdr;
    if( eHostTypeUnknown == pktReq->getHostType() || pktReq->getHostType() != getHostType() )
    {
        LogMsg( LOG_ERROR, "PluginBaseNetworkService::onPktHostInfoReq invalid host type" );
    }

    PktHostInfoReply pktReply;
    pktReply.setHostType( pktReq->getHostType() );
    pktReply.setSessionId( pktReq->getSessionId() );
    if( eHostTypeUnknown == pktReply.getHostType() || pktReply.getHostType() != getHostType() )
    {
        LogMsg( LOG_ERROR, "PluginBaseNetworkService::onPktHostInfoReq invalid host type" );
    }

    std::string hostTitle;
    std::string hostDesc;
    int64_t lastModifiedTime;
    if( m_PktHostInviteIsValid && isPluginEnabled() && m_Engine.getNetStatusAccum().getNetAvailStatus() != eNetAvailNoInternet && getHostTitleAndDescription( hostTitle,  hostDesc, lastModifiedTime ) )
    {
        pktReply.setHostTitleAndDescription( hostTitle, hostDesc, lastModifiedTime );
    }
    else
    {
        pktReply.setCommError( eCommErrPluginNotEnabled );
    }

    if( !txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply) )
    {
        LogMsg( LOG_DEBUG, "PluginBaseNetworkService failed send onPktHostInfoReq" );
    }
}

//============================================================================
void PluginBaseNetworkService::onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getHostedListMgr().onPktHostInfoReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
bool PluginBaseNetworkService::fromGuiRequestPluginThumb( VxNetIdent* netIdent, VxGUID& thumbId )
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
bool PluginBaseNetworkService::ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool isTemp )
{
    return m_ThumbXferMgr.requestPluginThumb( sktBase, netIdent, thumbId, isTemp );
}

//============================================================================
bool PluginBaseNetworkService::getHostTitleAndDescription( std::string& hostTitle, std::string& hostDesc, int64_t& lastModifiedTime )
{
    return m_PluginSetting.getHostTitleAndDescription( hostTitle, hostDesc, lastModifiedTime );
}

//============================================================================
void PluginBaseNetworkService::onPluginSettingsChanged( int64_t modifiedTimeMs )
{
    /*
    int64_t timeNow = GetGmtTimeMs();
    m_Engine.setPktAnnLastModTime( timeNow );

    m_Engine.lockAnnouncePktAccess();
    //m_Engine.getMyPktAnnounce().setHostOrThumbModifiedTime( getPluginType(), modifiedTimeMs );
    m_Engine.setPktAnnLastModTime( timeNow );
    // just time changes so other users will update when next connect.. no need to reannounce 
    m_Engine.getToGui().toGuiSaveMyIdent(m_Engine.getMyPktAnnounce().getVxNetIdent() );
    m_Engine.unlockAnnouncePktAccess();
    */
}

//============================================================================
void PluginBaseNetworkService::updateHostInvite( PluginSetting& pluginSetting )
{
    if( isPluginEnabled() )
    {
        pluginSetting.getHostTitleAndDescription( m_HostTitle, m_HostDescription, m_HostInfoModifiedTime );

        updateHostInviteUrl();
    }
}

//============================================================================
void PluginBaseNetworkService::updateHostInviteUrl( void )
{
    if( !PluginShouldAnnounceToNetwork( getPluginType() ) )
    {
        return;
    }

    if( eHostTypeUnknown == getHostType() )
    {
        LogMsg( LOG_VERBOSE, "PluginBaseNetworkService::updateHostInviteUrl unknown host type" );
        return;
    }

    if( m_HostInfoModifiedTime && m_Engine.getNetStatusAccum().isDirectConnectTested() && !m_Engine.getMyPktAnnounce().requiresRelay() )
    {
        m_Engine.lockAnnouncePktAccess();
        PktAnnounce& pktAnn = m_Engine.getMyPktAnnounce();
        std::string myOnlineUrl = pktAnn.getMyOnlineUrl();
        pktAnn.setHostOrThumbModifiedTime( getPluginType(), m_HostInfoModifiedTime );
        m_Engine.getToGui().toGuiSaveMyIdent( pktAnn.getVxNetIdent() );
        m_Engine.unlockAnnouncePktAccess();

        m_Engine.setPktAnnLastModTime( GetGmtTimeMs() );

        std::string inviteUrl = Invite::makeInviteUrl( getHostType(), myOnlineUrl );
        if( !inviteUrl.empty() )
        {
            VxGUID sessionId;
            sessionId.initializeWithNewVxGUID();

            m_AnnMutex.lock();
            m_HostInviteUrl = inviteUrl;
            m_PktHostInviteAnnounceReq.setHostType( getHostType() );
            m_PktHostInviteAnnounceReq.setSessionId( sessionId );
            VxGUID thumbId = pktAnn.getHostThumbId( getHostType(), true );
            bool result = m_PktHostInviteAnnounceReq.setHostInviteInfo( m_HostInviteUrl, m_HostTitle, m_HostDescription, m_HostInfoModifiedTime, thumbId );
            m_AnnMutex.unlock();

            if( result )
            {
                m_PktHostInviteIsValid = true;
            }
        }
    }
    else
    {
        m_PktHostInviteIsValid = false;
    }
}

//============================================================================
ECommErr PluginBaseNetworkService::getCommAccessState( VxNetIdent* netIdent )
{
    ECommErr commErr{ eCommErrNone };
    if( !isPluginEnabled() )
    {
        commErr = eCommErrPluginNotEnabled;
    }
    else
    {
        EPluginAccess pluginAccess = m_HostServerMgr.getPluginAccessState( netIdent );
        if( ePluginAccessOk != pluginAccess )
        {
            commErr = eCommErrPluginPermission;
        }
    }

    return commErr;
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieInfoReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieInfoReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieAnnReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieAnnReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieSearchReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieSearchReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieMoreReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseNetworkService::onPktGroupieMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_Engine.getGroupieListMgr().onPktGroupieMoreReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseNetworkService::onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostServerMgr.onUserJoinedHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostServerMgr.onUserLeftHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostServerMgr.onUserUnJoinedHost( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onGroupDirectUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    m_HostServerMgr.onGroupDirectUserAnnounce( groupieId, sktBase, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostServerMgr.onPktHostUserListReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostServerMgr.onPktHostUserListReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostServerMgr.onPktHostUserListMoreReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    m_HostServerMgr.onPktHostUserListMoreReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseNetworkService::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
    if( isOnline || connectId.getHostOnlineId() != connectId.getUserOnlineId() )
    {
        return;
    }

    if( m_HostServerMgr.isMember( connectId.getUserOnlineId(), true ) )
    {
        // announce user lost connection to host

        GroupieId groupieId( connectId.getUserOnlineId(), m_Engine.getMyOnlineId(), getHostType() );

        if( LogEnabled( eLogMembership ) )LogModule( eLogMembership, LOG_VERBOSE, "PluginBaseNetworkService::%s sending PktHostLeaveReply with groupie %s ", __func__,
            isOnline, m_Engine.describeGroupieId( groupieId ).c_str() );

        PktHostLeaveReply pktReply;
        
        pktReply.setGroupieId( groupieId );
        pktReply.setPluginType( getPluginType() );

        broadcastToClients( &pktReply, connectId.getUserOnlineId() );
    }
}
