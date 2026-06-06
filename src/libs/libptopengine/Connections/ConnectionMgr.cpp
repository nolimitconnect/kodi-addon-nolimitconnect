//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectionMgr.h"
#include "ConnectedInfo.h"
#include "HandshakeInfo.h"

#include <BigListLib/BigListMgr.h>
#include <BigListLib/BigListInfo.h>
#include <Network/NetworkMgr.h>

#include <P2PEngine/P2PEngine.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <UrlMgr/UrlMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>
#include <CoreLib/VxSktUtil.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxUrl.h>

#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktConnect.h>
#include <NetLib/VxSktCrypto.h>

#include <PktLib/PktsRelay.h>

//============================================================================
ConnectionMgr::ConnectionMgr( P2PEngine& engine )
    : m_Engine( engine )
    , m_BigListMgr( engine.getBigListMgr() )
    , m_ConnectedList( engine.getConnectList() )
    , m_PeerMgr( engine.getPeerMgr() )
    , m_AllList( engine )   
{
}

//============================================================================
std::string ConnectionMgr::getDefaultHostUrl( EHostType hostType )
{
    std::string hostUrl("");
    lockConnectionList();
    auto iter = m_DefaultHostUrlList.find( hostType );
    if( iter != m_DefaultHostUrlList.end() )
    {
        hostUrl = iter->second;
    }

    unlockConnectionList();
    return hostUrl;
}

//============================================================================
void ConnectionMgr::setDefaultHostOnlineId( EHostType hostType, VxGUID& hostOnlineId )
{
    lockConnectionList();
    m_DefaultHostIdList[hostType] = hostOnlineId;
    unlockConnectionList();
}

//============================================================================
bool ConnectionMgr::getDefaultHostOnlineId( EHostType hostType, VxGUID& retHostOnlineId )
{
    bool result = false;
    retHostOnlineId.clearVxGUID();

    lockConnectionList();
    auto iter = m_DefaultHostIdList.find( hostType );
    if( iter != m_DefaultHostIdList.end() )
    {
        retHostOnlineId = iter->second;
        result = true;
    }

    unlockConnectionList();
    return result;
}

//============================================================================
EHostAnnounceStatus ConnectionMgr::lookupOrQueryAnnounceId( EHostType hostType, VxGUID& sessionId, std::string hostUrl, 
    VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason )
{
    EHostAnnounceStatus hostStatus = eHostAnnounceUnknown;
    if( urlCacheOnlineIdLookup( hostUrl, hostGuid ) )
    {
        hostStatus = eHostAnnounceQueryIdSuccess;
        if( hostType != eHostTypeUnknown )
        {
            std::string emptyUrl;
            m_Engine.getHostUrlListMgr().updateHostUrl( hostType, hostGuid, hostUrl );
        }
    }
    else if( getQueryIdFailedCount( hostType ) > 2 )
    {
        // dont keep hammering server if is sending an error
        hostStatus = eHostAnnounceQueryIdFailed;
    }
    else
    {
        hostStatus = eHostAnnounceQueryIdInProgress;
        std::string myUrl = m_Engine.getMyOnlineUrl();
        m_Engine.getRunUrlAction().runUrlAction( sessionId, eNetCmdQueryHostOnlineIdReq, hostUrl.c_str(), myUrl.c_str(), this, callback, hostType, connectReason );
    }

    return hostStatus;
}

//============================================================================
EHostJoinStatus ConnectionMgr::lookupOrQueryJoinId( VxGUID& sessionId, std::string hostUrl, VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason )
{
    EHostJoinStatus joinStatus = eHostJoinUnknown;
    if( urlCacheOnlineIdLookup( hostUrl, hostGuid ) )
    {
        joinStatus = eHostJoinQueryIdSuccess;
    }
    else
    {
        joinStatus = eHostJoinQueryIdInProgress;
        std::string myUrl = m_Engine.getMyOnlineUrl();
        m_Engine.getRunUrlAction().runUrlAction( sessionId, eNetCmdQueryHostOnlineIdReq, 
            m_Engine.getUrlMgr().resolveUrl( hostUrl ).c_str(), myUrl.c_str(), this, callback, eHostTypeUnknown, connectReason );
    }

    return joinStatus;
}

//============================================================================
EHostSearchStatus ConnectionMgr::lookupOrQuerySearchId( VxGUID& sessionId, std::string hostUrl, VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason )
{
    EHostSearchStatus joinStatus = eHostSearchUnknown;
    if( urlCacheOnlineIdLookup( hostUrl, hostGuid ) )
    {
        joinStatus = eHostSearchQueryIdSuccess;
    }
    else
    {
        joinStatus = eHostSearchQueryIdInProgress;
        std::string myUrl = m_Engine.getMyOnlineUrl();
        m_Engine.getRunUrlAction().runUrlAction( sessionId, eNetCmdQueryHostOnlineIdReq, hostUrl.c_str(), myUrl.c_str(), this, callback, eHostTypeUnknown, connectReason );
    }

    return joinStatus;
}


//============================================================================
void ConnectionMgr::onSktConnectedWithPktAnn( std::shared_ptr<VxSktBase>& sktBase, BigListInfo * bigListInfo )
{
    std::vector<HandshakeInfo> shakeList;
    std::vector<HandshakeInfo> timedOutList;
    m_HandshakeMutex.lock();
    m_HandshakeList.getAndRemoveHandshakeInfo( sktBase->getSocketId(), bigListInfo->getMyOnlineId(), shakeList, timedOutList );
    m_HandshakeMutex.unlock();

    if( !timedOutList.empty() )
    {
        for( HandshakeInfo& shakeInfo : timedOutList )
        {
            shakeInfo.onHandshakeTimeout();
        }
    }

    if( !shakeList.empty() )
    {
        lockConnectionList();
        ConnectedInfo* connectInfo = m_AllList.getOrAddConnectedInfo( sktBase->getSocketId(), bigListInfo );
        if( nullptr == connectInfo )
        {
            LogMsg( LOG_ERROR, "ConnectionMgr get connection info FAILED" );
        }
        else
        {
            for( HandshakeInfo& shakeInfo : shakeList )
            {
                connectInfo->addConnectReason( shakeInfo );
            }
        }

        // if we do the onContactConnected() while locked then may deadlock if doneWithConnection is called in onContactConnected
        unlockConnectionList();

        if( sktBase->isConnected() )
        {
            for( auto& shakeInfo : shakeList )
            {
                if( shakeInfo.getSocketId() == sktBase->getSocketId() && shakeInfo.getOnlineId() == bigListInfo->getMyOnlineId() )
                {
                    onConnectStatusChange( shakeInfo.getSessionId(), eConnectStatusConnectSuccess, shakeInfo.getConnectReason(), shakeInfo.getHostType() );
                }

                shakeInfo.onContactConnected();
            }
        }
        else
        {
            // lost connection already
            VxGUID socketId = sktBase->getSocketId();
            for( auto iter = shakeList.begin(); iter != shakeList.end(); )
            {
                if( iter->getSocketId() == socketId )
                {
                    shakeList.erase( iter );
                }
                else
                {
                    ++iter;
                }
            }
        }
    }
}

//============================================================================
void ConnectionMgr::onSktDisconnected( std::shared_ptr<VxSktBase>& sktBase )
{
    m_HandshakeMutex.lock();
    m_HandshakeList.onSktDisconnected( sktBase->getSocketId() );
    m_HandshakeMutex.unlock();

    lockConnectionList();
    m_AllList.onSktDisconnected( sktBase->getSocketId() );
    unlockConnectionList();
}

//============================================================================
void ConnectionMgr::callbackInternetStatusChanged( EInternetStatus internetStatus )
{
    bool internetBecameAvailable = m_InternetStatus == eInternetNoInternet &&
        internetStatus != eInternetNoInternet;
    lockConnectionList();
    m_InternetStatus = internetStatus;
    unlockConnectionList();
    if( internetBecameAvailable )
    {
        onInternetAvailable();
    }
}

//============================================================================
void ConnectionMgr::callbackNetAvailStatusChanged( ENetAvailStatus netAvalilStatus )
{
    bool networkBecameAvailable = ( m_NetAvailStatus == eNetAvailNoInternet ) && 
        ( netAvalilStatus != eNetAvailNoInternet );
    lockConnectionList();
    m_NetAvailStatus = netAvalilStatus;
    unlockConnectionList();
    if( networkBecameAvailable )
    {
        onNoLimitNetworkAvailable();
    }
}

//============================================================================
void ConnectionMgr::onInternetAvailable( void )
{

}

//============================================================================
void ConnectionMgr::onNoLimitNetworkAvailable( void )
{

}

//============================================================================
void ConnectionMgr::resetDefaultHostUrl( EHostType hostType )
{
    m_DefaultHostIdList[hostType] = VxGUID::nullVxGUID();
    m_DefaultHostUrlList[hostType] = "";
    m_DefaultHostRequiresOnlineId[hostType] = "";
    m_DefaultHostQueryIdFailed[hostType] = std::make_pair(eRunTestStatusUnknown, 0);
}

//============================================================================
void ConnectionMgr::applyDefaultHostUrl( EHostType hostType, std::string& hostUrlIn )
{
    std::string hostUrl = m_Engine.getUrlMgr().resolveUrl( hostUrlIn );

    lockConnectionList();
    m_DefaultHostUrlList[hostType] = hostUrl;
    unlockConnectionList();

    VxPtopUrl ptopUrl( hostUrlIn );
    if( ptopUrl.isValid() && ptopUrl.getOnlineId().isValid() )
    {
        // the onlie id was given as part of the url.. no need to query the online id
        VxUrl parsedUrl( hostUrl.c_str() );
        if( parsedUrl.validateUrl( false ) )
        {
            VxGUID onlineId = ptopUrl.getOnlineId();
            lockConnectionList();
            m_DefaultHostIdList[hostType] = onlineId;
            unlockConnectionList();
            updateUrlCache( hostUrl, onlineId );
            m_Engine.getUrlMgr().updateUrlCache( hostUrl, onlineId );
            if( hostType != eHostTypeUnknown )
            {
                std::string resolvedUrl = m_Engine.getUrlMgr().resolveUrl( hostUrl );
                m_Engine.getHostUrlListMgr().updateHostUrl( hostType, onlineId, resolvedUrl );
                return;
            }
        }
    }

    VxUrl parsedUrl( hostUrl.c_str() );
    if( parsedUrl.validateUrl( false ) )
    {
        bool needOnlineId = true;
        VxGUID onlineId = parsedUrl.getOnlineId();
        if( onlineId.isValid() )
        {
            needOnlineId = false;

            lockConnectionList();
            m_DefaultHostIdList[hostType] = onlineId;  
            unlockConnectionList();
            updateUrlCache( hostUrl, onlineId );   
            m_Engine.getUrlMgr().updateUrlCache( hostUrl, onlineId );
            if( hostType != eHostTypeUnknown )
            {
                std::string resolvedUrl = m_Engine.getUrlMgr().resolveUrl( hostUrl );
                m_Engine.getHostUrlListMgr().updateHostUrl( hostType, onlineId, resolvedUrl );
            }
        }

        if( needOnlineId )
        {
            lockConnectionList();
            m_DefaultHostRequiresOnlineId[hostType] = hostUrl;
            unlockConnectionList();

            // for query online id my url is probably not valid yet and not needed
            std::string myUrl; // = m_Engine.getMyOnlineUrl();
            static VxGUID sessionId;
            VxGUID::generateNewVxGUID( sessionId );
            m_Engine.getRunUrlAction().runUrlAction( sessionId, eNetCmdQueryHostOnlineIdReq, hostUrl.c_str(), myUrl.c_str(), this, nullptr, hostType );
        }
    }
}

//============================================================================
void ConnectionMgr::updateMyEnabledHostUrl( EHostType hostType, std::string& myUrl, VxGUID myOnlineId )
{
    lockConnectionList();
    m_DefaultHostUrlList[hostType] = myUrl;
    m_DefaultHostIdList[hostType] = myOnlineId;
    unlockConnectionList();

    updateUrlCache( myUrl, myOnlineId );
}

//============================================================================
void ConnectionMgr::callbackQueryIdSuccess( UrlActionInfo& actionInfo, VxGUID onlineId )
{
    std::string emptyUrl;
    EHostType hostType = actionInfo.getHostType();
    if( eHostTypeUnknown != hostType )
    {
        lockConnectionList();
        m_DefaultHostIdList[hostType] = onlineId;
        m_DefaultHostRequiresOnlineId[hostType] = "";
        unlockConnectionList();

        //if( eHostTypeNetwork == actionInfo.getHostType() && onlineId.isValid() )
        //{
        //    // exclude network host from updating online status to gui because is just temporary
        //    // also set the network host id so that we do not block packet announce from network host
        //    // connection test does not need PktAnnounce so should not need to be excluded
        //    m_Engine.getConnectIdListMgr().updateOnlineExclusion( onlineId, true, true );
        //}
    }

    std::string hostUrl = actionInfo.getRemoteUrl();
    updateUrlCache( hostUrl, onlineId );
    if( actionInfo.getHostType() != eHostTypeUnknown )
    {
        std::string resolvedUrl = m_Engine.getUrlMgr().resolveUrl( hostUrl );
        m_Engine.getHostUrlListMgr().updateHostUrl( actionInfo.getHostType(), onlineId, resolvedUrl );
    }

    if( actionInfo.getConnectReqInterface() )
    {
        actionInfo.getConnectReqInterface()->onUrlActionQueryIdSuccess( actionInfo.getSessionId(), hostUrl, onlineId, actionInfo.getConnectReason() );
    }

    LogMsg( LOG_VERBOSE, "ConnectionMgr: Success query host %s for online id is %s",  hostUrl.c_str(),
        onlineId.toOnlineIdString().c_str());
}

//============================================================================
void ConnectionMgr::callbackActionFailed( UrlActionInfo& actionInfo, ERunTestStatus eStatus, ENetCmdError netCmdError )
{
    if( eHostTypeUnknown != actionInfo.getHostType() )
    {
        int failedCount = getQueryIdFailedCount( actionInfo.getHostType() );
        lockConnectionList();
        m_DefaultHostQueryIdFailed[actionInfo.getHostType()] = std::make_pair(eStatus, failedCount);
        unlockConnectionList();
    }

    std::string hostUrl = actionInfo.getRemoteUrl();
    if( actionInfo.getConnectReqInterface() )
    {
        actionInfo.getConnectReqInterface()->onUrlActionQueryIdFail( actionInfo.getSessionId(), hostUrl, eStatus, actionInfo.getConnectReason() );
    }

    LogMsg( LOG_ERROR, "ConnectionMgr: connect reason %s query host %s for id failed %s %s", 
            DescribeConnectReason( actionInfo.getConnectReason() ),  hostUrl.c_str(),
            DescribeRunTestStatus( eStatus ), DescribeNetCmdError( netCmdError ));
}

//============================================================================
void ConnectionMgr::setQueryIdFailedCount( EHostType hostType, int failedCount )
{
    lockConnectionList();
    auto iter = m_DefaultHostQueryIdFailed.find( hostType );
    if( iter != m_DefaultHostQueryIdFailed.end() )
    {
        iter->second.second = failedCount;
    }

    unlockConnectionList();
}

//============================================================================
int ConnectionMgr::getQueryIdFailedCount( EHostType hostType )
{
    int failedCount = 0;
    lockConnectionList();
    auto iter = m_DefaultHostQueryIdFailed.find( hostType );
    if( iter != m_DefaultHostQueryIdFailed.end() )
    {
        failedCount = iter->second.second;
    }

    unlockConnectionList();
    return failedCount;
}

//============================================================================
EConnectStatus ConnectionMgr::requestConnection( VxGUID& sessionId, std::string url, VxGUID onlineId, IConnectRequestCallback* callback, 
                                                 std::shared_ptr<VxSktBase>& retSktBase, EConnectReason connectReason, enum EHostType hostType )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectionMgr::%s must have valid online id", __func__ );
        onConnectStatusChange( sessionId, eConnectStatusBadParam, connectReason, hostType );
        return eConnectStatusBadParam;
    }

    if( onlineId == m_Engine.getMyOnlineId() )
    {
        LogModule( eLogHostConnect, LOG_DEBUG, "ConnectionMgr::%s %s Loopback Socket", __func__, DescribeConnectReason( connectReason ) );
        retSktBase = m_Engine.getSktLoopback();
        int64_t timeMs = GetGmtTimeMs();
        retSktBase->setLastActiveTimeMs( timeMs );
        retSktBase->setLastSessionTimeMs( timeMs );
        retSktBase->setLastImAliveTimeRxMs( timeMs );

        if( callback )
        {
            callback->onContactConnected( sessionId, retSktBase, onlineId, connectReason );
        }

        onConnectStatusChange( sessionId, eConnectStatusBadParam, connectReason, hostType );
        return eConnectStatusReady;
    }

    LogModule( eLogHostConnect, LOG_VERBOSE, "ConnectionMgr::%s %s url %s", __func__, DescribeConnectReason( connectReason ), url.c_str() );
    std::shared_ptr<VxSktBase> sktBase( nullptr );

    bool isTempConnectReason = IsConnectReasonTemporary( connectReason );

    if( !isTempConnectReason )
    {
        // first see if we already have a connection to the requested onlineId
        bool isDisconnected = false;

        // see if we already have a connection for a different reason
        bool isOnline = m_Engine.getConnectIdListMgr().isUserOnline( onlineId );
        if( isOnline )
        {
            sktBase = m_Engine.getConnectIdListMgr().findAnyHostOnlineConnection( onlineId );
        }

        if( !sktBase )
        {
            lockConnectionList();
            ConnectedInfo* connectInfo = m_AllList.getAnyConnectedInfo( onlineId );
            if( connectInfo )
            {
                sktBase = connectInfo->getSktBase();
                if( sktBase )
                {
                    if( sktBase->isConnected() )
                    {
                        uint64_t timeNow = GetTimeStampMs();
                        HandshakeInfo shakeInfo( sktBase, sessionId, onlineId, callback, connectReason, timeNow, hostType );
                        connectInfo->addConnectReason( shakeInfo );
                        isDisconnected = false;
                    }
                    else
                    {
                        isDisconnected = true;
                    }
                }
            }

            unlockConnectionList();
        }

        if( sktBase )
        {
            if( eConnectReasonUnknown == sktBase->getConnectReason() )
            {
                // set the connect reason so is marked as temporary connection and not announced to gui
                sktBase->setConnectReason( connectReason );
            }
        }

        if( isDisconnected && sktBase )
        {
            onSktDisconnected( sktBase );
            sktBase = nullptr;
        }
        else if( sktBase && callback )
        {
            callback->onContactConnected( sessionId, sktBase, onlineId, connectReason );
        }
    }

    if( sktBase )
    {
        retSktBase = sktBase;
        if( isTempConnectReason )
        {
            sktBase->setIsTempConnection( true );
        }

        onConnectStatusChange( sessionId, eConnectStatusReady, connectReason, hostType );
        return eConnectStatusReady;
    }
    else
    {
        return attemptConnection( sessionId, url, onlineId, callback, retSktBase, connectReason, hostType );
    }

    onConnectStatusChange( sessionId, eConnectStatusUnknown, connectReason, hostType );
    return eConnectStatusUnknown;
}

//============================================================================
EConnectStatus ConnectionMgr::attemptConnection( VxGUID& sessionId, std::string url, VxGUID& onlineId, IConnectRequestCallback* callback, 
                                                 std::shared_ptr<VxSktBase>& retSktBase, enum EConnectReason connectReason, enum EHostType hostType )
{
    EConnectStatus connectStatus = eConnectStatusConnecting;
    onConnectStatusChange( sessionId, connectStatus, connectReason, hostType );
    VxUrl connectUrl( url.c_str() );
    if( onlineId.isValid() && connectUrl.validateUrl( false ) )
    {
        connectStatus = directConnectTo( url, onlineId, callback, retSktBase, sessionId, connectReason, hostType );
        if( connectStatus == eConnectStatusConnectSuccess )
        {
            // connected but waiting for PktAnnounce reply
            connectStatus = eConnectStatusReady;
            if( !IsConnectReasonTemporary( connectReason ) )
            {
                retSktBase->addConnectReason( connectReason );
            }
            else
            {
                retSktBase->setIsTempConnection( true );
            }
        }

        onConnectStatusChange( sessionId, connectStatus, connectReason, hostType );
    }
    else
    {
        connectStatus = eConnectStatusBadAddress;
        onConnectStatusChange( sessionId, connectStatus, connectReason, hostType );
    }

    return connectStatus;
}

//============================================================================
void ConnectionMgr::doneWithConnection( VxGUID socketId, VxGUID sessionId, VxGUID onlineId, IConnectRequestCallback* callback, EConnectReason connectReason )
{
    LogModule( eLogHostConnect, LOG_DEBUG, "ConnectionMgr::%s %s", __func__, DescribeConnectReason( connectReason ));
    m_HandshakeMutex.lock();
    m_HandshakeList.removeHandshakeInfo( socketId, sessionId );
    m_HandshakeMutex.unlock();

    bool sktDisconnected{ false };
    std::shared_ptr<VxSktBase> sktBase( nullptr );
    lockConnectionList();
    ConnectedInfo* connectInfo = m_AllList.getConnectedInfo( socketId, onlineId );
    if( connectInfo )
    {
        sktDisconnected = connectInfo->removeConnectReason( sessionId, callback, connectReason, sktBase );
    }

    unlockConnectionList();
    if( sktBase.get() == nullptr || !sktBase->isConnected() )
    {
        return; // nothing more we can do
    }

    if( sktDisconnected && !sktBase->removeConnectReason( connectReason ) )
    {
        // this has to be done after list is unlocked or mutex delock can occur
        // let the normal socket disconnected code do the work of removing the connection
        sktBase->closeSkt( eSktCloseConnectReasonsEmpty );
    }
}

//============================================================================
void ConnectionMgr::doneWithConnection( VxGUID sessionId, VxGUID onlineId, IConnectRequestCallback* callback, EConnectReason connectReason )
{
    LogModule( eLogHostConnect, LOG_DEBUG, "ConnectionMgr::%s %s", __func__, DescribeConnectReason( connectReason ) );
    m_HandshakeMutex.lock();
    m_HandshakeList.removeHandshakeSession( sessionId );
    m_HandshakeMutex.unlock();

    bool sktDisconnected{ false };
    std::vector<std::shared_ptr<VxSktBase>> sktList;
    lockConnectionList();
    m_AllList.removeConnectedReason( sessionId, onlineId, callback, connectReason, sktList );
    unlockConnectionList();

    for( auto sktBase : sktList )
    {
        // this has to be done after list is unlocked or mutex deadlock can occur
        // let the normal socket disconnected code do the work of removing the connection
        sktBase->closeSkt( ConnectReasonToCloseReason( connectReason ) );
    }
}

//============================================================================
void ConnectionMgr::updateUrlCache( std::string& hostUrl, VxGUID& onlineId )
{
    if( !hostUrl.empty() && onlineId.isValid() )
    {
        // there should be only one online id per ip and port however the ip may change
        // only keep the latest url
        m_Engine.getUrlMgr().updateUrlCache( hostUrl, onlineId );
        lockConnectionList();
        for( auto iter = m_UrlCache.begin(); iter != m_UrlCache.end(); ++iter )
        {
            if( iter->second == onlineId )
            {
                if( iter->first == hostUrl )
                {
                    // already in map
                    unlockConnectionList();
                    return;
                }

                m_UrlCache.erase( iter );
                break;
            }
        }

        m_UrlCache[hostUrl] = onlineId;
        unlockConnectionList();
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectionMgr::%s empty url", __func__ );
    }
}

//============================================================================
bool ConnectionMgr::urlCacheOnlineIdLookup( std::string& hostUrl, VxGUID& onlineId )
{
    onlineId.clearVxGUID();
    bool foundId = m_Engine.getUrlMgr().lookupOnlineId( hostUrl, onlineId );
    if( foundId )
    {
        LogMsg( LOG_VERBOSE, "ConnectionMgr::%s found online Id in cache url %s", __func__, hostUrl.c_str() );
        return foundId;
    }

    // it may be part of the url .. if so no lookup required
    VxUrl testUrl( hostUrl );
    if( testUrl.hasValidOnlineId() )
    {
        onlineId = testUrl.getOnlineId();
        foundId = true;
    }

    if( !foundId )
    {
        // not part of url.. see if is in cache
        lockConnectionList();
        auto iter = m_UrlCache.find( hostUrl );
        if( iter != m_UrlCache.end() )
        {
            onlineId = iter->second;
            foundId = true;
        }

        unlockConnectionList();
    }

    return foundId;
}

//============================================================================
EConnectStatus ConnectionMgr::directConnectTo(  std::string                 url,
                                                VxGUID&                     onlineId,
                                                IConnectRequestCallback*    callback,
                                                std::shared_ptr<VxSktBase>& retSktBase,
                                                VxGUID                      sessionId,
                                                EConnectReason              connectReason,
                                                enum EHostType              hostType,
                                                int					        iConnectTimeoutMs
                                                 )
{
    VxUrl connectUrl( url.c_str() );
    std::string ipAddr = connectUrl.getHostString();
    uint16_t port = connectUrl.getPort();
    if( !ipAddr.empty() && port )
    {
        return directConnectTo( ipAddr, port, onlineId, retSktBase, callback, sessionId, connectReason, hostType, iConnectTimeoutMs );
    }
    else
    {
        onConnectStatusChange( sessionId, eConnectStatusBadParam, connectReason, hostType );
        return eConnectStatusBadParam;
    }
}

//============================================================================-
EConnectStatus ConnectionMgr::directConnectTo(  VxConnectInfo&		        connectInfo,
                                                std::shared_ptr<VxSktBase>& ppoRetSkt,		// return pointer to socket if not null
                                                VxGUID                      sessionId, 
                                                int					        iConnectTimeoutMs,// how long to attempt connect
                                                bool				        bUseUdpIp,
                                                bool				        bUseLanIp,
                                                IConnectRequestCallback*    callback,
                                                EConnectReason              connectReason,
                                                enum EHostType              hostType )
{
    ppoRetSkt = nullptr;

    std::string ipAddr;
    EIpAddrType addrType{ eIpAddrTypeUnknown };
    connectInfo.m_DirectConnectId.getIpAddress( ipAddr, addrType );

    return directConnectTo( ipAddr, connectInfo.getOnlinePort(), connectInfo.getMyOnlineId(), ppoRetSkt, callback, sessionId, connectReason, hostType, iConnectTimeoutMs );
}

//============================================================================-
EConnectStatus ConnectionMgr::directConnectTo(  std::string                 ipAddr,
                                                uint16_t                    port,
                                                VxGUID                      onlineId,
                                                std::shared_ptr<VxSktBase>& retSktBase,
                                                IConnectRequestCallback*    callback,
                                                VxGUID                      sessionId, 
                                                EConnectReason              connectReason,
                                                enum EHostType              hostType,
                                                int					        iConnectTimeoutMs )
{
    EConnectStatus connectStatus = eConnectStatusConnecting;
    onConnectStatusChange( sessionId, eConnectStatusConnecting, connectReason, hostType );

    lockConnectionList();
    std::shared_ptr<VxSktBase> sktBase = m_PeerMgr.connectTo(	ipAddr.c_str(),			// remote ip or url 
                                                                port,	                // port to connect to
                                                                iConnectTimeoutMs );	// milli seconds before connect attempt times out
    if( sktBase )
    {
        if( sktBase->getConnectReason() == eConnectReasonUnknown )
        {
            sktBase->setConnectReason( connectReason );
        }

        // it is possible that the rx thread closes the socket immediately so keep checking the connection
        if( !sktBase->isConnected() )
        {
            LogModule( eLogConnect, LOG_ERROR, "ConnectionMgr::%s: connection to %s:%d was closed abruptly by rx thread", __func__, ipAddr.c_str(), port );
            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        // generate encryption keys
        LogModule( eLogSktData, LOG_VERBOSE, "ConnectionMgr::%s: connect success.. generating tx key %s:%d %s", __func__, sktBase->getRemoteIp().c_str(), port, onlineId.toHexString().c_str() );

        GenerateTxConnectionKey( sktBase, sktBase->getRemoteIp(), port, onlineId, m_Engine.getNetworkMgr().getNetworkKey() );

        if (!sktBase->isConnected())
        {
            LogModule(eLogConnect, LOG_ERROR, "ConnectionMgr::%s: connection to %s:%d was closed abruptly by rx thread after tx key generated", __func__, ipAddr.c_str(), port);
            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        LogModule( eLogSktData, LOG_VERBOSE, "ConnectionMgr::%s: connect success.. generating rx key", __func__ );

        GenerateRxConnectionKey( sktBase, &m_Engine.getMyPktAnnounce().m_DirectConnectId, m_Engine.getNetworkMgr().getNetworkKey().c_str() );

        if( !sktBase->isConnected() )
        {
            LogModule(eLogConnect, LOG_ERROR, "ConnectionMgr::%s connection to %s:%d was closed abruptly by rx thread after rx key generated", __func__, ipAddr.c_str(), port);
            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        LogModule( eLogSktData, LOG_VERBOSE, "ConnectionMgr::%s connect success.. sending announce", __func__ );

        if( false == sendMyPktAnnounce( onlineId, sktBase, true, false, false, IsConnectReasonTemporary( connectReason ) ) )
        {
            LogModule( eLogConnect, LOG_DEBUG, "ConnectionMgr::%s connect failed sending announce", __func__ );
            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        if( !sktBase->isConnected() )
        {
            LogModule( eLogConnect, LOG_ERROR, "ConnectionMgr::%s connection to %s:%d was closed after pkt announce sent by rx thread", __func__, ipAddr.c_str(), port);
            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        m_HandshakeMutex.lock();
        m_HandshakeList.addHandshake( sktBase, sessionId, onlineId, callback, connectReason, hostType );
        m_HandshakeMutex.unlock();
        if( !sktBase->isConnected() )
        {
            LogModule( eLogConnect, LOG_ERROR, "ConnectionMgr::%s connection to %s:%d was closed after handshake set by rx thread", __func__, ipAddr.c_str(), port);
            m_HandshakeMutex.lock();
            m_HandshakeList.removeHandshake( sktBase );
            m_HandshakeMutex.unlock();

            unlockConnectionList();
            onConnectStatusChange( sessionId, eConnectStatusSendPktAnnFailed, connectReason, hostType );
            return eConnectStatusSendPktAnnFailed;
        }

        connectStatus = eConnectStatusHandshaking;
        onConnectStatusChange( sessionId, eConnectStatusHandshaking, connectReason, hostType );
        retSktBase = (std::shared_ptr<VxSktBase>&)sktBase;        
    }
    else
    {
        connectStatus = eConnectStatusConnectFailed;
        onConnectStatusChange( sessionId, eConnectStatusConnectFailed, connectReason, hostType );

        //LogMsg( LOG_INFO, "ConnectionMgr::directConnectTo: connect FAIL to %s:%d", strIpAddress.c_str(), connectInfo.getOnlinePort() );
        LogModule( eLogConnect, LOG_DEBUG, "ConnectionMgr::DirectConnectTo: failed" );
    }

    LogModule( eLogConnect, LOG_DEBUG, "ConnectionMgr::DirectConnectTo: done" );
    unlockConnectionList();
    return connectStatus;
}

//============================================================================
bool ConnectionMgr::connectToContact(	VxConnectInfo&		            connectInfo, 
                                        std::shared_ptr<VxSktBase>&		ppoRetSkt,
                                        VxGUID&                         sessionId,
                                        bool&				            retIsNewConnection )
{
    bool gotConnected	= false;
    retIsNewConnection	 = false;
    if( connectInfo.getMyOnlineId() == m_Engine.getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, "ConnectionMgr::%s cannot connect to ourself", __func__ );  
        return false;
    }

    ppoRetSkt = m_Engine.getConnectIdListMgr().findAnyUserOnlineConnection( connectInfo.getMyOnlineId() );
    if( ppoRetSkt.get() )
    {
        LogMsg( LOG_VERBOSE, "ConnectionMgr::%s already connected", __func__ );  
        return true;
    }

#ifdef DEBUG_CONNECTIONS
    LogMsg( LOG_SKT, "connectToContact: id %s %s",  
        connectInfo.getOnlineName(),
        connectInfo.getMyOnlineId().describeVxGUID().c_str() );
#endif // DEBUG_CONNECTIONS

    lockConnectionList();
    ConnectedInfo * rmtUserConnectInfo = m_AllList.getAnyConnectedInfo( connectInfo.getMyOnlineId() );
    if( rmtUserConnectInfo )
    {
#ifdef DEBUG_CONNECTIONS
        std::string strId;
        connectInfo.getMyOnlineId(strId);
        LogMsg( LOG_SKT, "connectToContact: User is already connected %s id %s", 
            m_Engine.knownContactNameFromId( connectInfo.getMyOnlineId() ),
            strId.c_str() );
#endif // DEBUG_CONNECTIONS

        ppoRetSkt = rmtUserConnectInfo->getSktBase();
        unlockConnectionList();
        gotConnected = true;
    }
    else
    {
        unlockConnectionList();
        if( connectUsingTcp( connectInfo, ppoRetSkt, sessionId ) )
        {
            gotConnected		= true;
            retIsNewConnection	= true;
        }
    }

    return gotConnected;
}

//============================================================================
bool ConnectionMgr::connectUsingTcp( VxConnectInfo&	connectInfo, std::shared_ptr<VxSktBase>& ppoRetSkt, VxGUID& sessionId )
{
    ppoRetSkt = nullptr;
    std::shared_ptr<VxSktBase> sktBase( nullptr );
    if( false == connectInfo.m_DirectConnectId.isValid() )
    {
        LogMsg( LOG_ERROR, "ConnectionMgr::connectUsingTcp: User invalid online id" );
        return false;
    }

#ifdef DEBUG_CONNECTIONS
    LogMsg( LOG_INFO, "ConnectionMgr::connectUsingTcp: Attempting direct connect to %s ip %s port %d",
        m_Engine.knownContactNameFromId( connectInfo.getMyOnlineId() ),
        strDirectConnectIp.c_str(),
        connectInfo.m_DirectConnectId.getPort() );
#endif // DEBUG_CONNECTIONS
    if( eConnectStatusConnectSuccess == directConnectTo( connectInfo, sktBase, sessionId, DIRECT_CONNECT_TIMEOUT ) )
    {
        // direct connection success
#ifdef DEBUG_CONNECTIONS
        LogMsg( LOG_SKT, "ConnectionMgr::connectUsingTcp: SUCCESS skt %d direct connect to %s ip %s port %d",
            sktBase->m_SktNumber,
            m_Engine.knownContactNameFromId( connectInfo.getMyOnlineId() ),
            strDirectConnectIp.c_str(),
            connectInfo.m_DirectConnectId.getPort() );
#endif // DEBUG_CONNECTIONS
        ppoRetSkt = sktBase;

        return true;
    }
    else
    {
#ifdef DEBUG_CONNECTIONS
        LogMsg( LOG_SKT, "ConnectionMgr::directConnectTo: FAIL LAN connect to %s ip %s port %d",
            connectInfo.getOnlineName(),
            strDirectConnectIp.c_str(),
            connectInfo.m_DirectConnectId.getPort() );
#endif // DEBUG_CONNECTIONS

    }

    if( sktBase )
    {
        ppoRetSkt = sktBase;
        return true;
    }

	return false; // no ipv6 support
}

//============================================================================
//! encrypt and send my PktAnnounce to someone of whom we have no record
bool ConnectionMgr::sendMyPktAnnounce(  VxGUID&				destinationId,
                                        std::shared_ptr<VxSktBase>&			sktBase, 
                                        bool				requestAnnReply,
                                        bool				requestReverseConnection,
                                        bool				requestSTUN,
                                        bool                isTempConnection )
{
    PktAnnounce pktAnn;
    m_Engine.copyMyPktAnnounce(pktAnn);
    pktAnn.setAppAliveTimeSec( GetApplicationAliveSec() );

    pktAnn.setIsPktAnnReplyRequested( requestAnnReply );
    pktAnn.setIsPktAnnRevConnectRequested( requestReverseConnection );
    pktAnn.setIsPktAnnStunRequested( requestSTUN );
    pktAnn.setIsPktAnnTempConnection( isTempConnection );

    BigListInfo * poInfo = m_Engine.getBigListMgr().findBigListInfo( destinationId, true );	// id of friend to look for
    if( poInfo )
    {
        EFriendState eMyFriendshipToHim = poInfo->getMyFriendshipToHim();
        EFriendState eHisFriendshipToMe = poInfo->getHisFriendshipToMe();

        pktAnn.setMyFriendshipToHim( eMyFriendshipToHim );
        pktAnn.setHisFriendshipToMe( eHisFriendshipToMe );

        if( eMyFriendshipToHim != eFriendStateAnonymous || eHisFriendshipToMe != eFriendStateAnonymous )
        {
            if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_DEBUG, "ConnectionMgr::%s myFriendship %s hisFriendship %s", __func__,
                    DescribeFriendState( eMyFriendshipToHim ), DescribeFriendState( eHisFriendshipToMe ) );
        }
    }
    else
    {
        pktAnn.setMyFriendshipToHim( eFriendStateAnonymous );
        pktAnn.setHisFriendshipToMe( eFriendStateAnonymous );
    }

    if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "ConnectionMgr::%s through peer %s to %s", __func__,
        m_Engine.describeUser( sktBase->getPeerOnlineId() ).c_str(), m_Engine.describeUser( destinationId ).c_str() );

    return txPacket( destinationId, sktBase, &pktAnn );	
}

//============================================================================
bool ConnectionMgr::txPacket(	VxGUID&				        destinationId, 
                                std::shared_ptr<VxSktBase>&	sktBase, 
                                VxPktHdr*			        poPkt )
{
    bool bSendSuccess = false;
    poPkt->setSrcOnlineId( m_Engine.getMyOnlineId() );
    vx_assert( poPkt->getPktType() == PKT_TYPE_ANNOUNCE );

    if( 0 == (poPkt->getPktLength() & 0xf ) )
    {
        if( sktBase->isConnected() && sktBase->isTxEncryptionKeySet() )
        {
            sktBase->m_u8TxSeqNum++;
            poPkt->setPktSeqNum( sktBase->m_u8TxSeqNum );
            int32_t rc = sktBase->txPacket( destinationId, poPkt );
            if( 0 == rc )
            {
                bSendSuccess = true;
            }
            else
            {
                LogMsg( LOG_ERROR, "ConnectionMgr::txPacket: %s error %d", sktBase->describeSktType().c_str(), rc );
            }
        }
        else
        {
            if( false == sktBase->isConnected() )
            {
                LogMsg( LOG_ERROR, "ConnectionMgr::txPacket: error skt %d not connected", sktBase->m_SktNumber );
            }
            else
            {
                LogMsg( LOG_ERROR, "ConnectionMgr::txPacket: error skt %d has no encryption key", sktBase->m_SktNumber );
            }
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectionMgr::txPacket: Invalid system Packet length %d type %d", 
            poPkt->getPktLength(),
            poPkt->getPktType() );
    }

    return bSendSuccess;
}

//============================================================================
void ConnectionMgr::handleConnectSuccess( BigListInfo * bigListInfo, std::shared_ptr<VxSktBase>& skt, bool isNewConnection, EConnectReason connectReason )
{
    if( 0 != bigListInfo )
    {
        int64_t timeNow = GetGmtTimeMs();
        bigListInfo->setTimeLastConnectAttemptMs( timeNow );

        if( eConnectReasonRandomConnectJoin == connectReason )
        {
            //m_Engine.getToGui().toGuiScanResultSuccess( eScanTypeRandomConnect, bigListInfo );
        }
    }
}

//============================================================================
void ConnectionMgr::closeConnection( ESktCloseReason closeReason, VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
    BigListInfo* poInfo = m_Engine.getBigListMgr().findBigListInfo( onlineId );

    if( nullptr == poInfo )
    {
        LogMsg( LOG_ERROR, "Failed to find info for %s %s", onlineId.toHexString().c_str(), poInfo->getOnlineName() );
        sktBase->closeSkt( eSktCloseFindBigInfoFail );
        return;
    }

    ConnectedInfo* poRmtUserConnectInfo = m_AllList.getConnectedInfo( sktBase->getSocketId(), onlineId );
    if( poRmtUserConnectInfo )
    {
        /*
        if( poRmtUserConnectInfo->isRelayServer()
            || poRmtUserConnectInfo->isRelayClient() )
        {
            PktRelayUserDisconnect pktRelayDisconnect;
            pktRelayDisconnect.setSrcOnlineId( m_Engine.getMyPktAnnounce().getMyOnlineId() );
            pktRelayDisconnect.m_UserId = onlineId;
            skt->txPacket( onlineId, &pktRelayDisconnect );
        }
        else 
        {
            skt->closeSkt( 236 );
        }
        */

        sktBase->closeSkt( closeReason );
    }
    else
    {
        LogMsg( LOG_ERROR, "Failed to find ConnectedInfo for %s %s", onlineId.toHexString().c_str(), poInfo->getOnlineName() );
        sktBase->closeSkt( eSktCloseFindConnectedInfoFail );
    }
}

//============================================================================
bool ConnectionMgr::doConnectRequest( ConnectReqInfo& connectRequest, bool ignoreToSoonToConnectAgain )
{
    int64_t timeNow = GetGmtTimeMs();
    VxConnectInfo& connectInfo = connectRequest.getConnectInfo();
    if( false == m_Engine.getNetStatusAccum().isNetworkOnline() )
    {
         LogMsg( LOG_ERROR, "ConnectionMgr::doConnectRequest when not online" );
    }

    P2PConnectList& connectedList = m_Engine.getConnectList();
    connectedList.connectListLock();
    RcConnectInfo *	rcInfo = connectedList.findConnection( connectRequest.getMyOnlineId(), true );
    if( rcInfo )
    {
        // already connected
        BigListInfo * bigInfo = m_Engine.getBigListMgr().findBigListInfo( connectInfo.getMyOnlineId() );
        if( bigInfo )
        {
            connectedList.connectListUnlock();
            bigInfo->setTimeLastTcpContactMs( timeNow );
            bigInfo->setTimeLastConnectAttemptMs( timeNow );
            connectRequest.setTimeLastConnectAttemptMs( timeNow );
            handleConnectSuccess( bigInfo, rcInfo->getSkt(), false, connectRequest.getConnectReason() );
            return true;
        }
    }

    connectedList.connectListUnlock();

    if( ( false == ignoreToSoonToConnectAgain )
        && connectRequest.isTooSoonToAttemptConnectAgain() )
    {
#ifdef DEBUG_CONNECTIONS
        LogMsg( LOG_INFO, "ConnectionMgr::doConnectRequest: to soon to connect again %s", m_Engine.describeContact( connectRequest ).c_str() );
#endif // DEBUG_CONNECTIONS
        return false;
    }

    connectRequest.setTimeLastConnectAttemptMs( timeNow );
    BigListInfo * bigListInfo = m_Engine.getBigListMgr().findBigListInfo( connectInfo.getMyOnlineId() );
    if( bigListInfo )
    {
        bigListInfo->setTimeLastConnectAttemptMs( timeNow );
    }

    std::shared_ptr<VxSktBase> retSktBase;
    bool isNewConnection = false;
    if( m_Engine.connectToContact( connectInfo, retSktBase, isNewConnection, connectRequest.getConnectReason() ) )
    {
        // handle success connect
#ifdef DEBUG_CONNECTIONS
        LogMsg( LOG_INFO, "ConnectionMgr::doConnectRequest: success  %s", m_Engine.describeContact( connectInfo ).c_str() );
#endif // DEBUG_CONNECTIONS
        if( 0 == bigListInfo )
        {
            // when connected should have created a big list entry when got back a packet announce
            bigListInfo = m_Engine.getBigListMgr().findBigListInfo( connectInfo.getMyOnlineId() );
        }

        if( bigListInfo )
        {
            handleConnectSuccess( bigListInfo, retSktBase, isNewConnection, connectRequest.getConnectReason() );
        }
#ifdef DEBUG_CONNECTIONS
        else
        {
            LogMsg( LOG_INFO, "ConnectionMgr::doConnectRequest: No BigList for connected  %s", m_Engine.describeContact( connectInfo ).c_str() );
        }
#endif // DEBUG_CONNECTIONS

        return true;
    }

    // handle fail connect
#ifdef DEBUG_CONNECTIONS
    LogMsg( LOG_INFO, "ConnectionMgr::doConnectRequest: connect fail  %s", m_Engine.describeContact( connectInfo ).c_str() );
#endif // DEBUG_CONNECTIONS
    return false;
}

//============================================================================
void ConnectionMgr::onConnectStatusChange( VxGUID& sessionId, enum EConnectStatus connectStatus, enum EConnectReason connectReason, enum EHostType hostType )
{
    if( hostType != eHostTypeUnknown )
    {
        if( connectReason == eConnectReasonNetworkHostListSearch )
        {
            m_Engine.getHostedListMgr().hostSearchStatus( hostType, sessionId, connectStatus );
        }
        else if( connectReason == eConnectReasonGroupJoin ||
                 connectReason == eConnectReasonChatRoomJoin ||
                 connectReason == eConnectReasonRandomConnectJoin )
        {
            m_Engine.getUserJoinMgr().announceUserJoinAHostStatus( hostType, sessionId, connectStatus );
        }
    }

}
