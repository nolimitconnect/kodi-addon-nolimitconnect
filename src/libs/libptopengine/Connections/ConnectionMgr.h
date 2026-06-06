//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include "ConnectedListAll.h"
#include "IConnectRequest.h"
#include "ConnectReqInfo.h"
#include "HandshakeList.h"

#include <Network/NetworkDefs.h>
#include <NetworkMonitor/NetStatusAccum.h>
#include <NetworkTest/RunUrlAction.h>

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxGUIDList.h>
#include <CoreLib/VxSemaphore.h>
#include <CoreLib/VxMutex.h>

class BigListMgr;
class HandshakeInfo;
class P2PEngine;
class P2PConnectList;
class VxPeerMgr;
class VxPktHdr;
class VxSktBase;

class ConnectionMgr : public NetAvailStatusCallbackInterface, public UrlActionResultInterface
{
public:
    ConnectionMgr() = delete;
    ConnectionMgr( P2PEngine& engine );
    virtual ~ConnectionMgr() = default;

    /// set default url from network settings
    void                        applyDefaultHostUrl( enum EHostType hostType, std::string& hostUrl );
    /// update default url for my hosted network service
    void                        updateMyEnabledHostUrl( EHostType hostType, std::string& myUrl, VxGUID myOnlineId );

    /// get default url for given host type
    std::string                 getDefaultHostUrl( enum EHostType hostType );

    void                        setDefaultHostOnlineId( enum EHostType hostType, VxGUID& hostOnlineId );
    bool                        getDefaultHostOnlineId( enum EHostType hostType, VxGUID& retHostOnlineId );

    EHostAnnounceStatus         lookupOrQueryAnnounceId( enum EHostType hostType, VxGUID& sessionId, std::string hostUrlIpv4, VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason = eConnectReasonUnknown );
    EHostJoinStatus             lookupOrQueryJoinId( VxGUID& sessionId, std::string hostUrlIpv4, VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason = eConnectReasonUnknown );
    EHostSearchStatus           lookupOrQuerySearchId( VxGUID& sessionId, std::string hostUrlIpv4, VxGUID& hostGuid, IConnectRequestCallback* callback, EConnectReason connectReason = eConnectReasonUnknown );

    EConnectStatus              requestConnection( VxGUID& sessionId, std::string url, VxGUID onlineId, IConnectRequestCallback* callback, std::shared_ptr<VxSktBase>& retSktBase, 
                                                   enum EConnectReason connectReason = eConnectReasonUnknown, enum EHostType hostType = eHostTypeUnknown );
    void                        doneWithConnection( VxGUID socketId, VxGUID sessionId, VxGUID onlineId, IConnectRequestCallback* callback, EConnectReason connectReason = eConnectReasonUnknown );
    // if failed to connect we will not have a socket id
    void                        doneWithConnection( VxGUID sessionId, VxGUID onlineId, IConnectRequestCallback* callback, EConnectReason connectReason );

    void                        onSktConnectedWithPktAnn( std::shared_ptr<VxSktBase>& sktBase, BigListInfo* bigListInfo );
    void                        onSktDisconnected( std::shared_ptr<VxSktBase>& sktBase );

    void                        lockConnectionList( void )      { m_ConnectionMutex.lock(); }
    void                        unlockConnectionList( void )    { m_ConnectionMutex.unlock(); }

    bool                        sendMyPktAnnounce(  VxGUID&				            destinationId,
                                                    std::shared_ptr<VxSktBase>&		sktBase,
                                                    bool				            requestAnnReply,
                                                    bool				            requestReverseConnection,
                                                    bool				            requestSTUN,
                                                    bool                            isTempConnection );

    bool                        connectToContact(   VxConnectInfo&		            connectInfo,
                                                    std::shared_ptr<VxSktBase>&		ppoRetSkt,
                                                    VxGUID&                         sessionId,
                                                    bool&				            retIsNewConnection );

    bool                        connectUsingTcp( VxConnectInfo& connectInfo, std::shared_ptr<VxSktBase>& ppoRetSkt, VxGUID& sessionId );

    void                        handleConnectSuccess( BigListInfo * bigListInfo, std::shared_ptr<VxSktBase>& skt, bool isNewConnection, EConnectReason connectReason );

    void                        closeConnection( enum ESktCloseReason closeReason, VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );

protected:
    virtual void				callbackInternetStatusChanged( EInternetStatus internetStatus ) override;
    virtual void				callbackNetAvailStatusChanged( ENetAvailStatus netAvalilStatus ) override;

    virtual void                callbackActionStatus( UrlActionInfo& actionInfo, ERunTestStatus eStatus, ENetCmdError netCmdError, std::string statusMsg ) override {};
    virtual void                callbackActionFailed( UrlActionInfo& actionInfo, ERunTestStatus eStatus, ENetCmdError netCmdError ) override;

    virtual void                callbackPingSuccess( UrlActionInfo& actionInfo, std::string myIp ) override {};
    virtual void                callbackConnectionTestSuccess( UrlActionInfo& actionInfo, bool canDirectConnect, std::string myIp ) override {};
    virtual void                callbackQueryIdSuccess( UrlActionInfo& actionInfo, VxGUID onlineId ) override;

    ConnectedInfo*              getOrAddConnectedInfo( const VxGUID& socketId, BigListInfo* bigListInfo ) { return m_AllList.getOrAddConnectedInfo( socketId, bigListInfo ); }

    void                        onInternetAvailable( void );
    void                        onNoLimitNetworkAvailable( void );
    void                        resetDefaultHostUrl( enum EHostType hostType );

    /// keep a cache of urls to online id to avoid time consuming query host id
    void                        updateUrlCache( std::string& hostUrl, VxGUID& onlineId );
    bool                        urlCacheOnlineIdLookup( std::string& hostUrl, VxGUID& onlineId );
    EConnectStatus              attemptConnection( VxGUID& sessionId, std::string url, VxGUID& onlineId, IConnectRequestCallback* callback, 
                                                   std::shared_ptr<VxSktBase>& retSktBase, enum EConnectReason connectReason, enum EHostType hostType = eHostTypeUnknown );

    //=== connection low level ===/
    EConnectStatus              directConnectTo( std::string                 url,
                                                 VxGUID&                     onlineId,
                                                 IConnectRequestCallback*    callback,
                                                 std::shared_ptr<VxSktBase>& retSktBase,
                                                 VxGUID                      sessionId,
                                                 enum EConnectReason         connectReason,
                                                 enum EHostType              hostType = eHostTypeUnknown,
                                                 int					     iConnectTimeoutMs = DIRECT_CONNECT_TIMEOUT );  

    EConnectStatus				directConnectTo(	VxConnectInfo&			    connectInfo,		 
                                                    std::shared_ptr<VxSktBase>& ppoRetSkt,		
                                                    VxGUID                      sessionId,
                                                    int						    iConnectTimeout = DIRECT_CONNECT_TIMEOUT,	 
                                                    bool					    useLanIp = false,
                                                    bool					    useUdpIp = false,
                                                    IConnectRequestCallback*    callback = nullptr,                                                
                                                    enum EConnectReason         connectReason = eConnectReasonUnknown,
                                                    enum EHostType              hostType = eHostTypeUnknown );

    EConnectStatus              directConnectTo( std::string                 ipAddr,
                                                 uint16_t                    port,
                                                 VxGUID                      onlineId,
                                                 std::shared_ptr<VxSktBase>& retSktBase,
                                                 IConnectRequestCallback*    callback,
                                                 VxGUID                      sessionId,
                                                 enum EConnectReason         connectReason,
                                                 enum EHostType              hostType,
                                                 int					     iConnectTimeoutMs );

    bool                        txPacket( VxGUID&				        destinationId,
                                          std::shared_ptr<VxSktBase>&	sktBase,
                                          VxPktHdr*			            poPkt );

    bool                        doConnectRequest( ConnectReqInfo& connectRequest, bool ignoreToSoonToConnectAgain );

    void                        setQueryIdFailedCount( enum EHostType hostType, int failedCount );
    int                         getQueryIdFailedCount( enum EHostType hostType );

    void                        onConnectStatusChange( VxGUID& sessionId, enum EConnectStatus connectStatus, enum EConnectReason connectReason, enum EHostType hostType );

    //=== vars ===//
    P2PEngine&					m_Engine;
    BigListMgr&					m_BigListMgr;
    P2PConnectList&             m_ConnectedList;
    VxPeerMgr&					m_PeerMgr;

    VxMutex						m_ConnectionMutex;
    ConnectedListAll            m_AllList;

    EInternetStatus             m_InternetStatus{ eInternetNoInternet };
    ENetAvailStatus             m_NetAvailStatus{ eNetAvailNoInternet };
    std::map<EHostType, VxGUID>         m_DefaultHostIdList;
    std::map<EHostType, std::string>    m_DefaultHostUrlList;
    std::map<EHostType, std::string>    m_DefaultHostRequiresOnlineId;
    std::map<EHostType, std::pair<ERunTestStatus, int>> m_DefaultHostQueryIdFailed;

    /// keep a cache of urls to online id to avoid time consuming query host id
    std::map<std::string, VxGUID>       m_UrlCache;

    HandshakeList               m_HandshakeList;
    VxMutex                     m_HandshakeMutex;
};

