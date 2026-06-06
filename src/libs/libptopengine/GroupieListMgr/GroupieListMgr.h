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

#include "GroupieInfo.h"
#include "GroupieListDb.h"

#include <CoreLib/VxMutex.h>

class P2PEngine;
class VxGUID;
class VxNetIdent;
class VxPktHdr;
class VxPtopUrl;
class GroupieListCallbackInterface;
class PluginBase;
class PluginBaseNetworkService;
class BaseSessionInfo;

class GroupieListMgr : public IConnectRequestCallback
{
    const int HOSTED_LIST_DB_VERSION = 1;
public:
    GroupieListMgr() = delete;
    GroupieListMgr( P2PEngine& engine );
    virtual ~GroupieListMgr() = default;

    int32_t                       groupieListMgrStartup( std::string& dbFileName );
    int32_t                       groupieListMgrShutdown( void );

    void                        lockList( void )    { m_GroupieInfoMutex.lock(); }
    void                        unlockList( void )  { m_GroupieInfoMutex.unlock(); }

    void                        addGroupieListMgrClient( GroupieListCallbackInterface* client, bool enable );

    bool                        setGroupieUrlAndTitleAndDescription( GroupieId& groupieId, std::string& nodeUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );
    bool                        getGroupieUrlAndTitleAndDescription( GroupieId& groupieId, std::string& nodeUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );

    bool                        fromGuiQueryMyGroupieInfo( enum EHostType hostType, std::vector<GroupieInfo>& groupieInfoList );
    bool                        fromGuiQueryGroupieInfoList( enum EHostType hostType, std::vector<GroupieInfo>& groupieInfoList, VxGUID& hostIdIfNullThenAll );

    // returns true if retGroupieInfo was filled
    bool                        updateGroupieInfo( enum EHostType hostType, GroupieInfo& groupieInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, GroupieInfo* retGroupieInfo = nullptr );
    void                        updateGroupie( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& hosted, int64_t timestampMs = 0 );

    void                        updateGroupieList( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
    void                        groupieSearchResult( enum EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, GroupieInfo& groupieInfo );
    void                        groupieSearchCompleted( enum EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, enum ECommErr commErr );

    virtual void				onPktGroupieInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, enum ECommErr commErr, PluginBase* plugin );
    virtual void				onPktGroupieInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, PluginBase* plugin );

    virtual void				onPktGroupieAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, enum ECommErr commErr, PluginBase* plugin );
    virtual void				onPktGroupieAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, PluginBase* plugin );
    virtual void				onPktGroupieSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, enum ECommErr commErr, PluginBase* plugin );
    virtual void				onPktGroupieSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, PluginBase* plugin );
    virtual void				onPktGroupieMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, enum ECommErr commErr, PluginBase* plugin );
    virtual void				onPktGroupieMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, PluginBase* plugin );

    void                        onGroupieAnnounceAdded( EHostType hostType, GroupieInfo& groupieInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
    void                        onGroupieAnnounceUpdated( EHostType hostType, GroupieInfo& groupieInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    void                        onHostJoinedByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    void                        onHostLeftByUser( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );

    virtual void                onUserOffline( VxGUID& onlineId ) {}; // TODO implement

protected:
    virtual bool                onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, enum ERunTestStatus testStatus,
                                        EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

    virtual bool                onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
    virtual void                onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};
    virtual void                onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    virtual bool                onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, EConnectStatus connectStatus,
                                        enum EConnectReason connectReason = eConnectReasonUnknown, enum ECommErr commErr = eCommErrNone ) override {};
    virtual void                onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    void						removeGroupieInfo( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType );
    void						clearGroupieInfoList( void );

    void                        updateAndRequestInfoIfNeeded( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType, std::string& nodeUrl, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    bool                        updateIsFavorite( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, bool isFavorite );
    bool                        getIsFavorite( VxGUID& groupieOnlineId );

    bool                        updateLastConnected( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, int64_t lastConnectedTime );
    bool                        updateLastJoined( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, int64_t lastJoinedTime );
    bool                        updateGroupieUrlAndTitleAndDescription( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& title, std::string& description, int64_t lastDescUpdateTime, VxNetIdent* netIdent = nullptr );

    bool                        requestGroupieInfo( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    void                        announceGroupieInfoUpdated( GroupieInfo* groupieInfo );
    void                        announceGroupieInfoSearchResult( GroupieInfo* groupieInfo, VxGUID& sessionId );
    void                        announceGroupieInfoRemoved( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType );

    void						addToListInJoinedTimestampOrder( std::vector<GroupieInfo>& groupieInfoList, GroupieInfo& groupieInfo );

    void						lockClientList( void )          { m_GroupieInfoListClientMutex.lock(); }
    void						unlockClientList( void )        { m_GroupieInfoListClientMutex.unlock(); }

    void                        logCommError( ECommErr commErr, const char* desc, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    void                        updateFromGroupieSearchBlob( enum EHostType hostType, VxGUID& hostOnlineId, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, PktBlobEntry& blobEntry, int hostInfoCount );
    bool                        requestMoreGroupiesFromHost( enum EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& nextGroupieOnlineId, PluginBase* plugin );

    void                        connectToGroupieIfPossible( GroupieInfo& groupieInfo, EConnectReason connectReason );

    P2PEngine&                  m_Engine;
    VxMutex                     m_GroupieInfoMutex;
    GroupieListDb               m_GroupieInfoListDb;

    std::vector<GroupieInfo>    m_GroupieInfoList;

    std::vector<GroupieListCallbackInterface*> m_GroupieInfoListClients;
    VxMutex						m_GroupieInfoListClientMutex;

    EHostType                   m_SearchHostType{ eHostTypeUnknown };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_SearchSpecificOnlineId;
};

