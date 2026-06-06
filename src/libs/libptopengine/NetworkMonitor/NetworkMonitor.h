#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <string>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxSemaphore.h>
#include <CoreLib/VxThread.h>

class P2PEngine;

class NetworkMonitor
{
public:
    const int NET_MONITOR_CHECK_INTERVAL_SEC = 10;

    NetworkMonitor() = delete;
	NetworkMonitor( P2PEngine& engine );
    ~NetworkMonitor() = default;

	void						networkMonitorStartup( void );
	void						networkMonitorShutdown( void );

	void						onOncePerSecond( void );

    bool                        getIsInitialized( void )                    { return m_bIsStarted; }
    void                        setIsInternetAvailable( bool isAvail );
    bool                        getIsInternetAvailable( void )              { return m_InternetAvailable; }

    void                        doNetworkConnectTestThread( VxThread* startupThread );

protected:
    void                        triggerDetermineNetworkState( void );

    void                        triggerDetermineIp( void );
    std::string                 determineLocalIp( void );
    bool                        requiresAnOpenPort( void );

	P2PEngine&					m_Engine;
	bool						m_bIsStarted{ false };
    bool						m_InternetAvailable{ false };
    int							m_iCheckInterval{ NET_MONITOR_CHECK_INTERVAL_SEC / 2 }; // shorten time till first connection test
    VxThread                    m_NetMonitorThread;
    VxSemaphore                 m_NetSemaphore;

	std::string					m_strPreferredAdapterIp;
	std::string					m_strCellNetIp;
	std::string					m_strLastFoundIp;
    uint64_t                    m_LastConnectAttemptTimeGmtMs{ 0 };
    uint64_t                    m_LastConnectSuccessTimeGmtMs{ 0 };
    bool                        m_ConnectAttemptSucessfull{ false };
    bool                        m_ConnectAttemptCompleted{ false };
    std::string                 m_ConnectedLclIp;

    std::string                 m_LastConnectedLclIp;
    bool                        m_LastConnectAttemptSuccessfull{ false };
};
