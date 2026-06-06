//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetHostSetting.h"

#include "NetHostSettingDefs.h"

//============================================================================
NetHostSetting& NetHostSetting::operator =( const NetHostSetting& rhs )
{
	if( this != &rhs )
	{
		m_NetHostSettingName		= rhs.m_NetHostSettingName;
		m_NetworkName				= rhs.m_NetworkName;
		m_NetworkHostUrl		    = rhs.m_NetworkHostUrl;
        m_NetConnectTestUrl         = rhs.m_NetConnectTestUrl;
        m_NetRandomConnectUrl       = rhs.m_NetRandomConnectUrl;
        m_GroupHostUrl              = rhs.m_GroupHostUrl;
        m_ChatRoomHostUrl           = rhs.m_ChatRoomHostUrl;
        m_ExternIpAddr              = rhs.m_ExternIpAddr;
        m_FirewallType              = rhs.m_FirewallType;
        m_UseUpnp                   = rhs.m_UseUpnp;
        m_UseIpv6                   = rhs.m_UseIpv6;
        m_TcpPort	                = rhs.m_TcpPort;
	}

	return *this;
}

//============================================================================
bool NetHostSetting::operator == ( const NetHostSetting& rhs ) const
{
    return 	m_NetHostSettingName == rhs.m_NetHostSettingName &&
        m_NetworkName == rhs.m_NetworkName &&
        m_NetworkHostUrl == rhs.m_NetworkHostUrl &&
        m_NetConnectTestUrl == rhs.m_NetConnectTestUrl &&
        m_NetRandomConnectUrl == rhs.m_NetRandomConnectUrl &&
        m_GroupHostUrl == rhs.m_GroupHostUrl &&
        m_ChatRoomHostUrl == rhs.m_ChatRoomHostUrl &&
        m_ExternIpAddr == rhs.m_ExternIpAddr &&
        m_FirewallType == rhs.m_FirewallType &&
        m_UseUpnp == rhs.m_UseUpnp &&
        m_UseIpv6 == rhs.m_UseIpv6 &&
        m_TcpPort == rhs.m_TcpPort;
}

//============================================================================
bool NetHostSetting::operator != ( const NetHostSetting& rhs ) const
{
    return !( *this == rhs );
}

//============================================================================
void NetHostSetting::resetToDefaultSettings( bool ipv6 )
{
    m_NetworkName = NET_DEFAULT_NETWORK_KEY;
    m_TcpPort = NET_DEFAULT_NETSERVICE_PORT;

    m_UseIpv6 = ipv6;
    m_UseUpnp = true;
    m_FirewallType = 0;
    m_ExternIpAddr.clear();
    if( ipv6 )
    {
        m_NetHostSettingName = "default ipv6";

        m_NetworkHostUrl = NET_DEFAULT_NET_HOST_URL_IPV6;
        m_NetConnectTestUrl = NET_DEFAULT_CONNECT_TEST_URL_IP6;
        m_NetRandomConnectUrl = NET_DEFAULT_RANDOM_CONNECT_URL_IPV6;
        m_GroupHostUrl = NET_DEFAULT_GROUP_HOST_URL_IPV6;
        m_ChatRoomHostUrl = NET_DEFAULT_CHAT_ROOM_HOST_URL_IPV6;
    }
    else
    {
        m_NetHostSettingName = "default ipv4";

        m_NetworkHostUrl = NET_DEFAULT_NET_HOST_URL_IPV4;
        m_NetConnectTestUrl = NET_DEFAULT_CONNECT_TEST_URL_IPV4;
        m_NetRandomConnectUrl = NET_DEFAULT_RANDOM_CONNECT_URL_IPV4;
        m_GroupHostUrl = NET_DEFAULT_GROUP_HOST_URL_IPV4;
        m_ChatRoomHostUrl = NET_DEFAULT_CHAT_ROOM_HOST_URL_IPV4;
    }
}
