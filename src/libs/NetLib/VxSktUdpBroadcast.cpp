//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktUdpBroadcast.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxSktUtil.h>

VxSktUdpBroadcast::VxSktUdpBroadcast()
{
	m_eSktType = eSktTypeUdpBroadcast;
	VxSocketsStartup();
}

//============================================================================
//! startup broadcast socket if pBroadcastIp is null
//! then will broadcast to 255.255.255.255 
int32_t VxSktUdpBroadcast::udpBroadcastOpen(	InetAddress& oIpAddr,
											uint16_t u16Port,	
											const char* pBroadcastIp )
{
	m_LclIp.setPort( u16Port );
	m_RmtIp.setPort( u16Port );
	if( pBroadcastIp )
	{
		m_strBroadcastIp = pBroadcastIp;
		m_strRmtIp = pBroadcastIp;
		m_BroadcastIp.setIp( pBroadcastIp );
	}
	else
	{
		m_strBroadcastIp = "255.255.255.255";
		m_strRmtIp = m_strBroadcastIp;
		m_BroadcastIp.setIp( m_strBroadcastIp.c_str() );
	}
	m_RmtIp.setIp( m_strBroadcastIp.c_str() );

	// open socket
	int32_t rc = VxSktUdp::udpOpen( oIpAddr, u16Port );
	if( rc )
	{
		return rc;
	}
	// tell socket to broadcast
	bool bBroadcast = true;
	if( SOCKET_ERROR == setsockopt( m_Socket, SOL_SOCKET,SO_BROADCAST, (const char*)&bBroadcast, sizeof(bBroadcast)) ) 
	{
		m_rcLastSktError = VxGetLastError();
		LogMsg( LOG_INFO, "VxSktUdpBroadcast:setsockopt error %s\n", VxDescribeSktError( m_rcLastSktError ) );
		return m_rcLastSktError;
	}
	return 0;
}

//============================================================================
//! broadcast data 
int32_t VxSktUdpBroadcast::broadcastData(	const char*	pData,		// data to send
										int				iDataLen,	// data length
										uint16_t				u16Port )	// port to send to ( if 0 then port specified when opened )
{
	if( 0 == u16Port )
	{
		u16Port = m_LclIp.getPort();
	}
	return sendTo( pData, iDataLen, m_BroadcastIp, u16Port );
}



