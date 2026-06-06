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

#include <CoreLib/VxMutex.h>
#include <CoreLib/VxThread.h>

class VxServerMgr;

class VxListenLogic
{
public:
    
	VxListenLogic() = delete;
    VxListenLogic( VxServerMgr& serverMgr, bool ipv6 );
	virtual ~VxListenLogic() = default;

	void				        sktMgrShutdown( void );

    bool                        getIsIpv6( void ) { return m_IsIpv6;  }

    bool				        getIsListening( void );

    uint16_t					getListenPort( void );

    bool						startListening( uint16_t port );
    void						stopListening( void );

    void				        setIsReadyToAcceptConnections( bool isReady );
    bool				        getIsReadyToAcceptConnections( void );

    bool                        shouldListenAbort( void );
	
    void						listenForConnectionsToAccept( void );

    
	virtual void				lockListenSettings( void ) 					{ m_ListenSettingsMutex.lock(); }
	virtual void				unlockListenSettings( void )				{ m_ListenSettingsMutex.unlock(); }

protected:
    bool				        startListeningThread( void );
	void						stopListeningThread( void );

    void                        setListenSkt( SOCKET skt );
    SOCKET                      getListenSkt( bool setExistingSktToInvalid = false );

    bool                        createNewListenSocket( SOCKET& retListenSock );

    void                        closeListenSocket( void );

    VxServerMgr&                m_ServerMgr;
    bool                        m_IsIpv6{ false };
    bool                        m_IsReadyToAcceptConnections{ false };

    VxMutex                     m_ListenSettingsMutex;
    VxMutex                     m_ListenMutex;
    VxThread					m_ListenThread;		// thread to listen for incoming connections

    int32_t						m_rcLastError = 0;					// last error that occurred
    
    uint16_t					m_ListenPort = 0;				// what port to listen on  
    SOCKET                      m_ListenSocket{INVALID_SOCKET};
    bool                        m_ListenIsEnabled{ false };

    int64_t						m_LastListenActivityMs{ 0 };
};

