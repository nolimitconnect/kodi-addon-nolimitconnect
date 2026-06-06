//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetSettings.h"

//============================================================================
NetSettings& NetSettings::operator =( const NetSettings& rhs )
{
	if( this != &rhs )
	{
        *( (NetHostSetting*)this ) = rhs;
        m_u16MyMulticastPort        = rhs.m_u16MyMulticastPort;
		m_bMulticastEnable			= rhs.m_bMulticastEnable;
        m_AllowMulticastBroadcast   = rhs.m_AllowMulticastBroadcast;
        m_UserRelayPermissionCount = rhs.m_UserRelayPermissionCount;
        m_SystemRelayPermissionCount = rhs.m_SystemRelayPermissionCount;
        m_AllowUserLocation         = rhs.m_AllowUserLocation;
	}

	return *this;
}
