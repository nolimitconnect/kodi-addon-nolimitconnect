#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SessionMgrBase.h"
#include "PluginSessionBase.h"
#include "TxSession.h"
#include "RxSession.h"
#include "P2PSession.h"

#include <map>

class ConnectId;
class OfferBaseInfo;

class PluginSessionMgr : public SessionMgrBase
{
public:
	PluginSessionMgr() = delete; // don't allow default constructor
	PluginSessionMgr( const PluginSessionMgr& ) = delete; // don't allow copy constructor

	PluginSessionMgr( P2PEngine& engine, PluginBase& plugin, PluginMgr& pluginMgr );
	virtual ~PluginSessionMgr() = default;

	std::vector<PluginSessionBase*>&	getSessions( void )			{ return m_aoSessions; }
	size_t						getSessionCount( void )				{ return m_aoSessions.size(); }
	int							getTxSessionCount( bool pluginIsLocked = false );

	virtual void				replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual	void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );
	virtual void				cancelSessionByOnlineId( VxGUID& onlineId );

	virtual void				onContactOnlineStatusChange( ConnectId& connectId, bool isOnline );

    virtual bool				fromGuiIsPluginInSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual void				fromGuiStopPluginSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual bool				fromGuiMakePluginOffer(	bool pluginIsLocked, VxGUID& onlineId, OfferBaseInfo& offerInfo );
	virtual bool				fromGuiOfferReply( bool pluginIsLocked, VxGUID& onlineId, OfferBaseInfo& offerInfo );

	virtual void				onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktSessionStopReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	void						addSession( VxGUID& sessionId, PluginSessionBase* session, bool pluginIsLocked );

	PluginSessionBase*			findPluginSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked );
	PluginSessionBase*			findPluginSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked );

	P2PSession *				findP2PSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked );
	P2PSession *				findP2PSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked );
	P2PSession *				findOrCreateP2PSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked);
    P2PSession *				findOrCreateP2PSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase,
																		bool pluginIsLocked, VxGUID lclSessionId = VxGUID::nullVxGUID() );

	TxSession *					findTxSessionBySessionId( bool pluginIsLocked, VxGUID& sessionId );
	TxSession *					findTxSessionByOnlineId( bool pluginIsLocked, VxGUID& onlineId );
	TxSession *					findOrCreateTxSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked );
    TxSession *					findOrCreateTxSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase,
																		bool pluginIsLocked, VxGUID lclSessionId = VxGUID::nullVxGUID() );

	RxSession *					findRxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked );
	RxSession *					findRxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked );
	RxSession *					findOrCreateRxSessionWithSessionId( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, bool pluginIsLocked );
    RxSession *					findOrCreateRxSessionWithOnlineId( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase,
																	bool pluginIsLocked, VxGUID lclSessionId = VxGUID::nullVxGUID() );

	void						endPluginSession( PluginSessionBase* session, bool pluginIsLocked );
	void						endPluginSession( VxGUID& sessionId, bool pluginIsLocked );

	void						removeTxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked );
	void						removeTxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked );
	void						removeRxSessionBySessionId( VxGUID& sessionId, bool pluginIsLocked );
	void						removeRxSessionByOnlineId( VxGUID& onlineId, bool pluginIsLocked );

	// returns true if found and removed session
	bool						removeSessionBySessionId( bool pluginIsLocked, VxGUID& sessionId, EOfferResponse offerResponse = eOfferResponseEndSession );
	bool						removeSession( bool pluginIsLocked, VxGUID& onlineId, VxGUID& sessionId, EOfferResponse offerResponse, bool fromGui = false );
	void						removeAllSessions( bool testSessionsOnly = false );

protected:
	void						doEndAndEraseSession( PluginSessionBase* sessionBase, EOfferResponse offerResponse, bool pluginIsLocked, bool sendSessionStop = true );

	//=== vars ===//
	std::vector<PluginSessionBase*>	m_aoSessions;
};

