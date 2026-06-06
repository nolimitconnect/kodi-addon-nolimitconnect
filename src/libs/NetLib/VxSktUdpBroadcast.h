#pragma once
//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktUdp.h"

class VxSktUdpBroadcast : public VxSktUdp
{
public:
	
	VxSktUdpBroadcast();
	
	virtual ~VxSktUdpBroadcast() = default;

	//! startup broadcast socket.. if pBroadcastIp is null
	//! then will broadcast to 255.255.255.255 
	virtual int32_t udpBroadcastOpen(	InetAddress& oIpAddr,
									uint16_t u16Port = 12345,			// port to listen on
									const char* pBroadcastIp = "255.255.255.255" ); // broadcast to address
	//virtual int32_t udpBroadcastOpen(	InetAddress& oIpAddr,
	//								uint16_t u16Port = 12345,			// port to listen on
	//								const char* pBroadcastIp = "192.168.60.21"  ); // broadcast to address

	//! broadcast data 
	virtual int32_t broadcastData(	const char*	pData,			// data to send
									int				iDataLen,		// data length
									uint16_t				u16Port = 0 );	// port to send to ( if 0 then port specified when opened )

	
	std::string			m_strBroadcastIp;	// ip to broadcast to in dotted form
    InetAddress			m_BroadcastIp;     // ip to broadcast to in host ordered binary form
};



