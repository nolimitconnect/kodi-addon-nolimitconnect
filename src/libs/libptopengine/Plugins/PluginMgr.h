#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <ConnectIdListMgr/ConnectIdListCallback.h>
#include <GuiInterface/IFromGui.h>
#include <NetServices/NetServiceUtils.h>
#include <Network/ConnectRequest.h>

#include <CoreLib/VxMutex.h>

#include <string>
#include <vector>

class PluginBase;
class IToGui;
class P2PEngine;
class MediaProcesser;
class PktAnnounce;
class PluginSetting;
class BigListMgr;
class VxPktHdr;

class PluginMgr : public ConnectIdListCallback
{
public:
	class AutoMgrLock
	{
	public:
		AutoMgrLock( PluginMgr * pluginMgr ) : m_Mutex(pluginMgr->getPluginMgrMutex())	{ m_Mutex.lock(); }
		~AutoMgrLock()																	{ m_Mutex.unlock(); }
		VxMutex&				m_Mutex;
	};

	PluginMgr( P2PEngine& engine );
	virtual ~PluginMgr();

	void						pluginMgrStartup( void );
	void						pluginMgrShutdown( void );

    IToGui&						getToGui( void );
	P2PEngine&					getEngine( void )									{ return m_Engine; }

	PluginBase*					getPlugin( EPluginType pluginType );
	EAppState					getPluginState( EPluginType pluginType );
	void						setPluginState( EPluginType pluginType, EAppState ePluginState );
	VxMutex&					getPluginMgrMutex( void )							{ return m_PluginMgrMutex; }
	PktAnnounce&				getPktAnnounce( void )								{ return m_PktAnn; }

	EFriendState				getPluginPermission( EPluginType pluginType );
	void						setPluginPermission( EPluginType pluginType, EFriendState ePluginPermission );

    bool                        setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs = 0 );
    void                        onPluginSettingChange( PluginSetting& pluginSetting, int64_t modifiedTimeMs = 0 );

	virtual void				onMyOnlineUrlIsValid( bool isValidUrl );

	virtual void				fromGuiUserLoggedOn( void );

	virtual bool                fromGuiStartPluginSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual void				fromGuiStopPluginSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() );
	virtual bool				fromGuiIsPluginInSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId = VxGUID::nullVxGUID() );

	virtual bool				fromGuiTodGameActionSend( EPluginType pluginType, VxGUID& onlineId, ETodGameAction todGameAction );

	virtual int					fromGuiDeleteFile( std::string& fileName, bool shredFile );

    virtual bool                pluginApiTxPacket(  EPluginType			pluginType,
													const VxGUID&		onlineId,
                                                    std::shared_ptr<VxSktBase>&			sktBase,
                                                    VxPktHdr*			poPkt,
                                                    EPluginType         overridePlugin = ePluginTypeInvalid );
	void						pluginApiLog( EPluginType pluginType, const char* pMsg, ... );
	virtual EPluginAccess	    pluginApiGetPluginAccessState( EPluginType pluginType, VxNetIdent* netIdent );
	virtual VxNetIdent*			pluginApiGetMyIdentity( void );
	virtual VxNetIdent*			pluginApiFindUser( const char* pUserName );

	virtual void				pluginApiPlayJpgVideo( EPluginType pluginType, VxNetIdent* netIdent, std::shared_ptr<CamJpgVideo>& jpgVideo );

	virtual void				pluginApiWantMediaInput( EPluginType pluginType, EMediaInputType mediaType, EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput );

	virtual bool				pluginApiSktConnectTo(	EPluginType			pluginType,		// plugin id
														VxNetIdent*			netIdent,			// identity of contact to connect to
														int					pvUserData,			// plugin defined data
														std::shared_ptr<VxSktBase>&		ppoRetSkt, 			// returned Socket
														EConnectReason		connectReason = eConnectReasonPlugin );	
	virtual void				pluginApiSktClose( ESktCloseReason closeReason, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				pluginApiSktCloseNow( ESktCloseReason closeReason, std::shared_ptr<VxSktBase>& sktBase );
	void						pluginApiToGuiSessionEnded(	EPluginType		pluginType,		// plugin
															VxNetIdent*		netIdent,			// identity of friend
															int				pvUserData,			// plugin defined data
															EOfferResponse	offerResponse );	// reason session ended

	VxNetIdent*					pluginApiOnlineIdToIdentity( VxGUID& oOnlineId );

	virtual void				onAppStateChange( EAppState eAppState );
	virtual void				onAppStartup( void );
	virtual void				onAppShutdown( void );

	virtual void				fromGuiListAction( EListAction listAction );

	virtual void				onOncePerSecond( void );
    virtual void				onThreadOncePer15Minutes( void );
	virtual void				onAfterUserLogOnThreaded( void );

	virtual void				onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );	

	void						callbackConnectionStatusChange( ConnectId& connectId, bool isOnline ) override;

	virtual void				fromGuiRelayPermissionCount( int userPermittedCount, int anonymousCount ); 
	virtual bool				fromGuiSendAsset( AssetBaseInfo& assetInfo );
	virtual bool				fromGuiMultiSessionAction( EMSessionAction mSessionAction, VxGUID& onlineId, int pos0to100000, VxGUID lclSessionId = VxGUID::nullVxGUID() );

	virtual void				fromGuiUpdatePluginPermission( EPluginType pluginType, EFriendState pluginPermission );

	virtual void				fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing );

	virtual EPluginAccess		canAcceptNewSession( EPluginType pluginType, VxNetIdent* netIdent );

	//! return true if access ok
	bool						canAccessPlugin( EPluginType pluginType, VxNetIdent* netIdent );

	void						replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );

	void						handleNonSystemPackets( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );
	void						handleFirstNetServiceConnection( std::shared_ptr<VxSktBase>& sktBase );

	PluginBase*				    findPlugin( EPluginType pluginType );
    PluginBase*				    findHostClientPlugin( EHostType hostType );
    PluginBase*				    findHostServicePlugin( EHostType hostType );

	void						leavePreviousHost( GroupieId& groupieId );

	void						onNetworkConnectionReady( bool requiresRelay );

	bool						isPluginMgrReady( void ) { return m_PluginMgrInitialized; }

protected:
	bool						isValidPluginNum( uint8_t u8PluginNum );
    PluginBase*                 hostClientToPlugin( EHostType hostType );
    PluginBase*                 hostServiceToPlugin( EHostType hostType );

	//=== vars ===//
	P2PEngine&					m_Engine;
	BigListMgr&					m_BigListMgr;
	PktAnnounce&				m_PktAnn;
	
	VxMutex						m_PluginMgrMutex;

	bool						m_PluginMgrInitialized;

	std::vector<PluginBase*>	m_aoPlugins;
	NetServiceUtils				m_NetServiceUtils;
};
