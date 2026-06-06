#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

// uncomment to show ident list mgr lock/unlock
//#define DEBUG_CONNECT_ID_LIST_MGR_LOCK 1

#include <IdentListMgrs/IdentListMgrBase.h>
#include <CoreLib/ConnectId.h>

#include <vector>
#include <set>
#include <map>
#include <memory>

// maintains a list of online users
class ConnectIdListCallback;
class OnlineStatusCallback;
class P2PEngine;
class PktAnnounce;
class VxNetIdent;
class VxSktBase;

class ConnectIdListMgr : public IdentListMgrBase
{
public:
    ConnectIdListMgr() = delete;
    ConnectIdListMgr( P2PEngine& engine );
    virtual ~ConnectIdListMgr() = default;
    
    bool                        isDirectConnected( VxGUID& onlineId );
    bool                        isRelayed( VxGUID& onlineId );
    bool                        isHosted( VxGUID& onlineId );

    bool                        isOnline( GroupieId& groupieId );
    bool                        isUserOnline( VxGUID& onlineId );
    bool                        isUserExcluded( VxGUID onlineId );

    void                        setNetworkHostId( VxGUID& onlineId )    { m_NetworkHostOnlineId = onlineId; }
    VxGUID                      getNetworkHostId( void )                { return m_NetworkHostOnlineId; }
    bool                        isNetworkHost( VxGUID& onlineId )       { return onlineId == m_NetworkHostOnlineId; }

    bool                        isConnectionInUse( VxGUID& sktConnectId );

    void                        setExcludeConnectId( VxGUID& sktConnectId, bool exclude = true );
    bool                        isConnectIdExcluded( VxGUID& sktConnectId );

    void                        userJoinedHost( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId );
    void                        userLeftHost( VxGUID& sktConnectId, GroupieId& groupieId );

    void                        fromGuiDisconnectFromUser( VxGUID& onlineId );

    bool                        getConnections( HostedId& hostId, std::set<ConnectId>& retConnectIdSet, std::set<ConnectId>& relayConnectIdSet );

    std::shared_ptr<VxSktBase>  findDirectConnection( VxGUID& onlineId );
    std::shared_ptr<VxSktBase>  findHostConnection( GroupieId& groupieId, bool tryPeerFirst = false );
    std::shared_ptr<VxSktBase>  findAnyHostConnection( EHostType hostType );
    std::shared_ptr<VxSktBase>  findRelayMemberConnection( VxGUID& onlineId );
    std::shared_ptr<VxSktBase>  findPeerConnection( VxGUID& onlineId );

    std::shared_ptr<VxSktBase>  findAnyHostOnlineConnection( VxGUID onlineId );
    std::shared_ptr<VxSktBase>  findAnyUserOnlineConnection( VxGUID onlineId, EHostType hostType = eHostTypeUnknown );

    std::shared_ptr<VxSktBase>  findBestHostOnlineConnection( VxGUID& onlineId );
    std::shared_ptr<VxSktBase>  findBestUserOnlineConnection( VxGUID& onlineId, EPluginType pluginType = ePluginTypeInvalid );

    virtual bool                findConnectionId( GroupieId& groupieId, VxGUID& retSktConnectId );
    virtual bool                findRelayConnectionId( VxGUID& onlineId, VxGUID& retSktConnectId );
    std::shared_ptr<VxSktBase>  findSktBase( VxGUID connectId );

    bool                        addConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId );
    bool                        addConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId, bool isRelayed );
    void                        addHostConnection( std::shared_ptr<VxSktBase>& sktBase, GroupieId& groupieId ); // only called when join host on temp connection

    bool                        addConnection( ConnectId& connectId );

    bool                        connectionExists( ConnectId& connectId );

    void                        removeConnection( ConnectId& connectId );
    void                        removeConnection( VxGUID sktConnectId, GroupieId& groupieId );

    void                        disconnectIfIsOnlyUser( GroupieId& groupieId );
    void                        disconnectFromHost( HostedId& hostId );

    virtual bool                onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );
    virtual bool                onConnectionLost( VxGUID& sktConnectId, bool tmpConnection ); ///< returns false if invalid or is excluded connection
    virtual void                onGroupUserAnnounce( PktAnnounce* pktAnn, std::shared_ptr<VxSktBase>& sktBase, bool relayed );
    void                        onGroupRelayedUserAnnounce( PktAnnounce* pktAnn, std::shared_ptr<VxSktBase>& sktBasw );

    void                        getOnlineMembers( HostedId& hostId, std::vector<VxGUID>& onlineIdList );
    bool                        isMemberOnline( HostedId& hostId, VxGUID& onlineId );

    void                        pktAnnRecieved( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, VxNetIdent* peerNetIdent );
    void                        updateOnlineExclusion( VxGUID onlineId, bool excludeFromOnlineStatus, bool isNetworkHost = false );

    void                        wantConnectIdListCallback( ConnectIdListCallback* client, bool enable );

    bool                        addUnconfirmedConnection( ConnectId& connectId );
    void                        addUnconfirmedConnection( std::vector<std::pair<int64_t,ConnectId>>& unonfirmedIdList, ConnectId& connectId );

    bool                        updateUserJoinedFriendships( GroupieId& groupieId, VxNetIdent* netIdent );

protected:
    void                        announceConnectionStatus( ConnectId& connectId, bool isConnected );

    void                        announceConnectionLost( VxGUID& sktConnectId );

    void                        addOnlineConnectionPair( VxGUID& sktConnectId, VxGUID& onlineId );
    void                        removeOnlineConnectionPair( VxGUID& sktConnectId, VxGUID& onlineId );
    void                        removeOnlineConnectionPairs( VxGUID& sktConnectId, std::set<VxGUID>& lostConnUserList );

    void						lockConnectIdClientList( void )         { m_ConnectIdCallbackMutex.lock(); }
    void						unlockConnectIdClientList( void )       { m_ConnectIdCallbackMutex.unlock(); }

    void						lockOnlineStatusClientList( void )      { m_OnlineStatusCallbackMutex.lock(); }
    void						unlockOnlineStatusClientList( void )    { m_OnlineStatusCallbackMutex.unlock(); }

    void						lockOnlineIdList( void )                { m_OnlineIdMutex.lock(); }
    void						unlockOnlineIdList( void )              { m_OnlineIdMutex.unlock(); }

    void						lockExclusionList( void )               { m_ConnectIdExclusionMutex.lock(); }
    void						unlockExclusionList( void )             { m_ConnectIdExclusionMutex.unlock(); }

    void						lockConnectIdList( void )               { m_ConnectIdListMutex.lock(); }
    void						unlockConnectIdList( void )             { m_ConnectIdListMutex.unlock(); }

    bool                        checkUnconfirmedConnections( std::shared_ptr<VxSktBase>& sktBasw, VxGUID& onlineId );
    bool                        checkUnconfirmedList( std::shared_ptr<VxSktBase>& sktBasw, VxGUID& onlineId, std::vector<std::pair<int64_t,ConnectId>>& unconfirmedIdList );

    std::vector<std::pair<int64_t,ConnectId>> m_UnconfirmedConnectIdList;
    VxMutex						m_UnonfirmedConnectIdListMutex;

    std::set<ConnectId>         m_ConnectIdList;
    VxMutex						m_ConnectIdListMutex;

    std::vector<ConnectIdListCallback*>    m_ConnectIdCallbackClients;
    VxMutex						m_ConnectIdCallbackMutex;

    std::vector<OnlineStatusCallback*>    m_OnlineStatusCallbackClients;
    VxMutex						m_OnlineStatusCallbackMutex;

    std::set<VxGUID>            m_OnlineIdExclusionList;
    std::set<VxGUID>            m_OnlineIdListList;
    VxMutex						m_OnlineIdMutex;
    std::vector<std::pair<VxGUID, VxGUID>> m_OnlineConnectionPairs; // first connection id second online id
    VxGUID                      m_NetworkHostOnlineId;

    std::set<VxGUID>            m_ConnectIdExclusionList;
    VxMutex						m_ConnectIdExclusionMutex;
 
};

