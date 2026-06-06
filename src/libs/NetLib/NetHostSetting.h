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

#include <CoreLib/VxDefs.h>

#include <string>

class NetHostSetting
{
public:
	NetHostSetting() = default;
	virtual ~NetHostSetting() = default;

	NetHostSetting&				operator =( const NetHostSetting& rhs );
    bool						operator == ( const NetHostSetting& rhs ) const;
    bool						operator != ( const NetHostSetting& rhs ) const;

	void						setNetHostSettingName( const char* settingName )			{ m_NetHostSettingName = settingName; }
	std::string&				getNetHostSettingName( void )								{ return m_NetHostSettingName; }

	void						setNetworkKey( const char* networkName )					{ m_NetworkName = networkName; }
	std::string&				getNetworkKey( void )										{ return m_NetworkName; }

	void						setNetworkHostUrl( const char* netHostUrl )				    { m_NetworkHostUrl = netHostUrl; }
	std::string&				getNetworkHostUrl( void )								    { return m_NetworkHostUrl; }
    void						setConnectTestUrl( const char* netServiceUrl )		        { m_NetConnectTestUrl = netServiceUrl; }
    std::string&				getConnectTestUrl( void )								    { return m_NetConnectTestUrl; }

    void						setRandomConnectUrl( const char* netServiceUrl )		    { m_NetRandomConnectUrl = netServiceUrl; }
    std::string&				getRandomConnectUrl( void )								    { return m_NetRandomConnectUrl; }
    void						setGroupHostUrl( const char* netServiceUrl )		        { m_GroupHostUrl = netServiceUrl; }
    std::string&				getGroupHostUrl( void )								        { return m_GroupHostUrl; }
    void						setChatRoomHostUrl( const char* netServiceUrl )		        { m_ChatRoomHostUrl = netServiceUrl; }
    std::string&				getChatRoomHostUrl( void )								    { return m_ChatRoomHostUrl; }

    void						setUserSpecifiedExternIpAddr( const char* ipAddr )		    { m_ExternIpAddr = ipAddr; }
    std::string&				getUserSpecifiedExternIpAddr( void )						{ return m_ExternIpAddr; }

    void                        setUseUpnpPortForward( bool useUpnp )                       { m_UseUpnp = useUpnp; }
    bool                        getUseUpnpPortForward( void )                               { return m_UseUpnp; }
    void                        setTcpPort( uint16_t tcpPort )                              { m_TcpPort = tcpPort; }
    uint16_t                    getTcpPort( void )                                          { return m_TcpPort; }
    void                        setFirewallTestType( int32_t firewallTestType )             { m_FirewallType = firewallTestType; }
    int32_t                     getFirewallTestType( void )                                 { return m_FirewallType; }

    void                        setUseIpv6( bool useIpv6 )                                  { m_UseIpv6 = useIpv6; }
    bool                        getUseIpv6( void )                                          { return m_UseIpv6; }

    void                        resetToDefaultSettings( bool ipv6 );

protected:
	//=== vars ===//
    std::string					m_NetHostSettingName;
    std::string					m_NetworkName;

    std::string					m_NetworkHostUrl;
	std::string					m_NetConnectTestUrl;
    std::string					m_NetRandomConnectUrl;
    std::string					m_GroupHostUrl;
    std::string					m_ChatRoomHostUrl;
    std::string					m_ExternIpAddr;

    int32_t                     m_FirewallType{ 0 };
    bool						m_UseUpnp{ false };
    bool						m_UseIpv6{ false };
    uint16_t					m_TcpPort{ 0 };
};
