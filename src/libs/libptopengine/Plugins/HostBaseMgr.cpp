//============================================================================
// Copyright (C) 2020 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostBaseMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginBase.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <UrlMgr/UrlMgr.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostJoin.h>
#include <PktLib/PktsSearch.h>
#include <PktLib/SearchParams.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
HostBaseMgr::HostBaseMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase )
: m_Engine( engine )
, m_PluginMgr( pluginMgr )
, m_MyIdent( myIdent )
, m_Plugin( pluginBase )
, m_ConnectionMgr(engine.getConnectionMgr())
{
    m_HostId = HostedId( engine.getMyOnlineId(), PluginTypeToHostType( getPluginType() ) );
}

//============================================================================
EPluginType HostBaseMgr::getPluginType( void )
{
    return m_Plugin.getPluginType();
}

//============================================================================
EHostType HostBaseMgr::getHostType( void )
{
    return PluginTypeToHostType( getPluginType() );
}

//============================================================================
EPluginAccess HostBaseMgr::getPluginAccessState( VxNetIdent* netIdent )
{
    EPluginAccess pluginAccess = ePluginAccessNotSet;

    EFriendState pluginState = m_MyIdent->getPluginPermission( m_Plugin.getPluginType() );
    if( eFriendStateIgnore == pluginState )
    {
        // we are not enabled
        pluginAccess = ePluginAccessDisabled;
    }
    else
    {
        if( netIdent->isIgnored() )
        {
            pluginAccess = ePluginAccessIgnored;
        }
        else
        {
            EFriendState friendState = netIdent->getMyFriendshipToHim();
            // everybody gets at least guest permission
            if( m_Engine.getConnectIdListMgr().isHosted( netIdent->getMyOnlineId() ) && friendState == eFriendStateAnonymous )
            {
                friendState = eFriendStateGuest;
            }

            if( friendState < pluginState )
            {
                // not enough permission
                pluginAccess = ePluginAccessLocked;
            }
            else
            {
                pluginAccess = ePluginAccessOk;
            }
        }
    }

    return pluginAccess;
}

//============================================================================
EConnectReason HostBaseMgr::getSearchConnectReason( EHostType hostType )
{
    EConnectReason connectReason = eConnectReasonUnknown;
    switch( hostType )
    {
    case eHostTypeChatRoom:
        connectReason = eConnectReasonChatRoomSearch;
        break;
    case eHostTypeGroup:
        connectReason = eConnectReasonGroupSearch;
        break;
    case eHostTypeRandomConnect:
        connectReason = eConnectReasonRandomConnectSearch;
        break;
    default:
        break;
    }

    return connectReason;
}

//============================================================================
void HostBaseMgr::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    if( ptopUrl.empty() || adminId.getHostType() == eHostTypeUnknown )
    {
        m_Engine.getToGui().toGuiHostAnnounceStatus( adminId.getHostType(), sessionId, eHostAnnounceInvalidUrl );
        return;
    }

    connectToHost( adminId.getHostType(), sessionId, ptopUrl, HostTypeToConnectAnnounceReason( adminId.getHostType() ) );
}

//============================================================================
void HostBaseMgr::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl )
{
    connectToHostByPtopUrlAndReason( adminId.getHostType(), sessionId, ptopUrl, HostTypeToConnectJoinReason( adminId.getHostType() ) );
}

//============================================================================
void HostBaseMgr::fromGuiLeaveHost( HostedId& adminId )
{
    m_Engine.disconnectFromHostIfNotNeeded( adminId );
}

//============================================================================
void HostBaseMgr::fromGuiUnJoinHost( HostedId& adminId )
{
    m_Engine.disconnectFromHostIfNotNeeded( adminId );
}

//============================================================================
void HostBaseMgr::fromGuiSearchHost( enum EHostType hostType, SearchParams& searchParams, bool enable )
{
    std::string url = searchParams.getSearchUrl();
    if( url.empty() )
    {
        url = m_ConnectionMgr.getDefaultHostUrl( hostType );
    }

    if( url.empty() || hostType == eHostTypeUnknown )
    {
        LogModule( eLogHostSearch, LOG_VERBOSE, "HostBaseMgr Invalid url or host type %d", hostType );
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, searchParams.getSearchSessionId(), eHostSearchInvalidUrl );
        m_Engine.getToGui().toGuiHostSearchComplete( hostType, searchParams.getSearchSessionId() );
        return;
    }

    if( !searchParams.getSearchSessionId().isValid() )
    {
        LogModule( eLogHostSearch, LOG_VERBOSE, "HostBaseMgr Search GUID invalid" );
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, searchParams.getSearchSessionId(), eHostSearchInvalidParam );
        m_Engine.getToGui().toGuiHostSearchComplete( hostType, searchParams.getSearchSessionId() );
        return;
    }

    searchParams.setHostType( hostType );
    if( m_Engine.isNetworkHostEnabled() )
    {
        // I am network host so search myself
        m_Engine.fromGuiSendAnnouncedList( hostType, m_Engine.getMyOnlineId() );
        m_Engine.getToGui().toGuiHostSearchStatus( hostType, m_Engine.getMyOnlineId(), eHostSearchCompleted );
        m_Engine.getToGui().toGuiHostSearchComplete( hostType, searchParams.getSearchSessionId() );
    }
    else
    {
        enum EConnectReason connectReason = eConnectReasonUnknown;
        switch( searchParams.getSearchType() )
        {
        case eSearchChatRoomHost:	        //!< Search for Chat Room to Join
            connectReason = eConnectReasonChatRoomSearch;
            break;
        case eSearchGroupHost:	            //!< Search for Group to Join
            connectReason = eConnectReasonGroupSearch;
            break;
        case eSearchRandomConnectHost:	    //!< Search for Random Connect Server to Join
            connectReason = eConnectReasonRandomConnectSearch;
            break;

        default:
            LogMsg( LOG_ERROR, "HostBaseMgr Unknown search type" );
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, searchParams.getSearchSessionId(), eHostSearchInvalidParam );
            m_Engine.getToGui().toGuiHostSearchComplete( hostType, searchParams.getSearchSessionId() );
            return;
        }

        if( !enable )
        {
            LogMsg( LOG_ERROR, "HostBaseMgr fromGuiSearchHost not enabled" );
        }
        else
        {
            if( addSearchSession( searchParams.getSearchSessionId(), searchParams ) )
            {
                connectToHost( hostType, searchParams.getSearchSessionId(), url, connectReason );
            }
        }
    }
}

//============================================================================
bool HostBaseMgr::isAnnounceConnectReason( enum EConnectReason connectReason )
{
    bool isAnnounceReason = false;
    switch( connectReason )
    {
    case eConnectReasonChatRoomAnnounce:
    case eConnectReasonGroupAnnounce:
    case eConnectReasonRandomConnectAnnounce:
        isAnnounceReason = true;
        break;
    default:
        break;
    }

    return isAnnounceReason;
}

//============================================================================
bool HostBaseMgr::isJoinConnectReason( enum EConnectReason connectReason )
{
    return IsConnectReasonJoin( connectReason );
}

//============================================================================
bool HostBaseMgr::isLeaveConnectReason( enum EConnectReason connectReason )
{
    return IsConnectReasonLeave( connectReason );
}

//============================================================================
bool HostBaseMgr::isUnJoinConnectReason( enum EConnectReason connectReason )
{
    return IsConnectReasonUnJoin( connectReason );
}

//============================================================================
bool HostBaseMgr::isSearchConnectReason( enum EConnectReason connectReason )
{
    return IsConnectReasonSearch( connectReason );
}

//============================================================================
void HostBaseMgr::onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostAnnounceStatus hostAnnStatus, std::string url )
{
    LogModule( eLogHostConnect, LOG_VERBOSE, "HostBaseMgr connect reason %s to host %s failed %s %s", DescribeConnectReason( connectReason ), DescribeHostType( hostType ),
        DescribeHostAnnounceStatus(hostAnnStatus), url.c_str());
    m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, hostAnnStatus );
    removeSession( sessionId, connectReason );
}

//============================================================================
void HostBaseMgr::onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostJoinStatus hostJoinStatus, std::string url )
{
    LogModule( eLogHostConnect, LOG_VERBOSE, "HostBaseMgr connect reason %s to host %s failed %s %s", DescribeConnectReason( connectReason ), DescribeHostType( hostType ),
           DescribeHostJoinStatus(hostJoinStatus), url.c_str());
    m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, hostJoinStatus );
    removeSession( sessionId, connectReason );
}

//============================================================================
void HostBaseMgr::onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostSearchStatus hostSearchStatus, std::string url )
{
    LogModule( eLogHostConnect, LOG_VERBOSE, "HostBaseMgr connect reason %s to host %s failed %s %s", DescribeConnectReason( connectReason ), DescribeHostType( hostType ),
        DescribeHostSearchStatus(hostSearchStatus), url.c_str());
    m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, hostSearchStatus );
    m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
    removeSession( sessionId, connectReason );
}

//============================================================================
bool HostBaseMgr::onConnectToHostSuccess( enum EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    bool result{ true };
    LogModule( eLogHostConnect, LOG_VERBOSE, "HostBaseMgr connect reason %s to host %s success ", DescribeConnectReason( connectReason ), DescribeHostType( hostType ) );
    GroupieId groupieId( m_Engine.getMyOnlineId(), onlineId,  hostType );
    if( groupieId.isValid() )
    {
        if( IsHostARelayForUsers( hostType ) && isJoinConnectReason( connectReason ) )
        {
            m_Engine.getHostedListMgr().connectToHostSuccess( groupieId.getHostedId() );
        }

        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceConnectSuccess );
            sendAnnounceRequest( groupieId, sessionId, sktBase, onlineId, connectReason );
        }
        else if( isJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnectSuccess );
            sendJoinRequest( groupieId, sessionId, sktBase, onlineId, connectReason );
        }
        else if( isLeaveConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnectSuccess );
            sendLeaveRequest( groupieId, sessionId, sktBase, onlineId, connectReason );
        }
        else if( isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnectSuccess );
            sendUnJoinRequest( groupieId, sessionId, sktBase, onlineId, connectReason );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchConnectSuccess );
            LogModule( eLogHostSearch, LOG_VERBOSE, "HostBaseMgr connect reason %s to host %s success Default function should be overridden", 
                    DescribeConnectReason( connectReason ), DescribeHostType( hostType ) );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
            result = false;
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "onConnectToHostSuccess invalid groupie id for connect reason %d", connectReason );
        result = false;
    }

    return result;
}

//============================================================================
void HostBaseMgr::onConnectionToHostDisconnect( enum EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    LogModule( eLogHostConnect, LOG_VERBOSE, "HostBaseMgr onConnectionToHostDisconnect reason %s to host %s ", DescribeConnectReason( connectReason ), DescribeHostType( hostType ) );
    removeSession( sessionId, connectReason );
}

//============================================================================
EHostType HostBaseMgr::connectReasonToHostType( enum EConnectReason connectReason )
{
    EHostType hostType = eHostTypeUnknown;
    switch( connectReason )
    {
    case eConnectReasonChatRoomAnnounce:
    case eConnectReasonChatRoomSearch:
    case eConnectReasonGroupAnnounce:
    case eConnectReasonGroupSearch:
    case eConnectReasonRandomConnectAnnounce:
    case eConnectReasonRandomConnectSearch:
        hostType = eHostTypeNetwork;
        break;

    case eConnectReasonGroupJoin:
    case eConnectReasonGroupLeave:
    case eConnectReasonGroupUnJoin:
    case eConnectReasonGroupUserConnect:
        hostType = eHostTypeGroup;
        break;

    case eConnectReasonChatRoomJoin:
    case eConnectReasonChatRoomLeave:
    case eConnectReasonChatRoomUnJoin:
    case eConnectReasonChatRoomUserConnect:
        hostType = eHostTypeChatRoom;
        break;

    case eConnectReasonRandomConnectJoin:
    case eConnectReasonRandomConnectLeave:
    case eConnectReasonRandomConnectUnJoin:
    case eConnectReasonRandomConnectUserConnect:
        hostType = eHostTypeRandomConnect;
        break;

    default:
        break;
    }

    return hostType;
}

//============================================================================
bool HostBaseMgr::onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason )
{
    bool result{ false };
    EHostType hostType = connectReasonToHostType( connectReason );
    LogModule( eLogHostConnect, LOG_DEBUG, "HostBaseMgr::%s %s url %s reason %s", __func__, 
        DescribeHostType( hostType ), url.c_str(), DescribeConnectReason( connectReason ) );
    if( eHostTypeUnknown != hostType )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceConnecting );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchConnecting );
        }
        else if( isJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnecting );
        }  
        else if( isLeaveConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnecting );
        }
        else if( isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnecting );
        }
        else
        {
            LogMsg( LOG_ERROR, "onUrlActionQueryIdSuccess Unknown connect reason %s", DescribeConnectReason( connectReason ) );
        }
    }

    std::shared_ptr<VxSktBase> sktBase( nullptr );
    EConnectStatus connectStatus = m_ConnectionMgr.requestConnection( sessionId, url, onlineId, this, sktBase, connectReason, hostType );
    if( eConnectStatusHandshaking == connectStatus )
    {
        if( sktBase )
        {
            result = onContactHandshaking( sessionId, sktBase, onlineId, connectReason );
        }
        else
        {
            LogModule( eLogHostConnect, LOG_DEBUG, "requestConnection success but has null socket" );
            if( isAnnounceConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceConnectFailed, url );
            }
            else if( isSearchConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchConnectFailed, url );
            }
            else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinConnectFailed, url );
            } 
            else
            {
                LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
            }
        }
    }
    else if( eConnectStatusReady == connectStatus )
    {
        if( sktBase )
        {
            result = onContactConnected( sessionId, sktBase, onlineId, connectReason );
        }
        else
        {
            LogModule( eLogHostConnect, LOG_DEBUG, "requestConnection success but has null socket" );
            if( isAnnounceConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceConnectFailed, url );
            }
            else if( isSearchConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchConnectFailed, url );
            }
            else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
            {
                onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinConnectFailed, url );
            } 
            else
            {
                LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
            }
        }
    }
    else
    {
        LogModule( eLogHostConnect, LOG_DEBUG, "requestConnection failed url %s last err %d", url.c_str(), VxGetLastError() );
        if( isAnnounceConnectReason( connectReason ) )
        {
            onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceConnectFailed, url );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchConnectFailed, url );
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinConnectFailed, url );
        } 
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }

    return result;
}

//============================================================================
void HostBaseMgr::onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, enum ERunTestStatus testStatus, enum EConnectReason connectReason, enum ECommErr commErr )
{
    EHostType hostType = connectReasonToHostType( connectReason );
    if( eHostTypeUnknown != hostType )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceQueryIdFailed, DescribeRunTestStatus( testStatus ) );
            m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchQueryIdFailed, commErr, DescribeRunTestStatus( testStatus ) );
            m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
        }
        else if( isJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinQueryIdFailed, DescribeRunTestStatus( testStatus ) );
            m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }
    else if( isAnnounceConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceQueryIdFailed, url );
    }
    else if( isSearchConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchQueryIdFailed, url );
    }
    else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinQueryIdFailed, url );
    }
    else
    {
        LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
    }
}

//============================================================================
bool HostBaseMgr::onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    EHostType hostType = connectReasonToHostType( connectReason );
    if( eHostTypeUnknown != hostType )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceHandshaking );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchHandshaking );
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinHandshaking );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }

    return true;
}

//============================================================================
void HostBaseMgr::onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    std::string url = sktBase ? sktBase->describeSktConnection() : "HostBaseMgr::onHandshakeTimeout";
    EHostType hostType = connectReasonToHostType( connectReason );
    if( eHostTypeUnknown != hostType )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceHandshakeTimeout );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchHandshakeTimeout );
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinHandshakeTimeout );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }
    else if( isAnnounceConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceHandshakeTimeout, url );
    }
    else if( isSearchConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchHandshakeTimeout, url );
    }
    else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
    {
        onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinHandshakeTimeout, url );
    }
    else
    {
        LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
    }
}

//============================================================================
void HostBaseMgr::onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    LogModule( eLogHostConnect, LOG_INFO, "onContactSessionDone  Reason %s", DescribeConnectReason( connectReason ) );
}

//============================================================================
bool HostBaseMgr::onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    bool result{ false };
    EHostType hostType = connectReasonToHostType( connectReason );
    if( eHostTypeUnknown != hostType )
    {  
        result = onConnectToHostSuccess( hostType, sessionId, sktBase, onlineId, connectReason);
    }
    else
    {
        LogModule( eLogHostConnect, LOG_DEBUG, "HostBaseMgr connect reason %s unknown host type %d - %s success ", DescribeConnectReason( connectReason ), hostType, DescribeHostType( hostType ) );
    }

    return result;
}

//============================================================================
void HostBaseMgr::onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    std::string url = sktBase ? sktBase->describeSktConnection() : "";
    url += m_Engine.describeUser( onlineId );
    EHostType hostType = connectReasonToHostType( connectReason );
    if( eHostTypeUnknown != hostType )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceFailConnectDropped );
            onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceConnectFailed, url );
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinFailConnectDropped );
            onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinFailConnectDropped, url );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchFailConnectDropped );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }
}

//============================================================================
void HostBaseMgr::onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, enum EConnectStatus connectStatus, enum EConnectReason connectReason, enum ECommErr commErr )
{
    EHostType hostType = connectReasonToHostType( connectReason );
    if( hostType != eHostTypeUnknown )
    {
        if( isAnnounceConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceConnectFailed, DescribeConnectStatus( connectStatus ) );
            m_ConnectionMgr.doneWithConnection( sessionId, onlineId, this, connectReason );
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinConnectFailed, DescribeConnectStatus( connectStatus ) );
            m_ConnectionMgr.doneWithConnection( sessionId, onlineId, this, connectReason );
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchConnectFailed, commErr, DescribeConnectStatus( connectStatus ) );
            m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
            m_ConnectionMgr.doneWithConnection( sessionId, onlineId, this, connectReason );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "HostBaseMgr::onConnectRequestFail unknown connect reason %s", DescribeConnectReason( connectReason ) );
    }
}

//============================================================================
void HostBaseMgr::sendAnnounceRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    vx_assert( nullptr != sktBase );
    LogModule( eLogHostConnect, LOG_DEBUG, "HostBaseMgr:: sendAnnounceRequest not done %s", DescribeConnectReason( connectReason ) );
    PktHostJoinReq pktJoin;
    pktJoin.setPluginType( m_Plugin.getPluginType() );
    pktJoin.setGroupieId( groupieId );
    pktJoin.setSessionId( sessionId );
    if( m_Plugin.txPacket( onlineId, sktBase, &pktJoin ) )
    {
        m_Engine.getToGui().toGuiHostAnnounceStatus( groupieId.getHostType(), sessionId, eHostAnnounceSendingJoinRequest );
    }
    else
    {
        m_Engine.getToGui().toGuiHostAnnounceStatus( groupieId.getHostType(), sessionId, eHostAnnounceSendJoinRequestFailed );
    }
}

//============================================================================
void HostBaseMgr::sendJoinRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    vx_assert( nullptr != sktBase );
    PktHostJoinReq pktJoin;

    m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendingJoinRequest );
    pktJoin.setPluginType( m_Plugin.getPluginType() );
    pktJoin.setGroupieId( groupieId );
    pktJoin.setSessionId( sessionId );
    if( !m_Plugin.txPacket( onlineId, sktBase, &pktJoin ) )
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendJoinRequestFailed );
    }
}

//============================================================================
void HostBaseMgr::sendLeaveRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    vx_assert( nullptr != sktBase );
    PktHostLeaveReq pktReq;

    m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendingLeaveRequest );
    pktReq.setPluginType( m_Plugin.getPluginType() );
    pktReq.setGroupieId( groupieId );
    pktReq.setSessionId( sessionId );
    if( !m_Plugin.txPacket( onlineId, sktBase, &pktReq ) )
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendLeaveRequestFailed );
    }
}

//============================================================================
void HostBaseMgr::sendUnJoinRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
    vx_assert( nullptr != sktBase );
    PktHostUnJoinReq pktJoin;

    m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendingUnJoinRequest );
    pktJoin.setPluginType( m_Plugin.getPluginType() );
    pktJoin.setGroupieId( groupieId );
    pktJoin.setSessionId( sessionId );
    if( !m_Plugin.txPacket( onlineId, sktBase, &pktJoin ) )
    {
        m_Engine.getToGui().toGuiHostJoinStatus( groupieId.getHostType(), sessionId, eHostJoinSendUnJoinRequestFailed );
        m_ConnectionMgr.doneWithConnection( sessionId, onlineId, this, connectReason );
    }
}

//============================================================================
bool HostBaseMgr::addContact( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    bool wasAdded = m_ContactList.addGuidIfDoesntExist( netIdent->getMyOnlineId() );
    if( wasAdded )
    {
        // TODO implement contact added
    }

    return wasAdded;
}

//============================================================================
bool HostBaseMgr::removeContact( VxGUID& onlineId )
{
    bool wasRemoved = m_ContactList.removeGuid( onlineId );
    if( wasRemoved )
    {
        // TODO implement contact removed
    }

    return wasRemoved;
}

//============================================================================
bool HostBaseMgr::addSearchSession( VxGUID& sessionId, SearchParams& searchParams )
{
    removeSearchSession( sessionId );
    m_SearchParamsMutex.lock();
    m_SearchParamsList[sessionId] = searchParams;
    m_SearchParamsMutex.unlock();
    return true;
}

//============================================================================
void HostBaseMgr::removeSearchSession( VxGUID& sessionId )
{
    m_SearchParamsMutex.lock();
    auto iter = m_SearchParamsList.find( sessionId );
    if( iter != m_SearchParamsList.end() )
    {
        m_SearchParamsList.erase( sessionId );
    }
    else
    {
        LogMsg( LOG_INFO, "HostBaseMgr::removeSearchSession session not found " );
    }

    m_SearchParamsMutex.unlock();
}

//============================================================================
void HostBaseMgr::onInvalidRxedPacket( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, const char* msg )
{
    // TODO proper invalid packet handling
    LogMsg( LOG_INFO, "HostBaseMgr::onInvalidRxedPacket " );
}

//============================================================================
void HostBaseMgr::onUserOnline( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& sessionId )
{
    if( sktBase && netIdent )
    {
        netIdent->upgradeToGuestFriendship();
        
        //GroupieId groupieId( netIdent->getMyOnlineId(), m_Engine.getMyOnlineId(), getHostType() );
        //HostUserSessionId hostUserSessionId( sktBase->getSocketId(), groupieId, sessionId );
        //BaseSessionInfo sessionInfo( hostUserSessionId );
        //m_Engine.getUserOnlineMgr().onUserOnline( sktBase, netIdent, sessionInfo );
    }
    else
    {
        LogMsg( LOG_ERROR, "HostBaseMgr::onUserOnline invalid param " );
    }
}

//============================================================================
void HostBaseMgr::onUserOffline( VxGUID& onlineId, VxGUID& sessionId )
{
    //m_UserListMutex.lock();
    //m_UserList.removeUser( onlineId, sessionId );
    //m_UserListMutex.unlock();
}

//============================================================================
EJoinState HostBaseMgr::getJoinState( VxNetIdent* netIdent, enum EHostType hostType )
{
    // BRJ TODO if has been accepted to host then should return eJoinStateJoinGranted
    return getPluginAccessState( netIdent ) == ePluginAccessOk ? eJoinStateJoinWasGranted : eJoinStateJoinRequested;
}

//============================================================================
EMembershipState HostBaseMgr::getMembershipState( VxNetIdent* netIdent, enum EHostType hostType )
{
    // BRJ TODO if has been accepted to host then should return eJoinStateJoinGranted
    return getPluginAccessState( netIdent ) == ePluginAccessOk ? eMembershipStateJoined : eMembershipStateJoinDenied;
}

//============================================================================
bool HostBaseMgr::connectToHostByPtopUrlAndReason( enum EHostType hostType, VxGUID& sessionId, std::string& ptopUrl, enum EConnectReason connectReason )
{
    VxPtopUrl hostUrl( ptopUrl );

    VxGUID onlineId;
    std::string ptopUrlAttemptConnect;
    if( hostUrl.isValid() )
    {
        onlineId = hostUrl.getOnlineId();
        ptopUrlAttemptConnect = ptopUrl;
    }

    if( !onlineId.isValid() || hostType == eHostTypeUnknown )
    {
        LogMsg( LOG_ERROR, "HostBaseMgr::connectToHostByPtopUrlAndReason invalid param" );
        m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinInvalidUrl );
        return false;
    }

    if( IsHostARelayForUsers( hostType ) )
    {
        // for the case of join last then there is no host listing because user did not get host list from network host
        m_Engine.getHostedListMgr().connectToHostAttempt( HostedId( onlineId, hostType ), ptopUrlAttemptConnect );
    }

    bool result{ false };

    if( hostUrl.isValid() )
    {
        std::shared_ptr<VxSktBase> sktBase( nullptr );
        if( m_Engine.getConnectMgr().isConnectedToHost( hostType, hostUrl, sktBase ) )
        {
            // handle already connected
            result = true;
            onConnectToHostSuccess( hostType, sessionId, sktBase, onlineId, connectReason );
        }
        else
        {
            // try lookup by groupie id
            GroupieId groupieId( m_Engine.getMyOnlineId(), onlineId, hostType );
            sktBase = m_Engine.getConnectIdListMgr().findHostConnection( groupieId );
            if( sktBase )
            {
                result = true;
                onConnectToHostSuccess( hostType, sessionId, sktBase, onlineId, connectReason );
            }
            else
            {
                // connect without having to query host id
                result = connectToHost( hostType, sessionId, ptopUrl, connectReason );
            }
        }
    }


    return result;
}

//============================================================================
bool HostBaseMgr::connectToHost( enum EHostType hostType, VxGUID& sessionId, std::string& urlIn, enum EConnectReason connectReason )
{
    bool result{ false };
    std::string url;
    VxPtopUrl ptopUrl( urlIn );
    if( ptopUrl.isValid() )
    {
        // no need to resolve. is already valid
        url = urlIn;
    }
    else
    {
        url = m_Engine.getUrlMgr().resolveUrl( urlIn );
    }

    if( !url.empty() )
    {
        VxGUID hostGuid = ptopUrl.getOnlineId();

        LogModule( eLogHostConnect, LOG_DEBUG, "HostBaseMgr::connectToHost %s url %s reason %s", DescribeHostType( hostType ), url.c_str(),
            DescribeConnectReason( connectReason ) );
        if( isAnnounceConnectReason( connectReason ) )
        {
            if( ptopUrl.isValid() )
            {
                onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
                result = true;
            }
            else
            {
                EHostAnnounceStatus annStatus = m_ConnectionMgr.lookupOrQueryAnnounceId( hostType, sessionId, url.c_str(), hostGuid, this, connectReason );
                if( eHostAnnounceQueryIdSuccess == annStatus )
                {
                    // no need for id query.. just request connection
                    onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
                    result = true;
                }
                else if( eHostAnnounceQueryIdInProgress == annStatus )
                {
                    // manager is attempting to query id
                    m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceQueryIdInProgress );
                    result = true;
                }
                else
                {
                    m_Engine.getToGui().toGuiHostAnnounceStatus( hostType, sessionId, eHostAnnounceInvalidUrl );
                    onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceInvalidUrl, urlIn );
                }
            }
        }
        else if( isJoinConnectReason( connectReason ) || isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            if( ptopUrl.isValid() )
            {
                result = onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
            }
            else
            {
                EHostJoinStatus joinStatus = m_ConnectionMgr.lookupOrQueryJoinId( sessionId, url.c_str(), hostGuid, this, connectReason );
                if( eHostJoinQueryIdSuccess == joinStatus )
                {
                    // no need for id query.. just request connection
                    result = onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
                }
                else if( eHostJoinQueryIdInProgress == joinStatus )
                {
                    // manager is attempting to query id
                    m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinQueryIdInProgress );
                    result = true;
                }
                else
                {
                    m_Engine.getToGui().toGuiHostJoinStatus( hostType, sessionId, eHostJoinInvalidUrl );
                    onConnectToHostFail( hostType, sessionId, connectReason, eHostJoinInvalidUrl, urlIn );
                }
            }
        }
        else if( isSearchConnectReason( connectReason ) )
        {
            if( ptopUrl.isValid() )
            {
                result = onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
            }
            else
            {
                EHostSearchStatus searchStatus = m_ConnectionMgr.lookupOrQuerySearchId( sessionId, url.c_str(), hostGuid, this, connectReason );
                if( eHostSearchQueryIdSuccess == searchStatus )
                {
                    // no need for id query.. just request connection
                    result = onUrlActionQueryIdSuccess( sessionId, url, hostGuid, connectReason );
                }
                else if( eHostSearchQueryIdInProgress == searchStatus )
                {
                    // manager is attempting to query id
                    m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchQueryIdInProgress );
                    result = true;
                }
                else
                {
                    m_Engine.getToGui().toGuiHostSearchStatus( hostType, sessionId, eHostSearchInvalidUrl );
                    m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );

                    onConnectToHostFail( hostType, sessionId, connectReason, eHostSearchInvalidUrl, urlIn );
                }
            }
        }
        else
        {
            LogMsg( LOG_WARN, "HostBaseMgr unknown connect reason %d-%s", connectReason, DescribeConnectReason( connectReason ) );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "HostBaseMgr host %s url is empty", DescribeHostType( hostType ) );
        if( isAnnounceConnectReason( connectReason ) || isSearchConnectReason( connectReason ) || isJoinConnectReason( connectReason ) ||
            isLeaveConnectReason( connectReason ) || isUnJoinConnectReason( connectReason ) )
        {
            onConnectToHostFail( hostType, sessionId, connectReason, eHostAnnounceInvalidUrl, urlIn );
        }
        else
        {
            LogMsg( LOG_ERROR, "Unknown Connect Reason %d", connectReason );
        }
    }

    return result;
}

//============================================================================
bool HostBaseMgr::isRelayedFromHost( GroupieId& groupieId, VxNetIdent* fromNetIdent )
{
    if( groupieId.getHostOnlineId() != fromNetIdent->getMyOnlineId() )
    {
        return true;
    }

    return groupieId.getUserOnlineId() != m_Engine.getMyOnlineId();
}