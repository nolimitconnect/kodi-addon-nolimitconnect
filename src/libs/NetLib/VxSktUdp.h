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

#include <CoreLib/InetAddress.h>
#include "VxSktBase.h"

class VxSktUdp : public VxSktBase
{
public:
	VxSktUdp();	
    virtual ~VxSktUdp();

	//! open a udp port
	virtual int					udpOpen( InetAddress& oLclIp, uint16_t u16Port = 54321, bool enableReceiveThread = true );
	virtual int					udpOpenUnicast( InetAddress& oLclIp, uint16_t u16Port );

	//! send data to given ip 
	virtual int					sendTo(	const char*		pData,			// data to send
										int				iDataLen,		// data length
										InetAddress&	u32RmpIp,		// destination ip
										uint16_t		u16Port = 0 );	// port to send to ( if 0 then port specified when opened )

	//! send data to given ip 
	virtual int					sendTo(	const char*		pData,			// data to send
										int				iDataLen,		// data length
										const char*		pRmpIp,			// destination ip in dotted format
										uint16_t		u16Port = 0 );	// port to send to ( if 0 then port specified when opened )

	//! send data to given ip 
	virtual int					sendToMulticast(	const char*		pData,				// data to send
													int				iDataLen,			// data length
													const char*		muliticastGroupIp,	// destination multicast group ip in dotted format
													uint16_t		u16Port = 0 );		// port to send to ( if 0 then port specified when opened )
protected:
	int							createSocket( InetAddress& oLclIp, uint16_t u16Port, struct addrinfo ** ppoResultAddr );
	void						startReceive( void );
};



