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

#include "ContactList.h"

#include <CoreLib/VxGUIDList.h>

#include <map>

class BigListInfo;
class VxSktBase;
class P2PEngine;
class BigListMgr;
class VxPktHdr;

class RcConnectInfo
{
public:
	RcConnectInfo()
		: m_SktBase(0)
		, m_BigListInfo(0)
		, m_bIsRelayClient(0)
		, m_bIsRelayServer(0)
	{
	}

	RcConnectInfo( std::shared_ptr<VxSktBase>& sktBase, BigListInfo * poBigListInfo, bool bIsRelayClient = false, bool bIsRelayServer = false  )
		: m_SktBase(sktBase)
		, m_BigListInfo(poBigListInfo)
		, m_bIsRelayClient(bIsRelayClient)
		, m_bIsRelayServer(bIsRelayServer)
	{
	}

	bool						isConnectionValid( void );

	bool						isRelayServer( void )							{ return m_bIsRelayServer; }
	void						setIsRelayServer( bool bIsServer )				{ m_bIsRelayServer = bIsServer; }
	bool						isRelayClient( void )							{ return m_bIsRelayClient; }
	void						setIsRelayClient( bool bIsClient )				{ m_bIsRelayClient = bIsClient; }
	BigListInfo	*				getBigListInfo( void )							{ return m_BigListInfo; }
	std::shared_ptr<VxSktBase>&	getSkt( void )									{ return m_SktBase; }

	std::shared_ptr<VxSktBase>	m_SktBase;
	BigListInfo	*				m_BigListInfo;		
	bool						m_bIsRelayClient;
	bool						m_bIsRelayServer;
};

class P2PConnectList
{
public:
	P2PConnectList( P2PEngine& engine );
    virtual ~P2PConnectList() = default;

	void						connectListLock( void );
	void						connectListUnlock( void );

	void						broadcastSystemPkt( VxPktHdr* pkt, bool onlyIncludeMyContacts );
	void						broadcastSystemPkt( VxPktHdr* pkt, VxGUIDList& retIdsSentPktTo, bool onlyIncludeMyContacts = false );

    void						fromGuiChangeMyFriendshipToHim(	const VxGUID&		onlineId,
																enum EFriendState	eMyFriendshipToHim,
																enum EFriendState	eHisFriendshipToMe );

	void						onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );

    bool						isContactConnected( VxGUID onlineId );
	bool						isRelayRequired( void )								{ return m_bRequireRelayService; }
	void						setIsRelayRequired( bool bRequireRelayService )		{ m_bRequireRelayService = bRequireRelayService; }
	bool						isMyRelayAvailable( void )							{ return m_RelayServiceConnection?1:0; }

	RcConnectInfo *				addConnection( std::shared_ptr<VxSktBase>& sktBase, BigListInfo * poBigListInfo, bool bNewContact = false );
    RcConnectInfo *				addConnection( const VxGUID& oOnlineId, RcConnectInfo * poInfo, bool bNewContact = false );
    RcConnectInfo *				findConnection( const VxGUID& oOnlineId, bool listIsLocked );
    void						removeConnection( const VxGUID& oOnlineId );
	void						removeSocket( std::shared_ptr<VxSktBase>& sktBase, bool listIsLocked );

	void						removeContactInfo( VxConnectInfo& contactInfo );
	
	//=== vars ===//
#ifdef TARGET_OS_ANDROID
	std::map<VxGUID, RcConnectInfo *> m_ConnectList;
	typedef std::map<VxGUID, RcConnectInfo *>::iterator ConnectListIter;
	std::map<VxGUID, RcConnectInfo *>&				getConnectedList( void )		{ return m_ConnectList; }
#else
	std::map<VxGUID, RcConnectInfo *, cmp_vxguid> m_ConnectList;
	typedef std::map<VxGUID, RcConnectInfo *, cmp_vxguid>::iterator ConnectListIter;
	std::map<VxGUID, RcConnectInfo *, cmp_vxguid>&	getConnectedList( void )		{ return m_ConnectList; }
#endif

	RcConnectInfo *				m_RelayServiceConnection;
	std::vector<RcConnectInfo *>m_RelayServerConnectedList;

protected:
	//=== vars ===//
	P2PEngine&					m_Engine;
	BigListMgr&					m_BigListMgr;

	bool						m_bRequireRelayService;

	VxMutex						m_ConnectListMutex;
};
