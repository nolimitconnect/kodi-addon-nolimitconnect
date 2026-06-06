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

#include <Connections/IConnectRequest.h>

#include "HostedInfo.h"
#include "HostedListDb.h"

#include <CoreLib/VxMutex.h>

class P2PEngine;
class PluginBase;
class VxGUID;
class VxNetIdent;
class VxPktHdr;
class VxPtopUrl;
class HostedListCallbackInterface;

class HostedListMgr : public IConnectRequestCallback
{
    const int HOSTED_LIST_DB_VERSION = 1;
public:
    HostedListMgr() = delete;
    HostedListMgr( P2PEngine& engine );
    virtual ~HostedListMgr() = default;

    int32_t                       hostedListMgrStartup( std::string& dbFileName );
    int32_t                       hostedListMgrShutdown( void );

    void                        lockList( void )    { m_HostedInfoMutex.lock(); }
    void                        unlockList( void )  { m_HostedInfoMutex.unlock(); }

    void                        wantHostedListCallback( HostedListCallbackInterface* client, bool enable );

    bool                        fromGuiQueryMyHostedInfo( enum EHostType hostType, std::vector<HostedInfo>& hostedInfoList );
    bool                        fromGuiQueryHostedInfoList(enum  EHostType hostType, std::vector<HostedInfo>& hostedInfoList, VxGUID& hostIdIfNullThenAll );
    bool                        fromGuiQueryHostListFromNetworkHost( VxPtopUrl& netHostUrl, EHostType hostType, VxGUID& hostIdIfNullThenAll );
    virtual bool				fromGuiQueryGroupiesFromHosted( VxPtopUrl& hostedUrl, enum EHostType hostType, VxGUID& onlineIdIfNullThenAll );

    void                        connectToHostAttempt( HostedId adminId, std::string& ptopUrlAttempted );
    void                        connectToHostSuccess( HostedId adminId );
    void                        joinedToHostSuccess( HostedId adminId );

    // returns true if retHostedInfo was filled
    bool                        updateHostInfo( enum EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, bool infoChanged = true, HostedInfo* retHostedInfo = nullptr );
    void                        updateHosted( enum EHostType hostType, VxGUID& hostGuid, std::string& hosted, int64_t timestampMs = 0 );

    void                        updateHostedList( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
    void                        hostSearchResult( enum EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, HostedInfo& hostedInfo );
    void                        hostSearchStatus( enum EHostType hostType, VxGUID& searchSessionId, EConnectStatus connectStatus );
    void                        hostSearchCompleted( enum EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, ECommErr commErr );

    void                        onPktHostInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, PluginBase* plugin );

    void                        onHostInviteAnnounceAdded( enum EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
    void                        onHostInviteAnnounceUpdated( enum EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, bool infoChanged = true );

protected:
    virtual bool                onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, enum ERunTestStatus testStatus,
                                        enum EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

    virtual bool                onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};
    virtual void                onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    virtual bool                onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, enum EConnectStatus connectStatus,
                                        enum EConnectReason connectReason = eConnectReasonUnknown, enum ECommErr commErr = eCommErrNone ) override {};
    virtual void                onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    void						removeClosedPortIdent( VxGUID& onlineId );
    void						removeHostedInfo( EHostType hostType, VxGUID& onlineId );
    void						clearHostedInfoList( void );

    void                        updateAndRequestInfoIfNeeded( enum EHostType hostType, VxGUID& onlineId, std::string& nodeUrl, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    bool                        updateIsFavorite( enum EHostType hostType, VxGUID& onlineId, bool isFavorite );
    bool                        getIsFavorite( VxGUID& groupieOnlineId );

    bool                        updateLastConnected( enum EHostType hostType, VxGUID& onlineId, int64_t lastConnectedTime );
    bool                        updateLastJoined( enum EHostType hostType, VxGUID& onlineId, int64_t lastJoinedTime );
    bool                        updateHostTitleAndDescription( enum EHostType hostType, VxGUID& onlineId, std::string& title, std::string& description, int64_t lastDescUpdateTime, VxNetIdent* netIdent = nullptr );

    bool                        requestHostedInfo( enum EHostType hostType, VxGUID& onlineId, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    void                        announceHostInfoUpdated( HostedInfo* hostedInfo );
    void                        announceHostInfoSearchResult( HostedInfo* hostedInfo, VxGUID& sessionId );
    void                        announceHostInfoSearchStatus( EHostType hostType, VxGUID& sessionId, EConnectStatus connectStatus );
    void                        announceHostInfoSearchComplete( EHostType hostType, VxGUID& sessionId );
    void                        announceHostInfoRemoved( enum EHostType hostType, VxGUID& onlineId );

    void						addToListInJoinedTimestampOrder( std::vector<HostedInfo>& hostedInfoList, HostedInfo& hostedInfo );

    void						lockClientList( void )          { m_HostedInfoListClientMutex.lock(); }
    void						unlockClientList( void )        { m_HostedInfoListClientMutex.unlock(); }

    P2PEngine&                  m_Engine;
    VxMutex                     m_HostedInfoMutex;
    HostedListDb                m_HostedInfoListDb;

    std::vector<HostedInfo>     m_HostedInfoList;

    std::vector<HostedListCallbackInterface*> m_HostedInfoListClients;
    VxMutex						m_HostedInfoListClientMutex;

    EHostType                   m_SearchHostType{ eHostTypeUnknown };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_SearchSpecificOnlineId;
};

