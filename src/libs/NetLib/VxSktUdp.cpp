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

#include <CoreLib/ISktStatCallbackInterface.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>

#include <time.h>
#include <memory.h>
#include <stdio.h>

#if !defined(TARGET_OS_WINDOWS)
# include <netdb.h>
#endif // defined(TARGET_OS_WINDOWS)

//#define DEBUG_VXSKT_UDP 1

//============================================================================
VxSktUdp::VxSktUdp()
: VxSktBase()
{
	m_eSktType = eSktTypeUdp;
	vx_assert( false );
	// TODO.. needs to setThisSkt to new VxSktUdp and do everything through the shared pointer
}

//============================================================================
VxSktUdp::~VxSktUdp()
{
    if( INVALID_SOCKET != m_Socket )
    {
        closeSkt( eSktCloseSktDestroy );
    }
}

//============================================================================
int VxSktUdp::udpOpen( InetAddress& oLclIp, uint16_t u16Port, bool enableReceive )	
{
	struct addrinfo * poResultAddr = 0;
#ifdef DEBUG_VXSKT_UDP
	LogMsg( LOG_INFO, "udpOpen port %d", u16Port );
#endif // DEBUG_VXSKT_UDP
	int rc = createSocket( oLclIp, u16Port, &poResultAddr );
	if( 0 == rc )
	{
		// for udp always allow reuse
		int reusePort = 1;
		if( 0 != setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reusePort, sizeof( int ) ) )
		{
			LogMsg( LOG_ERROR, "VxSktUdp::udpOpen setReuseSocket error %d", VxGetLastError() );
		}

		if( 0 != bind( m_Socket, ( sockaddr* )&poResultAddr, sizeof( struct addrinfo ) ) )
		{
			LogMsg( LOG_ERROR, "VxSktUdp::udpOpen bind error %d", VxGetLastError() );
		}

#if USE_BIND_LOCAL_IP
		if( false == VxBindSkt( m_Socket, oLclIp, u16Port ) )
		{
			m_rcLastSktError = VxGetLastError();
			if( 0 == m_rcLastSktError )
			{
				m_rcLastSktError = -1;
			}
			rc = m_rcLastSktError;
		}
#endif // #if USE_BIND_LOCAL_IP
	}

	if( ( 0 == rc ) && enableReceive )
	{
		if( VxGetSktStatCallback() )
		{
			VxGetSktStatCallback()->sktConnected4( m_Socket, oLclIp.toString(), eSktTypeUdp, eConnectReasonUnknown );
		}

		startReceive();
	}

	if( rc )
	{
		LogMsg( LOG_ERROR, "ERROR udpOpen error %d", rc );
	}
	else if( false == enableReceive )
	{
		m_bIsConnected = true;
	}

	return rc;
}

//============================================================================
int VxSktUdp::udpOpenUnicast( InetAddress& oLclIp, uint16_t u16Port )
{
	struct addrinfo * poResultAddr = 0;
	int rc = createSocket( oLclIp, u16Port, &poResultAddr );
#if USE_BIND_LOCAL_IP
	if( 0 == rc )
	{
		if( false == VxBindSkt( m_Socket, oLclIp, u16Port ) )
		{
			rc = -1;
		}
	}
#endif // USE_BIND_LOCAL_IP

	if( 0 == rc )
	{
		if( VxGetSktStatCallback() )
		{
			VxGetSktStatCallback()->sktConnected4( m_Socket, oLclIp.toString(), eSktTypeUdp, eConnectReasonUnknown );
		}

		startReceive();
	}

	if( rc )
	{
		LogMsg( LOG_ERROR, "udpOpenUnicast error %d", rc );
	}

	return rc;
}

//============================================================================
int VxSktUdp::createSocket( InetAddress& oLclIp, uint16_t u16Port, struct addrinfo ** ppoResultAddr )	
{
	m_rcLastSktError = 0;
	if( isConnected() )
	{
		closeSkt(eSktCloseUdpCreate);
	}

	m_LclIp = oLclIp;
	m_strLclIp = m_LclIp.toString();
	EIpAddrType addrType = VxGetIpAddrType( m_strLclIp.c_str() );

	m_LclIp.setPort( u16Port );
	m_RmtIp.setPort( u16Port );
	m_strRmtIp.clear();
	m_eSktCallbackReason = eSktCallbackReasonConnecting;

	struct addrinfo * poResultAddr;
	struct addrinfo oHints;
	VxFillHints( oHints, addrType );

	char as8Port[16];
	sprintf( as8Port, "%d", u16Port);

	int err;
	if( 0 != ( err = getaddrinfo( NULL, as8Port, &oHints, &poResultAddr ) ) )
	{
		LogMsg( LOG_ERROR, "VxSktUdp::%s ERROR %d", __func__, err );
		return err;
	}

	* ppoResultAddr = poResultAddr;
	m_Socket = socket( poResultAddr->ai_family, poResultAddr->ai_socktype, poResultAddr->ai_protocol );
	if( INVALID_SOCKET == m_Socket )
	{
		// create socket error
		m_eSktCallbackReason = eSktCallbackReasonConnectError;
		m_rcLastSktError = VxGetLastError();
		LogMsg( LOG_ERROR, "VxSktUdp::%s: socket create error %s", __func__, VxDescribeSktError( m_rcLastSktError ) );
		if( m_pfnReceive )
		{
			m_pfnReceive( getThisSkt(), getRxCallbackUserData() );
		}

		return m_rcLastSktError;
	}
	else
	{
		u_int yes = 1;
		if( 0 > setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, ( char* )&yes, sizeof( yes ) ) )
		{
			LogMsg( LOG_ERROR, "VxSktBase::udpOpen: Reusing ADDR failed error %s", VxDescribeSktError( m_rcLastSktError ) );
		}
	}

	LogModule( eLogSkt, LOG_INFO, "VxSktUdp::%s Success port %d skt handle %d ip %s", __func__, u16Port, m_Socket, m_strLclIp.c_str() );

	return m_rcLastSktError;
}

//============================================================================
void VxSktUdp::startReceive( void )	
{
	m_bIsConnected = true;
	if( m_pfnReceive )
	{
		// tell user we connected
		//vx_assert( m_pfnReceive );
		m_pfnReceive( getThisSkt(), getRxCallbackUserData() );

		// make a useful thread name
		std::string strThreadName;
		StdStringFormat( strThreadName, "VxSktBaseUDP%d", m_SktNumber );
		startReceiveThread( strThreadName.c_str() );
	}
}

//============================================================================
//! send data to given ip 
int  VxSktUdp::sendTo(	const char*		pData,		// data to send
							int				iDataLen,	// data len
							const char*		pRmtIp, 	// destination ip in dotted format
							uint16_t		u16Port )	// port to send to ( if 0 then port specified when opened )
{
	InetAddress oAddr( pRmtIp );
	return sendTo( pData, iDataLen, oAddr, u16Port );
}

//============================================================================
//! send data to given ip 
int  VxSktUdp::sendToMulticast(	const char*		pData,				// data to send
									int				iDataLen,			// data len
									const char*		muliticastGroupIp, 	// destination multicast group ip in dotted format
									uint16_t		u16Port )			// port to send to ( if 0 then port specified when opened )
{
	InetAddress oAddr( muliticastGroupIp );
	// it does not seem necessary to join a group to send to it.. just the send has to be to the group address
	return sendTo( pData, iDataLen, oAddr, u16Port );
}

//============================================================================
//! send data to given ip 
int VxSktUdp::sendTo(		const char*		pData,		// data to send
							int				iDataLen,	// data len
							InetAddress&	oRmtIp, 	// destination ip in host ordered u32
							uint16_t		u16Port )	// port to send to ( if 0 then port specified when opened )
{
	if(	( false == isConnected()) 
        || ( m_pfnReceive && !( m_SktRxThread.isThreadCreated() || m_SktRxThread.isThreadRunning() ) ) )
	{
		LogMsg( LOG_ERROR, "VxSktUdp::sendTo: NOT CONNECTED OR INVALID\n" );
		return -1;
	}

	if( 0 == u16Port )
	{
		u16Port = m_LclIp.getPort();
	}

	struct sockaddr_storage oSktAddr;
    oRmtIp.fillAddress( oSktAddr, u16Port );
	std::string ip = oRmtIp.toString();
#ifdef DEBUG_UDP
	LogMsg( LOG_INFO, "VxSktUdp::sendTo: ip %s port %d \n", ip.c_str(), u16Port );
#endif // DEBUG_UDP
	int structLen = oRmtIp.isIPv4() ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_storage);
	int iDataSentLen =  sendto( m_Socket, pData, iDataLen, 0, (struct sockaddr *)&oSktAddr, structLen );
	if( iDataSentLen !=  iDataLen )
	{
		m_rcLastSktError = VxGetLastError();
		if( 0 == m_rcLastSktError )
		{
			m_rcLastSktError = iDataSentLen;
			LogMsg( LOG_ERROR, "VxSktUdp::sendTo: Error 0.. assigning data sent as error %d\n", iDataSentLen );
		}
		else
		{
			LogMsg( LOG_ERROR, "VxSktUdp::sendTo: Error %s\n", VxDescribeSktError( m_rcLastSktError ) );
		}

		return m_rcLastSktError;
	}

	return 0;
}




