#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxHackerMgr.h"
#include "VxListenLogic.h"
#include "VxSktBaseMgr.h"

#include <vector>

// implements a manager to manage accept sockets
class VxServerMgr : public VxSktBaseMgr, public VxHackerMgr
{
public:
	VxServerMgr();
	virtual ~VxServerMgr() = default;

    void				        sktMgrStartup( bool ipv6 ) override;
	void				        sktMgrShutdown( void ) override;

    int                         getMgrId( void ) const { return m_iMgrId; }

    bool						startListening( bool ipv6, uint16_t port, bool usePortForwardIfEnabled );
    void						stopListening( bool ipv6 );

    bool				        isListening( bool ipv6 );

    void                        setListenPort( uint16_t port );
    uint16_t                    getListenPort( void );

    void                        setLocalIp( std::string ipAddr );
    std::string                 getLocalIp( bool ipv6 );

    virtual void				setIsReadyToAcceptConnections( bool ipv6, bool isReady );
    virtual bool				getIsReadyToAcceptConnections( bool ipv6 );

	// overrides SktMgrBase
	//! make a new socket... give derived classes a chance to override
	virtual std::shared_ptr<VxSktBase> makeNewAcceptSkt( bool ipv6 );  

    void						setUpnpEnable( bool enable );
    bool						getUpnpEnable( void );

    void						setUseIpv6( bool enable );
    bool						getUseIpv6( void );

    void 						listenSktWasOpened( bool ipv6, SOCKET listenSkt );
    void 						listenSktWasClosed( bool ipv6, SOCKET listenSkt );

    int32_t 						acceptConnection( bool ipv6, VxThread* poVxThread, SOCKET listenSkt, uint16_t port );

    virtual void				lockListenSettings( void ) 					{ m_ListenSettingsMutex.lock(); }
	virtual void				unlockListenSettings( void )				{ m_ListenSettingsMutex.unlock(); }

protected:
    bool                        addPortForward( bool ipv6, uint16_t port );
    bool                        isPortForwarded( void ) { return m_PortForwardResult; }

    VxListenLogic               m_ListenLogicIpv4;
    VxListenLogic               m_ListenLogicIpv6;

    VxMutex                     m_ListenSettingsMutex;
    uint16_t                    m_ListenPort{ 0 };

    std::string                 m_LocalIpAddrIpv4;
    std::string                 m_LocalIpAddrIpv6;

    bool                        m_PortForwardResult{ false };

    int							m_iMgrId{ 0 };			// unique id for this manager

    static constexpr size_t     m_u32MaxConnections = 30000;
    static int					m_iAcceptMgrCnt;				    // number of managers created
    int32_t						m_rcLastError = 0;					// last error that occurred

    bool						m_IsReadyToAcceptConnectionsIpv6{ false };
    bool						m_IsReadyToAcceptConnectionsIpv4{ false };   

    int64_t						m_LastWatchdogKickMs{ 0 };

    int64_t						m_LastListenActivityMs{ 0 };
    
};

