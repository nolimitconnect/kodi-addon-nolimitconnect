#pragma once
//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxThread.h>

#include <map>
#include <vector>

class NetAvailStatusCallbackInterface
{
public:
    virtual void				callbackInternetStatusChanged( EInternetStatus internetStatus ) = 0;
    virtual void				callbackNetAvailStatusChanged( ENetAvailStatus netAvalilStatus ) = 0;
};


class HostConnection
{
public:
    HostConnection() = delete;
    HostConnection( EHostType hostType, std::string hostUrl, VxGUID& connectId )
        : m_HostType( hostType )
        , m_HostUrl( hostUrl )
        , m_ConnectId( connectId )
    {
    }

    HostConnection( const HostConnection& rhs )
        : m_HostType( rhs.m_HostType )
        , m_HostUrl( rhs.m_HostUrl )
        , m_ConnectId( rhs.m_ConnectId )
    {
    }

    HostConnection& operator =( const HostConnection& rhs )
    {
        if( this != &rhs )
        {
            m_HostType = rhs.m_HostType;
            m_HostUrl = rhs.m_HostUrl;
            m_ConnectId = rhs.m_ConnectId;
        }

        return *this;
    }

    void                        setConnectionId( VxGUID&  connectId )           { m_ConnectId = connectId; }
    VxGUID&                     getConnectionId( void )                         { return m_ConnectId; }
    void                        clearConnectionId( void )                       { m_ConnectId.clearVxGUID(); }

    std::string&                getHostUrl( void )                              { return m_HostUrl; }

protected:
    EHostType                   m_HostType{ eHostTypeUnknown };
    std::string                 m_HostUrl;
    VxGUID&                     m_ConnectId;
};

class P2PEngine;

// network state accumulator
class NetStatusAccum
{
public:
    NetStatusAccum( P2PEngine& toGui );
    virtual ~NetStatusAccum() = default;

    void                        addNetStatusCallback( NetAvailStatusCallbackInterface* callbackInt );
    void                        removeNetStatusCallback( NetAvailStatusCallbackInterface* callbackInt );

    void                        fromGuiNetworkSettingsChanged( void );

    void                        resetNetworkStatusAll( void );
    void                        resetConnectionTestStatus( void );

    void                        setUseIpv6( bool useIpv6, uint16_t ipPort );
    bool                        getUseIpv6( void ) { return m_UseIpv6; }

    void                        setUseFixedIp( std::string externIp );

    void                        setIsFixedIpAddress( bool fixedIpAddr ) { m_HasFixedIpAddr = fixedIpAddr; }
    bool                        isFixedIpAddress( void )                { return m_HasFixedIpAddr; }

    void                        setInternetAvail( bool avail );
    bool                        isInternetAvailable( void )             { return m_InternetAvail; };

    void                        setConnectionTestAvail( bool avail );
    bool                        isConnectionTestAvailable( void )       { return m_ConnectionTestAvail; };

    void                        setNetHostAvail( bool avail );
    bool                        isNetHostAvailable( void )              { return m_NetworkHostAvail; };

    void                        setDirectConnectTested( bool isTested, bool requiresRelay, std::string& myExternalIp );
    bool                        isDirectConnectTested( void )       { return m_DirectConnectTested; };

    bool                        isNetHostOnlineIdAvailable( void )  { return m_NetHostIdAvail; };
    bool                        isNetworkOnline( void )             { return m_InternetAvail && ( m_HasFixedIpAddr || m_DirectConnectTested ); };
    bool                        isP2POnline( void )                 { return m_InternetAvail && ( m_HasFixedIpAddr || ( m_DirectConnectTested && ( !m_RequriesRelay || m_ConnectedToRelay ) ) ); }
    bool                        isRxPortOpen( void )                { return m_HasFixedIpAddr || ( m_DirectConnectTested && !m_RequriesRelay ); };

    bool                        hasDefaultNetworkKey( void );
    bool                        hasDefaultNetworkUrl( void );

    void                        setConnectToRelay( bool connectedToRelay );

    void                        setFirewallTestType( EFirewallTestType firewallTestType );
    EFirewallTestType           getFirewallTestType( void );
    
    bool                        requiresRelay( void )               { return m_RequriesRelay; };
    void                        getNodeUrl( std::string& retNodeUrl );

    void                        setIpPort( uint16_t ipPort);
    uint16_t                    getIpPort( void );

    void                        setExternalIpAddress( std::string ipAddr );
    std::string                 getExternalIpAddress( void );

    void                        setLocalIpAddress( std::string ipAddr );
    std::string                 getLocalIpAddress( void ); 

    std::string                 getLocalIpAddress( bool ipv6 );  

    std::string                 getLocalIpv4( void );
    std::string                 getLocalIpv6( void );
    std::string                 getMyNetServiceIpAddress( bool isIpv6Connection );

    EInternetStatus             getInternetStatus( void )           { return m_InternetStatus; }
    ENetAvailStatus             getNetAvailStatus( void )           { m_AccumMutex.lock(); ENetAvailStatus status = m_NetAvailStatus;  m_AccumMutex.unlock(); return status;  }

    void                        setJoinedHost( EHostType hostType, std::string hostUrl, VxGUID& connectId );
    bool                        isConnectedToHost( EHostType hostType );
    std::string                 getConnectedHostUrl( EHostType hostType );
    void                        onConnectionLost( VxGUID& connectId );

    bool                        isHostResolved( EHostType hostType );
    bool                        getResolvedHost( EHostType hostType, std::string& retHostUrl );

    void                        setNetworkKey( std::string networkName );
    std::string                 getNetworkKey( void );

    void                        setNetworkHostUrl( std::string netHostUrl );
    std::string                 getNetworkHostUrl( void );
    std::string                 getNetworkHostName( void ); // just host name or ip
    uint16_t                    getNetworkHostPort( void );

    bool                        getIsConnectTestHost( void ) { return m_IsConnectTestHost; }
    bool                        getIsNetworkHost( void ) { return m_IsNetworkHost; }

    // when using connection test host that is not us
    void                        setConnectionTestHostUrl( std::string connectTestUrl );
    std::string                 getConnectionTestHostUrl( void );
    std::string                 getConnectionTestHostName( void ); // just connection test name or ip
    uint16_t                    getConnectionTestHostPort( void );

    bool                        canAnnounceToNlcHost( bool iAmNetHost ); // true if using NLC network key and network host url and port is open

    void                        threadedSetupListen( void );

    bool                        isLocalAndExternIpsTheSame( void );

protected:
    void                        onNetStatusChange( void );

    void                        lockAccum( void ) { m_AccumMutex.lock(); }
    void                        unlockAccum( void ) { m_AccumMutex.unlock(); }

    void                        startSetupListenThread( void );

    P2PEngine&					m_Engine;
    VxMutex                     m_AccumMutex;
    VxMutex                     m_AccumCallbackMutex;
    std::vector<NetAvailStatusCallbackInterface*> m_CallbackList;

    bool                        m_HasFixedIpAddr{ false };

    bool                        m_InternetAvail{ false };
    bool                        m_NetworkHostAvail{ false };
    bool                        m_ConnectionTestAvail{ false };
    bool                        m_DirectConnectTested{ false };
    bool                        m_RequriesRelay{ true }; // assume requires relay until proved does not require relay
    bool                        m_ConnectedToRelay{ false };
    bool                        m_GroupListHostAvail{ false };
    bool                        m_GroupHostAvail{ false };
    bool                        m_IsConnectedGroupHost{ false };
    bool                        m_IsExternalIpValid{ false };

    uint16_t                    m_IpPort{ 0 };
    bool                        m_UseIpv6{ false };
    bool                        m_IsConnectTestHost{ false };
    bool                        m_IsNetworkHost{ false };

    std::string                 m_ExternAddr;
    std::string                 m_LocalAddr;
    std::string                 m_LocalAddrIpv4;
    std::string                 m_LocalAddrIpv6;

    std::string                 m_NetworkKey; // used as key for all encryption

    std::string                 m_NetHostUrl; // full network host url
    std::string                 m_NetHostName; // just host name or ip
    uint16_t                    m_NetHostPort{ 0 }; // network host port

    bool                        m_NetHostIdAvail{ false };
    VxGUID                      m_NetNostOnlineId;

    std::string                 m_ConnectionTestUrl; // full connection test host url
    std::string                 m_ConnectionTestHostName; // just connect test name or ip
    uint16_t                    m_ConnectionTestPort{ 0 }; // connection test host port

    EFirewallTestType           m_FirewallTestType{ eFirewallTestUrlConnectionTest };
    EInternetStatus             m_InternetStatus{ eInternetNoInternet };
    ENetAvailStatus             m_NetAvailStatus{ eNetAvailNoInternet };

    std::map<EHostType, HostConnection> m_HostConnectionMap;

    VxThread                    m_SetupListenThread;
};
