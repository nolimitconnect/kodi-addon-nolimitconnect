//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktBase.h"

#include "VxSktBaseMgr.h"

#include <CoreLib/InetAddress.h>
#include <CoreLib/ISktStatCallbackInterface.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxResolveHost.h>
#include <CoreLib/VxSktUtil.h>
#include <CoreLib/VxTimeUtil.h>

#include <PktLib/PktTypes.h>
#include <PktLib/PktsImAlive.h>

#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef TARGET_OS_WINDOWS
	#include "Mswsock.h"
#else
    #include <sys/signal.h>
	#include <sys/ioctl.h>
    #include <netdb.h>
#endif // TARGET_OS_WINDOWS

namespace
{
	const int					SKT_RX_RETRY_SLEEP_TIME_MS		= 400;
	const int64_t				IM_ALIVE_TIMEOUT_MS			    = 180000; // 3 minutes very long should be shortened to 65000 after no longer debugging connections
	const int64_t				NET_SERVICE_TIMEOUT_MS			= 180000; 
}

//============================================================================
static void * VxSktBaseReceiveVxThreadFunc( void * pvContext );

std::atomic<int> VxSktBase::m_TotalCreatedSktCnt{ 0 };
std::atomic<int> VxSktBase::m_CurrentSktCnt{ 0 };
std::atomic<int> VxSktBase::m_RunningRxThreadCnt{ 0 };

std::string VxSktBase::m_SktDirConnect{ "->" };
std::string VxSktBase::m_SktDirAccept{ "<-" };
std::string VxSktBase::m_SktDirUdp{  "<->" };
std::string VxSktBase::m_SktDirBroadcast{ "->>" };
std::string VxSktBase::m_SktDirLoopback{ "<==>" };
std::string VxSktBase::m_SktDirUnknown{ "<?\?>" }; // use \? instead of just ? to avoid warning: trigraph ignored
VxGUID VxSktBase::m_LoopbackSocketId{ 8740338989118525749U, 11880411481645876119U };	// !794BEC0096E81135A4DFB34C24F98397!

//============================================================================
VxSktBase::VxSktBase()
: VxSktBuf()
, VxSktThrottle()	
, m_SktNumber(0)
, m_LastImAliveTimeGmtRxMs( GetGmtTimeMs() )
//, m_u8TxSeqNum;			// sequence number used to twart replay attacks ( do not set )
{
	m_TotalCreatedSktCnt++;
	m_SktNumber = m_TotalCreatedSktCnt;
	m_CurrentSktCnt++;
    m_ConnectionId.generateNewVxGUID( m_ConnectionId );
	m_u8TxSeqNum = (uint8_t)rand();
	m_LclIp.setToInvalid();
	m_RmtIp.setToInvalid();

	LogModule( eLogSkt, LOG_VERBOSE, "skt num %d created", m_SktNumber );
#if !defined(TARGET_OS_WINDOWS)
	signal(SIGPIPE, SIG_IGN);
#endif // !defined(TARGET_OS_WINDOWS)
}

//============================================================================
VxSktBase::~VxSktBase()
{
	if( !m_HasBeenShutdown || m_ThisSkt )
	{
		LogModule( eLogSkt, LOG_VERBOSE, "VxSktBase::~VxSktBase skt num %d destroyed without shutdown", m_SktNumber );
	}

	m_CurrentSktCnt--;
}

//============================================================================
void VxSktBase::shutdownSkt( bool closingFromDestructor )
{
	if( !m_HasBeenShutdown )
	{
		m_HasBeenShutdown = true;

		m_bIsConnected = false;
		m_bClosingFromDestructor = closingFromDestructor;
		m_SktRxThread.abortThreadRun( true );

		closeSkt( eSktCloseSktDestroy, true );

		if( !m_bClosingFromRxThread )
		{
			m_SktRxThread.killThread();
		}
	}
}

//============================================================================
bool VxSktBase::toSocketAddrInfo(	int sockType, 
									const char*addr, 
									int port, 
									struct addrinfo **addrInfo, 
									bool isBindAddr )
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_socktype = sockType;
	hints.ai_flags= AI_NUMERICHOST | AI_PASSIVE;
	char portStr[32];
	sprintf(portStr, "%d", port);
	//LogMsg( LOG_INFO, "VxSktBase::toSocketAddrInfo %s:%d", addr, port ); 
	if( 0 != getaddrinfo( addr, portStr, &hints, addrInfo ))
	{
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_INFO, "VxSktBase::toSocketAddrInfo: error %d %s", 
			getLastSktError(),
			VxDescribeSktError( getLastSktError() ) );
		return false;
	}

	if (isBindAddr == true)
	{
		return true;
	}

	hints.ai_family = (*addrInfo)->ai_family;
	freeaddrinfo(*addrInfo);
	if (getaddrinfo(NULL, portStr, &hints, addrInfo) != 0)
	{
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_ERROR, "VxSktBase::toSocketAddrInfo: error %d %s",
			getLastSktError(),
			VxDescribeSktError( getLastSktError() ) );
		return false;
	}

	return true;
}

//============================================================================
bool VxSktBase::toSocketAddrIn(	const char*addr, 
								int port, 
								struct sockaddr_in *sockaddr, 
								bool isBindAddr )
{
	memset(sockaddr, 0, sizeof(sockaddr_in));

	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr->sin_port = htons((uint16_t)port);

	if( true == isBindAddr ) 
	{
		sockaddr->sin_addr.s_addr = inet_addr(addr);
		if (sockaddr->sin_addr.s_addr == INADDR_NONE) 
		{
			struct hostent *poHostEnt = gethostbyname(addr);
			if( NULL == poHostEnt )
			{
				return false;
			}

			memcpy(&(sockaddr->sin_addr), poHostEnt->h_addr, poHostEnt->h_length);
		}
	}

	return true;
}

//============================================================================
bool VxSktBase::bindSocket( struct addrinfo * poResultAddr )	
{
	setLastSktError( 0 );
	if( SOCKET_ERROR == bind( m_Socket, poResultAddr->ai_addr, (int)poResultAddr->ai_addrlen ) )
	{
		// connect error
		m_eSktCallbackReason = eSktCallbackReasonConnectError;
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_ERROR, "VxSktBase::%s bind error %d %s", __func__, 
			getLastSktError(),
			VxDescribeSktError( getLastSktError() ) );
		m_pfnReceive( getThisSkt(), getRxCallbackUserData());
		return false;
	}
	return true;
}

//============================================================================
bool VxSktBase::isIPv6Address(const char*addr)
{
	if( NULL == addr )
	{
		return false;
	}
	std::string addrStr = addr;
	if (addrStr.find(":") != std::string::npos)
	{
		return true;
	}

	return false;
}

//============================================================================
int VxSktBase::getIPv6ScopeID(const char*addr)
{
	if( false == isIPv6Address( addr ) )
	{
		return 0;
	}

	std::string addrStr = addr;
	int pos = (int)addrStr.find("%");
	if (pos == (int)std::string::npos)
	{
		return 0;
	}

	std::string scopeStr = addrStr.substr(pos+1, addrStr.length());
	return atoi(scopeStr.c_str());
}

//============================================================================
const char*VxSktBase::stripIPv6ScopeID( const char*addr, std::string &buf )
{
	std::string addrStr = addr;
	if( true == isIPv6Address( addr ) ) 
	{
		std::string::size_type pos = addrStr.find("%");
		if( pos != std::string::npos )
		{
			addrStr = addrStr.substr(0, pos);
		}
	}

	buf = addrStr;
	return buf.c_str();
}

//============================================================================
void VxSktBase::setTTL( uint8_t ttl )
{
	if( 0 != setsockopt( m_Socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof( ttl )) )
	{
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_ERROR, "VxSktBase::setTTL error %d", getLastSktError() ); 
	}
}

//============================================================================
void VxSktBase::setAllowLoopback( bool allowLoopback )
{
	int32_t loopbackOption = allowLoopback;

	if( 0 != setsockopt( m_Socket, 0, 11, (char *)&loopbackOption, sizeof( loopbackOption )) )
	{
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_ERROR, "VxSktBase::setAllowLoopback error %d", getLastSktError() ); 
	}
}

//============================================================================
void VxSktBase::setAllowBroadcast( bool allowBroadcast )
{
	int32_t broadcastOption = allowBroadcast;

	if( 0 != setsockopt( m_Socket, 0xffff, 32, (char *)&broadcastOption, sizeof( broadcastOption )) )
	{
		setLastSktError( VxGetLastError() );
		LogMsg( LOG_ERROR, "VxSktBase::setAllowBroadcast error %d", getLastSktError() ); 
	}
}

//============================================================================
void VxSktBase::setReceiveCallback( VX_SKT_CALLBACK pfnReceive, void * pvRxCallbackUserData )
{
	m_pfnReceive = pfnReceive;
	m_pvRxCallbackUserData = pvRxCallbackUserData;
}

//============================================================================
//! Set Transmit Callback ( optional for transmit statistics )
void VxSktBase::setTransmitCallback( VX_SKT_CALLBACK pfnTransmit, void * pvTxCallbackUserData )
{
	m_pfnTransmit = pfnTransmit;
	m_pvTxCallbackUserData = pvTxCallbackUserData;
}

//============================================================================
//! set socket to blocking or not
int32_t VxSktBase::setSktBlocking( bool bBlock )
{
	int32_t rc = ::VxSetSktBlocking( m_Socket, bBlock );
	if ( rc )
	{
		setLastSktError( rc );
		LogMsg( LOG_ERROR, "VxSktBase::setSktBlocking skt %d ioctlsocket error %s", m_SktNumber, VxDescribeSktError( getLastSktError() ) );
	}

	return rc;
}

//============================================================================
void VxSktBase::updateLastActiveTime( void )					
{ 
	setLastActiveTimeMs( GetGmtTimeMs() ); 
}

//============================================================================
void VxSktBase::updateLastSessionTime( void )
{
	setLastSessionTimeMs( GetGmtTimeMs() );
}

//============================================================================
int32_t VxSktBase::connectTo(	InetAddress&	oLclIp,
							    const char*		pIpUrlOrIp,				// remote ip 
							    uint16_t		u16Port,				// port to connect to
							    int				iTimeoutMilliSeconds)	// milli seconds before connect attempt times out
{
	m_LclIp = oLclIp;
	m_strLclIp = m_LclIp.toString();
	if( isConnected() )
	{
		LogMsg( LOG_ERROR, "VxSktBase::connectTo: skt %d connect attempt on already connected socket", m_SktNumber );
		vx_assert( false );
		return -1;
	}

	m_bIsConnected = false;
	// kill previous thread if any
	m_SktRxThread.killThread();

	LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::connectTo: skt %d ip %s port %d", this->m_SktNumber, pIpUrlOrIp, u16Port );
	//resolve url
	std::string strUrlFile;
	uint16_t u16ReturnedPort;
	bool bResolved = VxResolveHostToIp(	pIpUrlOrIp,				//web name to resolve
										m_strRmtIp,
										u16ReturnedPort );		
	if( false == bResolved )
	{
		m_rcLastSktError = -1;
		m_eSktCallbackReason = eSktCallbackReasonConnectError;
        LogModule( eLogConnect, LOG_INFO, "VxSktBase::connectTo: skt %d could not resolve url %s", m_SktNumber, pIpUrlOrIp );
		// cannot do callback except in thread because may cause mutex deadlock
		//m_pfnReceive( this );
		return getLastSktError();
	}

	m_RmtIp.setIp( m_strRmtIp.c_str() );
	m_RmtIp.setPort( u16Port );
	m_eSktCallbackReason	= eSktCallbackReasonConnecting;
	m_iConnectTimeout		= iTimeoutMilliSeconds;

	int32_t rc = doConnectTo();
	if( rc )
	{
		//LogMsg( LOG_INFO, "doConnectTo returned error %d", rc );
		return rc;
	}

	// make a useful thread name
	std::string strVxThreadName;
	StdStringFormat( strVxThreadName, "VxSktBaseTCPa_%d", m_SktNumber );
	startReceiveThread( strVxThreadName.c_str() );
    LogModule( eLogConnect, LOG_VERBOSE,  "VxSktBase::%s skt num %d connected to %s:%d", __func__, m_SktNumber, pIpUrlOrIp, u16Port );

	return 0;
}

//============================================================================
void VxSktBase::createConnectionUsingSocket( SOCKET skt, const char* rmtIp, uint16_t port )
{
	m_bIsConnected = false;
	m_SktRxThread.killThread();

	m_RmtIp.setIp( rmtIp );
	m_strRmtIp = rmtIp;
	m_eSktCallbackReason	= eSktCallbackReasonConnecting;
	m_iConnectTimeout		= 3000;
	m_bIsConnected			= true;
	m_Socket				= skt;

	std::string strVxThreadName;
	StdStringFormat( strVxThreadName, "VxSktBaseTCPb_%d", m_SktNumber );
	startReceiveThread( strVxThreadName.c_str() );
    LogModule( eLogConnect, LOG_VERBOSE,  "createConnectionUsingSocket id %d connected to %s:%d", m_SktNumber, rmtIp, port );
}

//============================================================================
//! Do connect to from thread
int32_t VxSktBase::doConnectTo( void )
{
	uint16_t u16Port = m_RmtIp.getPort();

	m_Socket = ::VxConnectTo( m_LclIp, m_RmtIp, u16Port, m_iConnectTimeout );

	if( INVALID_SOCKET != m_Socket )
	{
		if( m_rcLastSktError )
		{
			//LogMsg( LOG_INFO, "VxSktBase::doConnectTo: skt %d handle %d connect to %s get remote ip error %s",
			//	m_SktNumber,
			//	m_Socket,
			//	m_strRmtIp.c_str(),
			//	VxDescribeSktError( m_rcLastError ) );
			// we connected.. dont error out just cause getpeername failed
			m_rcLastSktError = 0;
		}

        m_strLclIp = m_LclIp.toString();
        m_strRmtIp = m_RmtIp.toString();

        LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::doConnectTo: SUCCESS reason %s desc %s", DescribeConnectReason( getConnectReason() ), describeSktConnection().c_str() );
		m_bIsConnected = true;
		if( VxGetSktStatCallback() )
		{
			VxGetSktStatCallback()->sktConnected4( m_Socket, m_strRmtIp, getSktType(), getConnectReason() );
		}

		return 0;
	}
	else
	{
		m_bIsConnected = false;
        LogModule( eLogConnect, LOG_ERROR, "VxSktBase::doConnectTo: FAILED INVALID_SKT skt %d connect to %s port %d",
			m_SktNumber,
			m_strRmtIp.c_str(),
			m_RmtIp.getPort() );
		return -1;
	}
}

//============================================================================
std::string	 VxSktBase::describeSktConnection( void )
{
    std::string sktDesc;
    StdStringFormat( sktDesc, "%s num %d handle %d %s:%d%s%s:%d skt id %s", DescribeSktType( getSktType() ), getSktNumber(), m_Socket,
					 m_strLclIp.c_str(), m_LclIp.getPort(), 
					 describeSktDirection().c_str(), m_strRmtIp.c_str(), m_RmtIp.getPort(), getSocketIdText().c_str() );
    return sktDesc;
}

//============================================================================
void VxSktBase::closeSkt( ESktCloseReason closeReason, bool sktMgrLocked )
{
	if( INVALID_SOCKET == m_Socket || getIsInEraseList() )
	{
		return;
	}

	if( !m_bClosingFromRxThread )
	{
		m_bClosingFromRxThread = VxGetCurrentThreadId() == m_SktRxThread.getThreadId();
	}
	
    if( !isTempConnection() )
    {
        if(LogEnabled(eLogConnect))LogModule( eLogConnect, LOG_VERBOSE, "--%s skt num %d handle %d id %s reason %s last err %d description %s", __func__, getSktNumber(),
                  getSktHandle(), getSocketIdText().c_str(), DescribeSktCloseReason( closeReason ), getLastSktError(),  describeSktConnection().c_str() );
    }

	if( !m_HasBeenShutdown )
	{
		if( m_SktCloseReason == eSktCloseReasonUnknown )
		{
			m_SktCloseReason = closeReason;
		}

		setCallbackReason( eSktCallbackReasonClosing );
		if( m_pfnReceive && getThisSkt() )
		{
			m_pfnReceive( getThisSkt(), getRxCallbackUserData());
		}

		if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "%s %s %s", __func__, DescribeSktCloseReason( closeReason ), describeSktConnection().c_str() );

		if( m_bClosingFromRxThread || m_bClosingFromDestructor )
		{
			m_SktRxThread.abortThreadRun( true );

			m_bIsConnected = false;
			if( INVALID_SOCKET != m_Socket )
			{
				if( ( 0 != getLastActiveTimeMs() )
					&& isRxCryptoKeySet()
					&& ( 0 != getLastSktError() ) )
				{
					if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::closeSkt: reason %s %s thread %d err %d %s", 
						DescribeSktCloseReason( closeReason ), describeSktType().c_str(), VxGetCurrentThreadId(), getLastSktError(), describeSktError( getLastSktError() ) );
				}

                doCloseThisSocketHandle();

				//LogMsg( LOG_INFO, "VxSktBase::closeSkt: Skt %d handle %d close done", m_SktNumber, oSocket );
			}

			// if thread tries to suicide then problems occurs because thread cannot
			// exit while attempting to kill itself
			if( (false == m_bClosingFromRxThread ) &&
				( m_SktRxThread.getThreadTid() != VxGetCurrentThreadId() ) )
			{
				//LogMsg( LOG_INFO, "VxSktBase::closeSkt: Skt %d killing thread tid %d", m_SktNumber, m_SktRxThread.getThreadTid() );
				m_SktRxThread.killThread();
			}
		}
		else
		{
			if( ( INVALID_SOCKET != m_Socket ) 
				&& ( 0 != getLastActiveTimeMs() )
				&& isRxCryptoKeySet()
				&& ( 0 != getLastSktError() ) )
			{
				if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::closeSkt: reason %s %s err %d %s", 
					DescribeSktCloseReason( closeReason ), describeSktType().c_str(), getLastSktError(), describeSktError( getLastSktError() ) );
			}

            doCloseThisSocketHandle();
		}
	}

    m_SktRxThread.abortThreadRun( true );
    if( m_SktMgr && ! getIsInEraseList() )
    {
        m_SktMgr->sktWasClosed( this, sktMgrLocked );
    }
}

//============================================================================
void VxSktBase::doCloseThisSocketHandle( void )
{
	// NOTE: if VxFlushThenCloseSkt is called instead of VxCloseSktNow then thread will not be released if waiting on
	// blocking socket operation
	if( INVALID_SOCKET != m_Socket )
	{	
		SOCKET oSocket = m_Socket; 
		m_Socket = INVALID_SOCKET;
		if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "--VxSktBase::%s skt %s num %d handle %d id %s peer %s", __func__,
				describeSktType().c_str(), getSktNumber(), getSktHandle(), getSocketIdText().c_str(), getPeerPktAnn().describeUser().c_str() );
		VxCloseSktNow( oSocket );
	}
}

//============================================================================
//! send data without encrypting
int32_t VxSktBase::sendData(const char* pData, int iDataLen, bool sktMgrLocked )	// if true disconnect after data is sent
{
	if( false == isConnected() )
	{
        LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::sendData: Attempted send on disconnected skt %s", this->describeSktType().c_str() );
		return -1;
	}

    if(LogEnabled( eLogSktTx ))
    {
        LogModule( eLogSktTx, LOG_INFO, "VxSktBase::%s: skt num %d tx %d bytes to %s", __func__, 
            getSktNumber(), iDataLen, m_RmtIp.toString().c_str() );
    }

	if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "skt %d sendData length %d to %s:%d", m_SktNumber, iDataLen, m_strRmtIp.c_str(), m_RmtIp.getPort() );
	if( INVALID_SOCKET != m_Socket )
	{
		int iSentLen;
		int tryCnt{ 0 };
		while( true )
		{
			tryCnt++;
			if( INVALID_SOCKET == m_Socket )
			{
				// socket was closed while sending
				if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "skt %d was closed while sending %d bytes to %s:%d", m_SktNumber, iDataLen, m_strRmtIp.c_str(), m_RmtIp.getPort() );
				#if defined(TARGET_OS_WINDOWS)
					int32_t sktClosedErr = WSAECONNRESET;
				#else
					int32_t sktClosedErr = ECONNRESET;
				#endif

				setLastSktError(sktClosedErr );
				return sktClosedErr;
			}

			// nothing else seems to work to fix the broken pipe SIGPIPE exception
			// so for linux only set MSG_NOSIGNAL on a per call bases so returns an error instead of throwing a exception
			#if defined(TARGET_OS_WINDOWS)
				iSentLen = send( m_Socket, (const char*)pData, iDataLen, 0);
			#else
				iSentLen = send( m_Socket, (const char*)pData, iDataLen, MSG_NOSIGNAL);
			#endif // defined(TARGET_OS_WINDOWS)

			if( 0 > iSentLen )
			{
				setLastSktError( VxGetLastError() );
				if( 0 == m_rcLastSktError )
				{
					m_rcLastSktError = iSentLen;
				}

				if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::sendData: Skt %d Handle %d Error %s",
									m_SktNumber, 
									m_Socket, 
									VxDescribeSktError( m_rcLastSktError ) );

				return getLastSktError();
			}

			if( iSentLen > 0 )
			{
				tryCnt = 0;
			}

			pData = pData + iSentLen;
			iDataLen -= iSentLen;
			TxedPkt( iSentLen );
			m_iLastTxLen = iSentLen;
			if( m_pfnTransmit )
			{
				m_pfnTransmit( getThisSkt(), getTxCallbackUserData());
			}

			if( 0 >= iDataLen )
			{
				// all done
				return 0;
			}

			if( VxIsFatalSktError( getLastSktError() ) )
			{
				closeSkt( eSktCloseSktWithError, sktMgrLocked );
				return -1;
			}

			// sleep and try again
			VxSleep( 30 );
			
			if( tryCnt > 100 )
			{
				LogMsg( LOG_ERROR, "Tried send %d times skt %d sendData length %d to %s:%d", tryCnt, m_SktNumber, iDataLen, m_strRmtIp.c_str(), m_RmtIp.getPort() );
			}
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "INVALID SKT skt %d sendData length %d to %s:%d", m_SktNumber, iDataLen, m_strRmtIp.c_str(), m_RmtIp.getPort() );
	}

	return -1;
}

//============================================================================
//! encrypt then send data using session crypto
int32_t VxSktBase::txEncrypted( const char* pDataIn, int iDataLen, bool sktMgrLocked )	
{
    if( !pDataIn )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted null data");
        closeSkt( eSktCloseCryptoNullData, sktMgrLocked );

        vx_assert( pDataIn );
        return -1;
    }

    if( !iDataLen || ( 0 != ( iDataLen & 0x0f ) ) )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted invalid data len %d", iDataLen);
        closeSkt( eSktCloseCryptoInvalidLength, sktMgrLocked );

        vx_assert( false );
        return -2;
    }

	if( !isConnected() )
	{
		LogMsg( LOG_VERBOSE, "VxSktBase::txEncrypted skt is already disconnected" );
		return -5;
	}

	VxPktHdr* pktHdr = ( (VxPktHdr*)pDataIn );
	if( !pktHdr->isValidPktHdr() )
	{
		LogMsg( LOG_ERROR, "VxSktBase::txEncrypted invalid pkt len %d (pkt type %d)", pktHdr->getPktLength(), pktHdr->getPktType() );
		closeSkt( eSktClosePktLengthInvalid, sktMgrLocked );

		vx_assert( false );
		return -3;
	}

    if( !m_TxCrypto.isKeyValid() )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted invalid crypto key");
        vx_assert( m_TxCrypto.isKeyValid() );

        closeSkt( eSktCloseCryptoInvalidKey, sktMgrLocked );

        return -4;
    }

	// make copy of data so data is not destroyed
	unsigned char * pu8Data = new unsigned char[ iDataLen ];
	memcpy( pu8Data, pDataIn, iDataLen );

    // protect in case we are sending from thread other than rx thread
	#if defined(DEBUG_SKT_TX_LOCK)
		int lockStartMs = GetApplicationAliveMs();
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.lock", __func__ );
    #endif // defined(DEBUG_SKT_TX_LOCK)
    m_TxMutex.lock();
    #if defined(DEBUG_SKT_TX_LOCK)
		int lockGainedMs = GetApplicationAliveMs();
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.lock done %d ms", __func__, lockGainedMs - lockStartMs );
    #endif // defined(DEBUG_SKT_TX_LOCK)
    
	// encrypt
	int32_t rc =	m_TxCrypto.encrypt( pu8Data, iDataLen );
	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSktBase::txEncrypted: crypto error %d", rc );
		vx_assert( 0 == rc );
	}
	else
	{
		// send
		rc = this->sendData( (char *)pu8Data, iDataLen, sktMgrLocked );
	}

    setLastSktError( rc );
	#if defined(DEBUG_SKT_TX_LOCK)
		int unlockAfterMs = GetApplicationAliveMs();
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.unlock after %d ms", __func__, unlockAfterMs - lockGainedMs );
    #endif // defined(DEBUG_SKT_TX_LOCK)
    m_TxMutex.unlock();
	delete[] pu8Data;

    if( rc && isFatalSocketError( rc ) )
    {
        LogModule( eLogSktData, LOG_ERROR, "VxSktBase::txEncrypted: sendData error %d %s", rc, VxDescribeSktError( rc ) );
		closeSkt( eSktCloseTxFailed, sktMgrLocked );
    }

	return rc;
}

//============================================================================
//! encrypt with given key then send.. does not affect session crypto
int32_t VxSktBase::txEncrypted( VxKey* poKey,	const char*	pDataIn, int iDataLen, bool sktMgrLocked )		// length of data
{
    if( !pDataIn )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted2 null data");
        closeSkt( eSktCloseCryptoNullData, sktMgrLocked );

        vx_assert( pDataIn );
        return -1;
    }

    if( !iDataLen )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted2 invalid data len %d", iDataLen);
        closeSkt( eSktCloseCryptoInvalidLength, sktMgrLocked );

        vx_assert( pDataIn );
        return -2;
    }

    if( 0 != (iDataLen & 0x0f) )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted2 invalid pkt len %d (pkt type %d)", iDataLen, ((VxPktHdr*)pDataIn)->getPktType() );
        closeSkt( eSktClosePktLengthInvalid, sktMgrLocked );

        vx_assert( 0 == (iDataLen & 0x0f) );
        return -3;
    }

    if( !poKey || !poKey->isKeySet() )
    {
        LogMsg( LOG_ERROR, "VxSktBase::txEncrypted2 invalid crypto key");
        vx_assert( m_TxCrypto.isKeyValid() );

        closeSkt( eSktCloseCryptoInvalidKey, sktMgrLocked );

        return -4;
    }

	// make copy of data so data is not destroyed
	char * pData = new char[ iDataLen ];
	memcpy( pData, pDataIn, iDataLen );

    // protect in case we are sending from thread other than rx thread
	#if defined(DEBUG_SKT_TX_LOCK)
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.lock", __func__ );
    #endif // defined(DEBUG_SKT_TX_LOCK)
    m_TxMutex.lock();
    #if defined(DEBUG_SKT_TX_LOCK)
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.lock done", __func__ );
    #endif // defined(DEBUG_SKT_TX_LOCK)
	// encrypt
	VxSymEncrypt( poKey, (char *)pData, iDataLen );
	// send
	int32_t rc = this->sendData( (char *)pData, iDataLen, sktMgrLocked );
	#if defined(DEBUG_SKT_TX_LOCK)
        LogMsg( LOG_DEBUG, "VxSktBase::%s m_TxMutex.unlock", __func__ );
    #endif // defined(DEBUG_SKT_TX_LOCK)
    m_TxMutex.unlock();
	
	delete[] pData;

	if( rc && isFatalSocketError( rc ) )
	{
		LogMsg( LOG_ERROR, "VxSktBase::txEncrypted: error %d", rc );
		closeSkt( eSktCloseTxFailed, sktMgrLocked );
	}

	return rc;
}

//============================================================================
int32_t VxSktBase::txPacket(	VxGUID destOnlineId, VxPktHdr* pktHdr, bool sktMgrLocked )		// packet to send
{
	pktHdr->setDestOnlineId( destOnlineId );
	return txPacketWithDestId( pktHdr, sktMgrLocked );
}

//============================================================================
int32_t VxSktBase::txPacketWithDestId( VxPktHdr* pktHdr, bool sktMgrLocked ) 		// packet to send
{
    if( !isConnected() )
    {
        LogMsg( LOG_ERROR, "%s no longer connected to %s", __func__, describePeerUser().c_str() );
        return -1;
    }

	m_u8TxSeqNum = (uint8_t)rand();
	pktHdr->setPktSeqNum( m_u8TxSeqNum );
	vx_assert( pktHdr->getDestOnlineId().isValid() );
	uint64_t timestamp = GetGmtTimeMs();
	uint16_t pktType = pktHdr->getPktType();


	if( !pktHdr->getSrcOnlineId().isValid() || !pktHdr->getDestOnlineId().isValid() )
	{
		LogMsg( LOG_ERROR, "ERROR VxPeerMgr::txPacketWithDestId: invalid src or dest id pkt %s src id %s dest id %s",
				pktHdr->describePktHdr().c_str(), pktHdr->getSrcOnlineId().toOnlineIdString().c_str(),
				pktHdr->getDestOnlineId().toOnlineIdString().c_str() );

        return -4;
	}

	setLastActiveTimeMs( timestamp );
	if( PKT_TYPE_IM_ALIVE_REQ != pktType && PKT_TYPE_IM_ALIVE_REPLY != pktType && PKT_TYPE_PING_REQ != pktType && PKT_TYPE_PING_REPLY != pktType )
	{
		setLastSessionTimeMs( timestamp );
	}

	if( PKT_TYPE_ANNOUNCE == pktType )
	{
		PktAnnounce* pktAnn = (PktAnnounce*)pktHdr;
		if( getIsPeerPktAnnSet() )
		{
			bool isAnnToPeer = pktHdr->getDestOnlineId() != getPeerOnlineId();
			if( isAnnToPeer )
			{
				if(LogEnabled(eLogConnect))LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::txPacketWithDestId tx PktAnn %s to %s through relay %s %s",
						DescribeFriendState( pktAnn->getMyFriendshipToHim() ),
						pktHdr->getDestOnlineId().toOnlineIdString().c_str(),
						getPeerOnlineName().c_str(),
						getPeerOnlineId().toOnlineIdString().c_str() );
			}
			else
			{
				if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::txPacketWithDestId tx PktAnn %s to %s %s",
						DescribeFriendState( pktAnn->getMyFriendshipToHim() ),
						getPeerOnlineName().c_str(),
						getPeerOnlineId().toOnlineIdString().c_str() );
			}
		}
		else
		{
			if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::txPacketWithDestId tx PktAnn %s to %s",
					DescribeFriendState( pktAnn->getMyFriendshipToHim() ),
					pktHdr->getDestOnlineId().toOnlineIdString().c_str() );
		}
	}

    // filter out im alive packets to declutter long
    if( pktHdr->getPktType() != PKT_TYPE_IM_ALIVE_REPLY && pktHdr->getPktType() != PKT_TYPE_IM_ALIVE_REQ )
    {
        if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_VERBOSE, "skt num %d id %s send pkt %s to %s:%d src id %s dest id %s peer id %s", getSktNumber(),
                getSocketIdText().c_str(), pktHdr->describePktHdr().c_str(), m_strRmtIp.c_str(), m_RmtIp.getPort(),
                pktHdr->getSrcOnlineId().toOnlineIdString().c_str(), pktHdr->getDestOnlineId().toOnlineIdString().c_str(),
				getPeerOnlineId().isValid() ? getPeerOnlineId().toOnlineIdString().c_str() : "0" );
    }

	if( getPeerOnlineId().isValid() && getPeerOnlineId() != pktHdr->getDestOnlineId() )
	{
		if(LogEnabled(eLogPkt)) LogModule( eLogPkt, LOG_WARN, "pkt %s will be relayed if possible", pktHdr->describePktHdr().c_str() );
	}

	return txEncrypted( (const char*)pktHdr, pktHdr->getPktLength(), sktMgrLocked );
}

//============================================================================
//! decrypt as much as possible in receive buffer
int32_t VxSktBase::decryptReceiveData( void )
{
	if( false == isRxEncryptionKeySet() )
	{
		// no key to decrypt with
        LogMsg( LOG_ERROR, "VxSktBase::%s No Rx Crypto Key Set %s", __func__, describeSktType().c_str() );
		return -1;
	}

	lockCryptoAccess();
    uint32_t u32Datalen = getSktBufDataLen();
	// truncate to 16 byte boundary
	u32Datalen = u32Datalen & 0xfffffff0;
	if( u32Datalen )
	{
		vx_assert( u32Datalen >= m_u32RxDecryptedLen );
		int32_t u32LenToDecrypt = u32Datalen - m_u32RxDecryptedLen;
		if( u32LenToDecrypt )
		{
			m_RxCrypto.decrypt( &m_pau8SktBuf[ m_u32RxDecryptedLen ], u32LenToDecrypt );
			m_u32RxDecryptedLen += u32LenToDecrypt;
		}
	}

	unlockCryptoAccess();
	return 0;
}

//============================================================================
void VxSktBase::setConnectReason( EConnectReason connectReason ) 
{ 
    m_ConnectReason = connectReason;
    if( IsConnectReasonTemporary( connectReason ) )
    {
        setIsTempConnection( true );
    }

	if( isConnected() && VxGetSktStatCallback() )
	{
		VxGetSktStatCallback()->sktConnected4( m_Socket, m_strRmtIp, getSktType(), getConnectReason() );
	}
}

//============================================================================
//! return true if is connected
bool VxSktBase::isConnected( void )
{
	if( INVALID_SOCKET == m_Socket )
	{
		m_bIsConnected = false;
	}

	return m_bIsConnected;
}

//============================================================================
//! get the sockets peer connection ip address as net order int32_t
int32_t VxSktBase::getRemoteIp(	InetAddress &u32RetIp,		// return ip
								uint16_t &u16RetPort )	// return port
{
	u32RetIp = m_RmtIp;
	u16RetPort = m_RmtIp.getPort();
	return 0;
}

//============================================================================
std::string VxSktBase::getRemoteIp( void )
{
	return m_strRmtIp.c_str();
}

//============================================================================
std::string VxSktBase::getLocalIp( void )
{
	return m_strLclIp.c_str();
}

//============================================================================
void VxSktBase::setRmtAddress( struct sockaddr_storage& oSktAddr )
{
	m_RmtIp.setIpAndPort( oSktAddr );
	m_strRmtIp = m_RmtIp.toString();
}

//============================================================================
void VxSktBase::setRmtAddress( struct sockaddr& oSktAddr )
{
	m_RmtIp.setIpAndPort( oSktAddr );
	m_strRmtIp = m_RmtIp.toString();
}

//============================================================================
void VxSktBase::setRmtAddress( struct sockaddr_in& oSktAddrIn )
{
	struct sockaddr_storage oSktAddr;
	memset( &oSktAddr, 0, sizeof( oSktAddr ) );
	memcpy( &oSktAddr, &oSktAddrIn, sizeof( oSktAddrIn ) );

	setRmtAddress( oSktAddr );
}

//============================================================================
void VxSktBase::setLclAddress( struct sockaddr_storage& oSktAddr )
{
	m_LclIp.setIpAndPort( oSktAddr );
	m_strLclIp = m_LclIp.toString();
}

//============================================================================
void VxSktBase::startReceiveThread( const char* pVxThreadName )
{
	m_SktRxThread.killThread();
    setInUseByRxThread( true );
	m_SktRxThread.startThread( (VX_THREAD_FUNCTION_T)VxSktBaseReceiveVxThreadFunc, this, pVxThreadName );
}

//============================================================================
void VxSktBase::setLastSktError( int32_t rc )						
{ 
	m_rcLastSktError = rc; 
	if( ( 0 != rc ) 
		&& ( -1 != rc ) 
		&& ( 11 != rc ) )
	{
		if( !isUdpSocket() && ( 0 != getLastActiveTimeMs() ) )
		{
			LogMsg( LOG_INFO, "VxSktBase::setLastSktError: %s %d %s", describeSktType().c_str(), m_rcLastSktError, describeSktError( m_rcLastSktError ) );
		}
	}
}

//============================================================================
void * VxSktBaseReceiveVxThreadFunc( void * pvContext )
{
#if !defined(TARGET_OS_WINDOWS)
	signal(SIGPIPE, SIG_IGN);
#endif // !defined(TARGET_OS_WINDOWS)

	VxThread* poVxThread = (VxThread*)pvContext;
	VxSktBase* sktBase{ nullptr };
    if( !poVxThread->isAborted() )
    {
        poVxThread->setIsThreadRunning( true );
        sktBase = (VxSktBase*)poVxThread->getThreadUserParam();
        if( sktBase )
        {
			sktBase->setRxStartTimeMs( GetGmtTimeMs() );
			sktBase->incrementRunningRxSktThreadCnt();
            LogModule( eLogThread, LOG_VERBOSE, "%s rx thread 0x%x started for skt %d skt id %d ", __func__, VxGetCurrentThreadId(), sktBase->getSktHandle(), sktBase->getSktNumber() );

            char as8Buf[ 0x8000 ];
            int iDataLen = 0;
            //int iBufferAlmostFull = sktBase->getSktBufSize() - sktBase->getSktBufSize() / 10;
            struct sockaddr_storage oAddr;
            bool bIsUdpSkt = true;

            if( eSktTypeTcpConnect == sktBase->getSktType() ||
                eSktTypeTcpAccept == sktBase->getSktType() )
            {
                bIsUdpSkt = false;
                sktBase->setLastImAliveTimeRxMs( GetGmtTimeMs() ); // so we don't get closed if takes awhile for everything to get going
                if( eSktTypeTcpConnect == sktBase->getSktType()  )
                {
                    // we couldn't do callbacks in connect function ( mutex issues ) so
                    // do the callback now
                    sktBase->setCallbackReason( eSktCallbackReasonConnecting );
                    sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData());
                }
            }

            if(	( poVxThread->isAborted() ) ||
                ( INVALID_SOCKET == sktBase->m_Socket ) )
            {
                // something has already happened to the connection
                //! VxThread calls this just before exit
                poVxThread->threadAboutToExit();
                sktBase->setInUseByRxThread( false );
                return nullptr;
            }

            sktBase->setLastSktError( 0 );
            sktBase->setIsConnected( true );
            sktBase->setCallbackReason( eSktCallbackReasonConnected );
            sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData() );
            sktBase->setCallbackReason( eSktCallbackReasonData );

            //LogMsg( LOG_SKT, "VxSktBaseReceiveVxThreadFunc: set blocking true\n" );
            sktBase->setSktBlocking( true );

            while(	( false == poVxThread->isAborted() ) &&
                    ( INVALID_SOCKET != sktBase->m_Socket ) &&
                    ( eSktCallbackReasonData == sktBase->getCallbackReason() ) )
            {
                if( !sktBase->isConnected() )
                {
                    if( !sktBase->isUdpSocket() && ( 0 != sktBase->getLastActiveTimeMs() ) )
                    {
                        LogModule( eLogConnect, LOG_VERBOSE, "%s: %s skt %d skt id %d no longer connected thread 0x%x",
                                   __func__, sktBase->describeSktType().c_str(), sktBase->getSktHandle(), sktBase->getSktNumber(), VxGetCurrentThreadId() );
                    }

                    break;
                }

                int iAttemptLen = sktBase->getSktBufFreeSpace();
                if( iAttemptLen <= 0 )
                {
                    LogMsg( LOG_VERBOSE, "skt %d handle %d buffer is full ", sktBase->getSktNumber(), sktBase->getSktHandle() );
                }

                vx_assert( iAttemptLen >= 0 );
                if( iAttemptLen >= (int)sizeof( as8Buf ) )
                {
                    iAttemptLen = (int)sizeof( as8Buf ) - 16;
                }

                if( bIsUdpSkt )
                {
					if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "udp wait for rx attempt len %d on skt %d skt id %d lcl ip %s thread 0x%x",
						iAttemptLen, sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->m_strLclIp.c_str(), VxGetCurrentThreadId() );

					if( INVALID_SOCKET == sktBase->m_Socket )
					{
						LogModule( eLogSktData, LOG_VERBOSE, "udp closing with invalid socket" );
						// has been closed by outside thread
						sktBase->setCallbackReason( eSktCallbackReasonClosing );
						goto closed_skt_exit;
					}

					memset( &oAddr, 0, sizeof( struct sockaddr_storage ) );
					socklen_t iSktAddrLen = sizeof( struct sockaddr_storage );
					if( sktBase->m_LclIp.isIPv4() )
					{
						struct sockaddr_in addr;
						memset( &addr, 0, sizeof( addr ) );
						addr.sin_family = AF_INET;
						addr.sin_addr.s_addr = INADDR_ANY; // differs from sender
						addr.sin_port = htons( sktBase->getLocalPort() );

						iSktAddrLen = sizeof( addr );

						iDataLen = recvfrom( sktBase->m_Socket,	// socket
							as8Buf,				// buffer to read into
							iAttemptLen,		// length of buffer space
							0,					// flags
							( struct sockaddr* )&addr, // source address
							&iSktAddrLen );		// size of address structure
					}
					else
					{
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_INFO, "udp recvfrom IPV6" );
						iSktAddrLen = sizeof( struct sockaddr_in6 );
						( ( struct sockaddr_in6* )&oAddr )->sin6_family = AF_INET6;
						( ( struct sockaddr_in6* )&oAddr )->sin6_addr  = in6addr_any;
						iDataLen = recvfrom( sktBase->m_Socket,	// socket
							as8Buf,				// buffer to read into
							iAttemptLen,		// length of buffer space
							0,					// flags
							( struct sockaddr* )&oAddr, // source address
							&iSktAddrLen );		// size of address structure
					}

					if( poVxThread->isAborted() || !sktBase->isConnected() )
					{
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "udp recvfrom skt handle %d skt id %d close from thread",
								   sktBase->getSktHandle(), sktBase->getSktNumber() );
						goto closed_skt_exit;
					}

					if( iDataLen < 0 )
					{
						int32_t rc = VxGetLastError();
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "udp recvfrom skt handle %d skt id %d port %d error %d",
							sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->m_RmtIp.getPort(), rc );
						VxSleep( 500 );
						continue;
					}
                    else if( iDataLen > 0 )
                    {
						sktBase->m_RmtIp.m_u16Port = sktBase->m_RmtIp.setIp( oAddr );
						sktBase->m_strRmtIp = sktBase->m_RmtIp.toString();

						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "udp recvfrom skt handle %d skt id %d len %d port %d thread 0x%x",
							sktBase->getSktHandle(), sktBase->getSktNumber(), iDataLen, sktBase->m_RmtIp.getPort(), VxGetCurrentThreadId() );
						if( sktBase->m_RxKey.isKeySet() )
						{
							if( false == sktBase->m_RxKey.isValidDataLen( iDataLen ) )
							{
								// throw away the data because not valid length
								LogMsg( LOG_INFO, "udp recvfrom not valid data length" );
								continue;
							}
							// set encryption context
							sktBase->m_RxCrypto.importKey( &sktBase->m_RxKey );
						}
                    }
                    else
                    {
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_INFO, "%s: udp recvfrom skt handle %d skt id %d len %d thread 0x%x", __func__, sktBase->getSktHandle(), sktBase->getSktNumber(), iDataLen, VxGetCurrentThreadId() );
                    }
                }
                else
                {
                    // TCP data
                    //LogMsg( LOG_SKT, "VxSktBaseReceiveVxThreadFunc: recv skt handle %d attempt len %d", sktBase->m_Socket, iAttemptLen );

                    iDataLen = recv(		sktBase->m_Socket,	// socket
                                            as8Buf,				// buffer to read into
                                            iAttemptLen,		// length of buffer space
                                            0 );				// flags
                    if( INVALID_SOCKET == sktBase->m_Socket )
                    {
                        // has been closed by outside thread
                        sktBase->setCallbackReason( eSktCallbackReasonClosing );
                        goto closed_skt_exit;
                    }

					if( iDataLen < 0 && VxIsFatalSktError( VxGetLastError() ) )
					{
						LogMsg( LOG_ERROR, "%s fatal socket error %d %s", __func__, VxGetLastError(), VxDescribeSktError( VxGetLastError() ) );
						sktBase->setLastSktError( VxGetLastError() );
						sktBase->setCallbackReason( eSktCallbackReasonClosing );
                        goto closed_skt_exit;
					}

					if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "%s: tcp recv skt handle %d skt id %d rxed len %d attempt len %d thread 0x%x",
                               __func__, sktBase->getSktHandle(), sktBase->getSktNumber(), iDataLen, iAttemptLen, VxGetCurrentThreadId() );
                }

                if( poVxThread->isAborted()
                    || ( eSktCallbackReasonData != sktBase->getCallbackReason() )
                    || ( INVALID_SOCKET == sktBase->m_Socket )
                    || ( iDataLen <= 0 ) )
                {
                    if( poVxThread->isAborted() || ( INVALID_SOCKET == sktBase->m_Socket ) )
                    {
                        // normal close or shutdown
                        //if( !sktBase->isUdpSocket() && ( 0 != sktBase->getLastActiveTime() ) )
                        //{
                        //	LogMsg( LOG_SKT, "VxSktBaseReceiveVxThreadFunc: skt %d 0x%x closed or aborted", sktBase->m_Socket, sktBase );
                        //}

                        sktBase->setLastSktError( 0 );
                        break;
                    }

                    // socket error occurred
                    sktBase->setLastSktError(  VxGetLastError() );
                    if( 0 == sktBase->getLastSktError() )
                    {
                        sktBase->setLastSktError( -1 );
                    }

        #ifdef TARGET_OS_WINDOWS
                    if ((iDataLen < 0)
                        && ((11 == sktBase->getLastSktError()) || (WSAEWOULDBLOCK == sktBase->getLastSktError())))
        #else
                    if ((iDataLen < 0)
                        && ((11 == sktBase->getLastSktError()) || (EINPROGRESS == sktBase->getLastSktError())))
        #endif // TARGET_OS_WINDOWS
                    {
                        // try again
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "%s: skt handle %d skt Id %d ip %s trying again thread 0x%x",
                                   __func__, sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->m_strLclIp.c_str(), VxGetCurrentThreadId() );

                        VxSleep( SKT_RX_RETRY_SLEEP_TIME_MS );
                        continue;
                    }

                    break;
                }

                sktBase->setLastSktError( 0 );
                if( iDataLen > 0 )
                {
                    if(LogEnabled( eLogSktRx ))
                    {
                        LogModule( eLogSktRx, LOG_INFO, "VxSktBase::%s: skt num %d rx %d bytes from %s", __func__, 
                            sktBase->getSktNumber(), iDataLen, sktBase->m_RmtIp.toString().c_str() );
                    }

                    sktBase->m_iLastRxLen = iDataLen;

                    memcpy( sktBase->getSktWriteBuf(), as8Buf, iDataLen );
                    sktBase->sktBufAmountWrote( iDataLen );

                    // decrypt as much as possible
                    sktBase->decryptReceiveData();

                    sktBase->RxedPkt( iDataLen );
                    // call back user with the good news.. we have data
					if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "%s: skt handle %d skt Id %d receiving len %d thread 0x%x",
                               __func__, sktBase->getSktHandle(), sktBase->getSktNumber(), iDataLen, VxGetCurrentThreadId() );
                    sktBase->updateLastActiveTime();
                    sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData() );
                }
            }

        closed_skt_exit:
            sktBase->setIsConnected( false );
			if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "--VxSktBase::%s skt handle %d skt num %d close user %s tmp %d", __func__,
				sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->describePeerUser().c_str(), sktBase->isTempConnection() );

            if( eSktCallbackReasonNewMgr != sktBase->getCallbackReason() )
            {
                if( 0 != sktBase->getLastSktError() )
                {
                    if( !sktBase->isUdpSocket() && ( 0 != sktBase->getLastActiveTimeMs() ) )
                    {
						if( LogEnabled( eLogSktData ) )LogModule( eLogSktData, LOG_VERBOSE, "%s: %s skt handle %d skt Id %d exit with error %d %s thread 0x%x",
								   __func__,
                                    sktBase->describeSktType().c_str(),
                                    sktBase->getSktHandle(), sktBase->getSktNumber(),
                                    sktBase->getLastSktError(),
                                    sktBase->describeSktError( sktBase->getLastSktError() ),
                                    VxGetCurrentThreadId()
                                 );
                    }

                    // we had a error
                    sktBase->setCallbackReason( eSktCallbackReasonError );
                    sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData() );
                }

                if( false == poVxThread->isAborted() )
                {
                    // we are closing due to error .. not because user called close
                    sktBase->m_bClosingFromRxThread = true;
                }

                sktBase->setCallbackReason( eSktCallbackReasonClosing );
                sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData() );
                sktBase->setCallbackReason( eSktCallbackReasonClosed );
                sktBase->m_pfnReceive( sktBase->getThisSkt(), sktBase->getRxCallbackUserData() );
            }

            if( INVALID_SOCKET != sktBase->m_Socket )
            {
                poVxThread->abortThreadRun( true );
                sktBase->m_bClosingFromRxThread = true;
                if( sktBase->getLastSktError() )
                {
                    sktBase->closeSkt( eSktCloseSktWithError );
                }
                else
                {
                    sktBase->closeSkt( eSktCloseNotNeeded );
                }
            }

            if( sktBase->m_SktMgr )
            {
                sktBase->m_SktMgr->handleSktCloseEvent( sktBase->getThisSkt() );
            }
        }
    }

    poVxThread->abortThreadRun( true );
    if( sktBase )
    {
        sktBase->setInUseByRxThread( false );
		// just to make sure we are not exiting without closing
		sktBase->closeSkt( eSktCloseNotNeeded );
		sktBase->decrementRunningRxSktThreadCnt();
		LogModule( eLogThread, LOG_VERBOSE, "VxSktBase rx thread 0x%x exiting for skt %d skt id %d ", VxGetCurrentThreadId(), sktBase->getSktHandle(), sktBase->getSktNumber() );
    }

	poVxThread->threadAboutToExit();
    return nullptr;
}

//============================================================================
void VxSktBase::setTxCryptoPassword( const char* data, int len )
{
	m_TxKey.m_bIsSet = true;
	m_TxCrypto.setPassword( data, len );
}

//============================================================================
void VxSktBase::setRxCryptoPassword( const char* data, int len )
{
	m_RxKey.m_bIsSet = true;
	m_RxCrypto.setPassword( data, len );
}

//============================================================================
bool VxSktBase::isTxCryptoKeySet( void )
{
	return m_TxKey.m_bIsSet;
}

//============================================================================
bool VxSktBase::isRxCryptoKeySet( void )
{
	return m_RxKey.m_bIsSet;
}

//============================================================================
const char* VxSktBase::describeSktError( int32_t rc )
{
	return VxDescribeSktError( rc );
}

//============================================================================
const char* VxSktBase::describeSktCallbackReason( ESktCallbackReason reason )
{
    switch( reason )
    {
    case eSktCallbackReasonUnknown:
        return "SktCallbackReasonUnknown";
    case eSktCallbackReasonConnecting:
        return "eSktCallbackReasonConnecting";
    case eSktCallbackReasonConnectError:
        return "eSktCallbackReasonConnectError";
    case eSktCallbackReasonConnected:
        return "eSktCallbackReasonConnected";
    case eSktCallbackReasonData:
        return "eSktCallbackReasonData";
    case eSktCallbackReasonClosing:
        return "eSktCallbackReasonClosing";
    case eSktCallbackReasonClosed:
        return "eSktCallbackReasonClosed";
    case eSktCallbackReasonError:
        return "eSktCallbackReasonError";
    case eSktCallbackReasonNewMgr:
        return "eSktCallbackReasonNewMgr";
    default:
        return "SktCallbackReasonInvalid";
    }
}

//============================================================================
std::string VxSktBase::describeSktType( void )
{
	std::string sktTypeDesc = "unknown";
	if( isLoopbackSocket() )
	{
		sktTypeDesc = "loopback";
	}
	else if( isAcceptSocket() )
	{
		sktTypeDesc = "accept";
	}
	else if( isConnectSocket() )
	{
		sktTypeDesc = "connect";
	}
	else if( isUdpSocket() )
	{
		sktTypeDesc = "udp";
	}

    std::string sktDesc;
    StdStringFormat( sktDesc, "skt id %d handle %d type %s ip %s last active %s ",
                     m_SktNumber, 
                     m_Socket,
					 sktTypeDesc.c_str(),
					m_strRmtIp.c_str(),
					 ( 0 == getLastActiveTimeMs() ) ? "never" : VxTimeUtil::formatTimeStampIntoHoursAndMinutesAndSeconds( GmtTimeMsToLocalTimeMs( getLastActiveTimeMs() ) ).c_str() );
    return sktDesc;
}

//============================================================================
bool VxSktBase::setPeerPktAnn( PktAnnounce &peerAnn )
{
    m_PeerAnnMutex.lock();
    bool isSameSize = peerAnn.getPktLength() && peerAnn.getPktLength() == m_PeerPktAnn.getPktLength();
    if( isSameSize )
    {
        memcpy( &m_PeerPktAnn, &peerAnn, m_PeerPktAnn.getPktLength() );
        m_PeerOnlineId = m_PeerPktAnn.getMyOnlineId();
        setIsPeerPktAnnSet( isSameSize );
		setRxPktAnnTimeMs( GetGmtTimeMs() );
    }

    m_PeerAnnMutex.unlock();
	
	if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "VxSktBase::setPeerPktAnn: skt %s num %d handle %d id %s peer %s ip %s", this->describeSktType().c_str(),
				   getSktNumber(), getSktHandle(), getSocketIdText().c_str(), peerAnn.describeUser().c_str(), getRemoteIp().c_str() );
	if( !isSameSize )
	{
		closeSkt( eSktCloseWrongPktAnnSize );
	}

    return isSameSize;
}

//============================================================================
bool VxSktBase::getPeerPktAnnCopy( PktAnnounce& peerAnn )
{
    m_PeerAnnMutex.lock();
    bool copyResult = getIsPeerPktAnnSet() && peerAnn.getPktLength() && peerAnn.getPktLength() == m_PeerPktAnn.getPktLength();
    if( copyResult )
    {
        memcpy( &peerAnn, &m_PeerPktAnn, m_PeerPktAnn.getPktLength() );
    }

    m_PeerAnnMutex.unlock();
    return copyResult;
}

//============================================================================
std::string VxSktBase::getPeerOnlineName()
{
	if( getIsPeerPktAnnSet() )
	{
		return getPeerPktAnn().getOnlineName();
	}
	else
	{
		return "Peer Name Not Set";
	}
}

//============================================================================
void VxSktBase::dumpSocketStats( const char* reason, bool fullDump )
{
    std::string reasonMsg = reason ? reason : "";
	if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "Dump %s connected ? %d last active %s", describeSktConnection().c_str(), isConnected(),
        ( 0 == getLastActiveTimeMs() ) ? "never" : VxTimeUtil::formatTimeStampIntoHoursAndMinutesAndSeconds( GmtTimeMsToLocalTimeMs( getLastActiveTimeMs() ) ).c_str() );
}

//============================================================================
const std::string& VxSktBase::describeSktDirection( void )
{
    switch( m_eSktType )
    {
    case eSktTypeTcpConnect: return m_SktDirConnect;
    case eSktTypeTcpAccept: return m_SktDirAccept;
    case eSktTypeUdp: return m_SktDirUdp;
    case eSktTypeUdpBroadcast: return m_SktDirBroadcast;
    case eSktTypeLoopback: return m_SktDirLoopback;
    case eSktTypeNone: return m_SktDirUnknown;
    default: return m_SktDirUnknown;
    }
}

//============================================================================
bool VxSktBase::isFatalSocketError( int32_t rc )
{
	return VxIsFatalSktError( rc );
}

//============================================================================
void VxSktBase::onOncePer30Seconds( VxGUID& myOnlineId, bool sktMgrLocked )
{
	if( eSktTypeTcpConnect != m_eSktType && eSktTypeTcpAccept != m_eSktType )
	{
		// not a tcp type that can use im alive packets
		return;
	}

    if( !isConnected() )
    {
        // do not send alive so connection will timeout if for some reason it never closed
        return;
    }

	if( isTempConnection() )
	{
		// do not send alive so connection will timeout if for some reason it never closed
		return;
	}

	int64_t timeNow( GetGmtTimeMs() );
	if( isNetServiceConnection() )
	{
		int64_t timeLastActive = getLastActiveTimeMs();
		if( timeNow - timeLastActive > NET_SERVICE_TIMEOUT_MS )
		{
			LogMsg( LOG_VERBOSE, "VxSktBase::onOncePer30Seconds net service timeout skt hande %d num %d id %s peer %s desc %s",
					getSktHandle(), getSktNumber(), getSocketIdText().c_str(),
					describePeerUser().c_str(), describeSktConnection().c_str() );
            closeSkt( eSktCloseNetServiceTimeout, sktMgrLocked );
		}
	
		return;
	}

	int64_t timeAliveRx( getLastImAliveTimeRxMs() );
	int64_t timeAliveTx( getLastImAliveTimeTxMs() );
	if( timeAliveTx && timeNow - timeAliveRx > IM_ALIVE_TIMEOUT_MS )
	{
        LogMsg( LOG_VERBOSE, "VxSktBase::onOncePer30Seconds im alive timeout skt handle %d num %d tmp %d id %s peer %s desc %s",
				getSktHandle(), getSktNumber(), isTempConnection(), getSocketIdText().c_str(),
				describePeerUser().c_str(), describeSktConnection().c_str() );
        closeSkt( eSktCloseImAliveTimeout, sktMgrLocked );
	}
	else if( getIsPeerPktAnnSet() )
	{
		PktImAliveReq pktImAliveReq;
		pktImAliveReq.setSrcOnlineId( myOnlineId );
		pktImAliveReq.setDestOnlineId( getPeerOnlineId() );

		int32_t rc = txPacketWithDestId( &pktImAliveReq, sktMgrLocked );
		if( rc )
		{
            LogMsg( LOG_VERBOSE, "VxSktBase::%s tx im alive error %d skt hande %d num %d id %s peer %s",
                    __func__, rc, getSktHandle(), getSktNumber(), getSocketIdText().c_str(),
					describePeerUser().c_str() );
		}

		setLastImAliveTimeTxMs( timeNow );
	}
    else if( !isTempConnection() )
    {
        LogMsg( LOG_ERROR, "%s no peer user so cannot send PktImAliveReq to %s", __func__, getRemoteIp().c_str() );
		if( timeNow - getRxStartTimeMs() > 120000 )
		{
			// has been connected for at least a minute
			if( !getRxPktAnnTimeMs() )
			{
				// failed to send us a PktAnnounce
				//VxReportHack( eHackerLevelSevere, eHackerReasonLurkerDidNotSendPktAnn, m_Socket, getRemoteIp().c_str(), "VxSktBase::onOncePer30Seconds" );
                // if the other side is hung then not necessarily a hacker, so just close the socket
                LogMsg( LOG_ERROR, "VxSktBase::%s no peer pkt ann received for skt handle %d num %d ip %s closing socket", __func__, 
                    getSktHandle(), getSktNumber(), getRemoteIp().c_str() );
				closeSkt( eSktCloseHackLevelSevere, sktMgrLocked );
			}
		}
    }
}

//============================================================================
void VxSktBase::setIsNetServiceConnection( bool isNetSrv )
{
	if( !isNetSrv )
	{
		m_IsNetServiceConnection = false;
		return;
	}

	m_IsNetServiceConnection = true;
	int64_t timeNow( GetGmtTimeMs() );
	setLastImAliveTimeRxMs( timeNow );
	setLastImAliveTimeTxMs( timeNow );
	setLastActiveTimeMs( timeNow );
	setLastSessionTimeMs( timeNow );
}

//============================================================================
void VxSktBase::addConnectReason( enum EConnectReason connectReason )
{
	if( IsConnectReasonTemporary( connectReason ) )
	{
		LogMsg( LOG_ERROR, "VxSktBase::%s temporary connect reason cannot be added to list", __func__ );
		return;
	}

	bool reasonAlreadyExists{ false };
	// time mutex is rarely used. we just need to avoid multiple threads accessing m_ConnectReasonList
	lockTimeAccess();
	for( auto reason : m_ConnectReasonList )
	{
		if( reason == connectReason )
		{
			reasonAlreadyExists = true;
			break;
		}
	}

	if( !reasonAlreadyExists )
	{
		m_ConnectReasonList.emplace_back( connectReason );
	}

	unlockTimeAccess();
}

//============================================================================
bool VxSktBase::removeConnectReason( enum EConnectReason connectReason )
{
	lockTimeAccess();
	for( auto iter = m_ConnectReasonList.begin(); iter != m_ConnectReasonList.end(); ++iter )
	{
		if( *iter == connectReason )
		{
			m_ConnectReasonList.erase( iter );
			break;
		}
	}

	bool reasonsAreEmpty = m_ConnectReasonList.empty();
	unlockTimeAccess();
	return reasonsAreEmpty;
}

//============================================================================
void VxSktBase::setIsTempConnection( bool isTemp ) 
{ 
	if( m_IsTempConnection != isTemp )
	{
		m_IsTempConnection = isTemp;
        if( isTemp )
		{
            if(LogEnabled(eLogConnect))LogMsg( LOG_VERBOSE, "VxSktBase::%s tmp skt num %d handle %d id %s", __func__,
                      m_SktNumber, m_Socket, getSocketIdText().c_str() );
		}
	}
}

//============================================================================
VxSktStatRecord	 VxSktBase::getSktStatRecord( void )
{
	return VxSktStatRecord( m_Socket, m_strRmtIp, m_ConnectionId, GetBytesSent(), GetBytesReceived(), m_LastActiveTimeGmtMs, isTempConnection(), getPeerOnlineId() );
}
