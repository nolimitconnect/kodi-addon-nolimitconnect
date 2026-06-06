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
#include <User/UserList.h>

#include <PktLib/SearchParams.h>
#include <CoreLib/VxGUIDList.h>
#include <CoreLib/VxMutex.h>

#include <map>
#include <memory>

class ConnectionMgr;
class GroupieId;
class P2PEngine;
class PluginMgr;
class VxNetIdent;
class PluginBase;
class VxPktHdr;
class SearchParams;

class HostBaseMgr : public IConnectRequestCallback
{
public:
    HostBaseMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase );
	virtual ~HostBaseMgr() = default;

    EPluginType                 getPluginType( void );
    EHostType                   getHostType( void );
    void                        setHostId( HostedId& hostedId )       { m_HostId = hostedId; }
    virtual HostedId&           getHostId( void )                     { return m_HostId; }

    bool                        isRelayedFromHost( GroupieId& groupieId, VxNetIdent* fromNetIdent );

    //=== hosting ===//
    virtual void				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl );
    virtual void				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& ptopUrl );
    virtual void				fromGuiLeaveHost( HostedId& adminId );
    virtual void				fromGuiUnJoinHost( HostedId& adminId );
    virtual void				fromGuiSearchHost( enum EHostType hostType, SearchParams& searchParams, bool enable );

    virtual EPluginAccess	    getPluginAccessState( VxNetIdent* netIdent );
    virtual EJoinState	        getJoinState( VxNetIdent* netIdent, enum EHostType hostType );
    virtual EMembershipState	getMembershipState( VxNetIdent* netIdent, enum EHostType hostType );
    virtual EConnectReason      getSearchConnectReason( enum EHostType hostType );

    virtual bool                connectToHostByPtopUrlAndReason( enum EHostType hostType, VxGUID& sessionId, std::string& ptopUrl, enum EConnectReason connectReason );
    virtual bool                connectToHost( enum EHostType hostType, VxGUID& sessionId, std::string& url, EConnectReason connectReason );
    virtual void                removeSession( VxGUID& sessionId, enum EConnectReason connectReason = eConnectReasonUnknown ) {};

    // error handling for invalid packet
    virtual void				onInvalidRxedPacket( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent, const char* msg = "" );

protected:
    virtual bool                onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, enum ERunTestStatus testStatus, 
                                                        enum EConnectReason connectReason = eConnectReasonUnknown, enum ECommErr commErr = eCommErrNone ) override;

    virtual bool                onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
    virtual bool                onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;
    virtual void                onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, enum EConnectStatus connectStatus, 
                                                      enum EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override;

    virtual void                onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostAnnounceStatus hostJoinStatus, std::string url );
    virtual void                onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostJoinStatus hostJoinStatus, std::string url );
    virtual void                onConnectToHostFail( enum EHostType hostType, VxGUID& sessionId, enum EConnectReason connectReason, enum EHostSearchStatus hostSearchStatus, std::string url );
    virtual bool                onConnectToHostSuccess( enum EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );
    virtual void                onConnectionToHostDisconnect( enum EHostType hostType, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );

    void                        sendAnnounceRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );
    void                        sendJoinRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );
    void                        sendLeaveRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );
    void                        sendUnJoinRequest( GroupieId& groupieId, VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason );

    virtual void                onPktHostJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void                onPktHostLeaveReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void                onPktHostUnJoinReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void                onPktPluginSettingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void                onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

    virtual void				onPktHostUserInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserStatusReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserStatusReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

    virtual void				onPktHostUserListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserListMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void				onPktHostUserListMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

        
	virtual void				onPktTestConnTestReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
	virtual void				onPktTestConnTestReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr , VxNetIdent* netIdent ) {};
	virtual void				onPktTestConnPingReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
	virtual void				onPktTestConnPingReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

	virtual void				onPktQueryHostUrlReq		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
	virtual void				onPktQueryHostUrlReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

    virtual bool                addContact( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual bool                removeContact( VxGUID& onlineId );
    EHostType                   connectReasonToHostType( EConnectReason connectReason );

    virtual bool                isAnnounceConnectReason( enum EConnectReason connectReason );
    virtual bool                isJoinConnectReason( enum EConnectReason connectReason );
    virtual bool                isLeaveConnectReason( enum EConnectReason connectReason );
    virtual bool                isUnJoinConnectReason( enum EConnectReason connectReason );
    virtual bool                isSearchConnectReason( enum EConnectReason connectReason );

    virtual bool                addSearchSession( VxGUID& sessionId, SearchParams& searchParams );
    virtual void                removeSearchSession( VxGUID& sessionId );

    virtual void				onUserOnline( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& sessionId );
    virtual void				onUserOffline( VxGUID& onlineId, VxGUID& sessionId );

    //=== vars ===//
    P2PEngine&                  m_Engine; 
    PluginMgr&                  m_PluginMgr; 
    VxNetIdent*                 m_MyIdent{ nullptr };
    PluginBase&                 m_Plugin;
    ConnectionMgr&              m_ConnectionMgr; 
    VxMutex                     m_MgrMutex;
    VxGUIDList                  m_ContactList;
    std::map<VxGUID, SearchParams> m_SearchParamsList;
    VxMutex                     m_SearchParamsMutex;
    UserList                    m_UserList;
    VxMutex                     m_UserListMutex;
    HostedId                    m_HostId;
};

