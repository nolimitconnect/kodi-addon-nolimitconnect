#pragma once
//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseMultimedia.h"
#include "HostClientMgr.h"

#include <Connections/IConnectRequest.h>

#include <PktLib/PktsHostInvite.h>
#include <CoreLib/VxGUIDList.h>

class VxNetIdent;
class ConnectionMgr;

class PluginBaseHostClient : public PluginBaseMultimedia, public IConnectRequestCallback
{
public:

    PluginBaseHostClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginBaseHostClient() override = default;

    //=== hosting ===//
    virtual void				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;
    virtual void				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl ) override;
    virtual void				fromGuiLeaveHost( HostedId& adminId ) override;
    virtual void				fromGuiUnJoinHost( HostedId& adminId ) override;
    virtual void				fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable ) override;

    virtual bool				fromGuiRequestPluginThumb( VxNetIdent* netIdent, VxGUID& thumbId ) override;
    virtual bool                ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb = false ) override;

    virtual bool                sendLeaveHost( GroupieId& groupieId );
    virtual bool                sendUnJoinHost( GroupieId& groupieId );
    virtual bool                queryUserListFromHost( GroupieId& groupieId );

    virtual void                onUserJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) override;
    virtual void                onUserLeftHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) override;
    virtual void                onUserUnJoinedHost( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) override;
    virtual void                onGroupRelayedUserAnnounce( GroupieId& groupieId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent ) override;
 
    void				        onPktHostJoinReply              ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostLeaveReply             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostUnJoinReply            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostOfferReply             ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostInfoReply              ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostSearchReply            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktGroupieInfoReply           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktGroupieAnnReply            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktGroupieSearchReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktGroupieMoreReply           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

    void				        onPktHostUserListReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
    void				        onPktHostUserListMoreReply      ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

protected:
    //=== callback overrides ==//
    bool                        onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return true;  };
    void				        onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, ERunTestStatus testStatus,
                                                        EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    bool                        onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return false; };
    void				        onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, EConnectStatus connectStatus,
                                                      EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};
    void				        onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};
    void				        onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};
    void				        onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override {};
    bool                        onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };

    virtual void                buildHostAnnounce( PluginSetting& pluginSetting ) {};
    virtual void				sendHostAnnounce( void ) {};
    virtual void				sendLeaveHost( HostedId& adminId );
    virtual void				sendUnJoinHost( HostedId& adminId );


    void				        onContactWentOnline         ( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override {};
    void				        onContactWentOffline        ( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override {};
    void				        replaceConnection           ( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt ) override {};
    void				        onConnectionLost            ( std::shared_ptr<VxSktBase>& sktBase ) override {};

    void				        onContactOnlineStatusChange( ConnectId& connectId, bool isOnline ) override {};

    //=== vars ===//
    ConnectionMgr&              m_ConnectionMgr; 
    HostClientMgr               m_HostClientMgr;
    VxMutex                     m_ClientMutex;
    VxGUIDList                  m_JoinedHosts;

    bool                        m_SendAnnounceEnabled{ false };
    bool                        m_PktHostInviteIsValid{ false };
    PktHostInviteAnnounceReq    m_PktHostInviteAnnounceReq;
    VxMutex                     m_AnnMutex;
};
