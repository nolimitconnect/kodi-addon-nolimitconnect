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

#include "HostClientSearchMgr.h"

#include <PktLib/PluginIdList.h>
#include <PktLib/SearchParams.h>
#include <CoreLib/GroupieId.h>

#include <set>

class BaseSessionInfo;
class PktAnnounce;
class PktHostSearchReply;

class HostClientMgr : public HostClientSearchMgr
{
public:
    HostClientMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase );
	virtual ~HostClientMgr() = default;

    virtual void                removeSession( VxGUID& sessionId, EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                sendHostSearchToNetworkHost( VxGUID& sessionId, SearchParams& searchParams, EConnectReason connectReason );

    virtual void                onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void                onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void                onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void                onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void				onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void				onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    virtual void				onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    virtual void                onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual void                onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    virtual void                onUserJoinHostGranted( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    virtual void                onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual void                onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );
    virtual void                onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual void                onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo );

    virtual void                onGroupRelayedUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );

protected:
    virtual bool                onConnectToHostSuccess( EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason ) override;
    virtual void                onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override;

    virtual void                startHostDetailSession( PktHostSearchReply* hostReply, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual bool                stopHostSearch( EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

    void                        addPluginRxSession( VxGUID& sessionId, PluginIdList& pluginIdList );
    void                        removePluginRxSession( VxGUID& sessionId );

    void                        clearUserInfoRequests( void );

    virtual void                sendNextUserInfoRequest( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& onlineId, VxGUID& sessionId );
    void                        announceUserInfo( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, VxGUID& sessionId, VxGUID& hostOnlineId, EHostType hostType );

    enum EConnectReason         hostTypeToConnectReason( EHostType hostType );

    void                        clearLastHostUserInfoRequestTime( void ) { m_SentUserInfoReqTime = 0; };

    VxMutex                     m_ClientMutex;
    VxGUIDList                  m_UserInfoReqList;
    int64_t                     m_SentUserInfoReqTime{ 0 };

    VxMutex                     m_ServerListMutex;
    std::set<GroupieId>         m_ServerList;
    std::map<VxGUID, PluginIdList> m_PluginRxList;
    VxMutex                     m_PluginRxListMutex;
};

