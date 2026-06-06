//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktConnectSimple.h"

#include "VxSktBase.h"

#include <CoreLib/ISktStatCallbackInterface.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxResolveHost.h>
#include <CoreLib/VxSktUtil.h>
#include <CoreLib/VxThread.h>

#include <PktLib/VxPktHdr.h>

#include <memory.h>

#ifndef TARGET_OS_WINDOWS
	#include <fcntl.h>
#endif // TARGET_OS_WINDOWS

//============================================================================
VxSktConnectSimple::VxSktConnectSimple()
: m_Socket( INVALID_SOCKET )
{
    m_SktNumber = VxSktBase::getNewSktNumber();
}

//============================================================================
VxSktConnectSimple::~VxSktConnectSimple()
{
	closeSkt( 99 );
}

//============================================================================
std::string VxSktConnectSimple::getLocalIpAddress( void )
{ 
	std::string lclIp = m_LclIp.toString();
	if( lclIp.empty() )
	{
		lclIp = "";
		if( INVALID_SOCKET != m_Socket )
		{
			uint16_t retPort{ 0 };
			lclIp = VxGetLclIpAddress( m_Socket, &retPort );
			if( retPort && !lclIp.empty() )
			{
				m_LclIp.setIpAndPort( lclIp.c_str(), retPort );
			}
		}
	}

	return lclIp;
}

//============================================================================
bool VxSktConnectSimple::isConnected( void )
{
	if( INVALID_SOCKET == m_Socket )
	{
		m_bIsConnected = false;
	}

	return m_bIsConnected;
}

//============================================================================
//! connect to remote ip
SOCKET VxSktConnectSimple::connectTo(	const char*		pIpOrUrl,				// remote ip or url
										uint16_t		u16Port,				// port to connect to
                                        EIpAddrType     addrType,
										int				iTimeoutMilliSeconds,
                                        int32_t*		retErrorCode )	
{
	if( isConnected() )
	{
		LogMsg( LOG_INFO, "VxSktConnectSimple::connectTo: connect attempt on allready connected socket thread 0x%x", VxGetCurrentThreadId() );
		return -1;
	}

	m_bIsConnected	= false;
	m_Socket		= VxConnectTo( m_LclIp, m_RmtIp, pIpOrUrl, u16Port, addrType, iTimeoutMilliSeconds, retErrorCode );
	if( INVALID_SOCKET != m_Socket )
	{
		VxGetLclAddress( m_Socket, m_LclIp );
		m_bIsConnected = true;
        if( VxGetSktStatCallback() )
        {
            VxGetSktStatCallback()->sktSetType( m_Socket, eSktTypeSimple );
        }
	}

	return m_Socket;
}

//============================================================================
SOCKET VxSktConnectSimple::connectTo( const char*   lclAdapterIp,					// local adapter ip
                                      const char*	pIpOrUrl,						// remote ip or url
                                      uint16_t		u16Port,						// port to connect to
                                      EIpAddrType   addrType,
                                      int			iTimeoutMilliSeconds,
                                      int32_t*		retErrorCode )	       
{
    int32_t rc = 0;
    InetAddrAndPort	rmtIpAddr;
    if( VxIsIPv4Address( pIpOrUrl ) || VxIsIPv6Address( pIpOrUrl ) )
    {
        rmtIpAddr.setIp( pIpOrUrl );
        rmtIpAddr.setPort( u16Port );
    }
    else if( VxResolveHostToIp( pIpOrUrl, u16Port, rmtIpAddr, addrType ) )
    {
        rmtIpAddr.setPort( u16Port );
    }
    else
    {
        return INVALID_SOCKET;
    }

    m_bIsConnected = false;
    m_Socket = INVALID_SOCKET;
    std::string strLclAddr = lclAdapterIp;
    InetAddress lclIpAddr( lclAdapterIp );				// local ip address

    // attempt connect
    // Open a socket with the correct address family for this address.
    SOCKET skt = socket( rmtIpAddr.isIPv4() ? PF_INET : PF_INET6, SOCK_STREAM, 0 );
    if( skt == INVALID_SOCKET )
    {
        LogMsg( LOG_INFO, "VxSktConnectSimple::connectTo: failed to create socket thread 0x%x", VxGetCurrentThreadId() );
    }
    else
    {
#if USE_BIND_LOCAL_IP
        // when using a vpn then binding to local ip causes connection fail..
        struct sockaddr_storage oLclSktStorage;
        lclIpAddr.fillAddress( oLclSktStorage, 0 );
        if( false == VxBindSkt( skt, &oLclSktStorage ) )
        {
            LogMsg( LOG_INFO, "VxSktConnectSimple::connectTo: failed to bind skt with ip %s\n", strLclAddr.c_str() );
        }
        else
        {
#endif // USE_BIND_LOCAL_IP
            struct sockaddr_storage rmtSktAddr;
            int iRmtSktAddrLen = rmtIpAddr.fillAddress( rmtSktAddr, u16Port );

            rc = connect( skt, ( struct sockaddr * )&rmtSktAddr, iRmtSktAddrLen );
            if( 0 == rc )
            {
                rc = VxGetLclAddress( skt, m_LclIp );
                if( !rc )
                {
                    rc = VxGetRmtAddress( skt, m_RmtIp, true );
                }

                if( !rc )
                {
                    m_Socket = skt;
                    m_bIsConnected = true;
                    if( VxGetSktStatCallback() )
                    {
                        VxGetSktStatCallback()->sktConnected4( m_Socket, rmtIpAddr.toString(), eSktTypeSimple, eConnectReasonUnknown );
                    }
                }
            }
            else
            {
                rc = VxGetLastError();
                if( !rc )
                {
                    rc = -1;
                }
            }

            if( rc )
            {
                if( skt != INVALID_SOCKET )
                {
                    VxCloseSkt( skt );
                    skt = INVALID_SOCKET;
                }

                LogMsg( LOG_ERROR, "TestConnectionOnSpecificLclAddress: connect error %s thread 0x%x", VxDescribeSktError( rc ), VxGetCurrentThreadId() );
            }
#if USE_BIND_LOCAL_IP
        }
#endif // #if USE_BIND_LOCAL_IP
    }

    if( INVALID_SOCKET == skt && retErrorCode )
    {
        *retErrorCode = rc;
    }

    return skt;
}

//============================================================================
int32_t VxSktConnectSimple::sendData(	const char*		pData,					// data to send
									int				iDataLen,				// length of data
									int				timeoutMs )		        // milliseconds before send attempt times out
{
    int32_t rc = -1;
    setLastError( rc );
	if( false == this->isConnected() )
	{
		LogMsg( LOG_INFO, "VxSktConnectSimple::sendData: attempted send on disconnected socket thread 0x%x", VxGetCurrentThreadId() );
		return rc;
	}

    if( isRxCryptoKeySet() && iDataLen >= sizeof( VxPktHdr ) )
    {
        if( encryptAndSendTxData( pData, iDataLen, timeoutMs ) )
        {
            rc = 0;
        }
    }
    else
    {
	    rc = VxSendSktData( m_Socket, pData, iDataLen, timeoutMs );

	    if( rc || (INVALID_SOCKET == m_Socket) )
	    {
		    closeSkt(8689);		
	    }
    }

    if(LogEnabled( eLogSktTx ))
    {
        LogModule( eLogSktTx, LOG_INFO, "VxSktConnectSimple::%s: skt num %d tx %d bytes to %s", __func__, 
            getSktNumber(), iDataLen, m_RmtIp.toString().c_str() );
    }

    setLastError( rc );
	return rc;
}
		
//============================================================================
//! receive data.. if timeout is set then will keep trying till buffer is full or error or timeout expires
int32_t VxSktConnectSimple::recieveData(char *			pRetBuf,				// buffer to receive data into
										int				iBufLenIn,				// length of buffer
										int *			iRetBytesReceived,		// number of bytes actually received
										int				iTimeoutMilliSeconds )	// milliseconds before receive attempt times out ( 0 = dont wait )
{
	int32_t rc = VxReceiveSktData( m_Socket, pRetBuf, iBufLenIn, iRetBytesReceived, iTimeoutMilliSeconds );
    setLastError( rc );
    if( VxIsFatalSktError( rc ) )
    {
        LogMsg( LOG_INFO, "VxSktConnectSimple::%s: skt %d failed length %d timeout %d error %s", __func__,
            getSktHandle(), iBufLenIn, iTimeoutMilliSeconds, VxDescribeSktError( rc ) );
        closeSkt( 8690 );
    }

    if(LogEnabled( eLogSktRx ))
    {
        LogModule( eLogSktRx, LOG_INFO, "VxSktConnectSimple::%s: skt num %d rx %d bytes from %s", __func__, 
            getSktNumber(), *iRetBytesReceived, m_RmtIp.toString().c_str() );
    }

    return rc;
}

//============================================================================
//! close socket
void VxSktConnectSimple::closeSkt( int iInstance )
{
	m_bIsConnected	= false;
	if( INVALID_SOCKET != m_Socket )
	{
		SOCKET skt = m_Socket;
		m_Socket = INVALID_SOCKET;

		VxCloseSktNow( skt );
	}
}

//============================================================================
bool VxSktConnectSimple::connectToWebsite(	const char*			pWebsiteUrl,
											std::string&		strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
											std::string&		strFile,		// return file name.. images/me.png
											uint16_t&           u16Port,
                                            EIpAddrType			addrType,
											int					iConnectTimeoutMs,
                                            int*		        retErrorCode )
{
	std::string strResolveIpAddr;
	return connectToWebsite( pWebsiteUrl,
							strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
							strFile,		// return file name.. images/me.png
							u16Port,
							strResolveIpAddr,
                            addrType,
							iConnectTimeoutMs,
                            retErrorCode );

}

//============================================================================
bool VxSktConnectSimple::connectToWebsite( const char*			pWebsiteUrl,
											std::string&		strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
											std::string&		strFile,		// return file name.. images/me.png
											uint16_t&			u16Port,
											std::string&		strResolveIpAddr,
                                            EIpAddrType			addrType,
											int					iConnectTimeoutMs,
                                            int*		        retErrorCode )
{
	closeSkt( 99 );
    setLastError( 0 );
	m_bIsConnected	= false;
	m_Socket = VxConnectToWebsite( m_LclIp, m_RmtIp, pWebsiteUrl, strHost, strFile, u16Port, addrType, iConnectTimeoutMs, retErrorCode );
	strResolveIpAddr = m_RmtIp.toString();
	if( INVALID_SOCKET != m_Socket )
	{
		m_bIsConnected = true;
        int rc = VxGetLclAddress( m_Socket, m_LclIp );
        if( !rc )
        {
            rc = VxGetRmtAddress( m_Socket, m_RmtIp );
        }

        if( rc )
        {
            setLastError( rc );
            m_bIsConnected = false;
            VxCloseSktNow( m_Socket );
            m_Socket = INVALID_SOCKET;
        }


#ifdef DEBUG_SKT
		LogMsg( LOG_INFO, "VxSktConnectSimple::connectToWebsite Lcl port %d (0x%4.4x) Rmt port %d (0x%4.4x)", 
			m_LclIp.getPort(),
			m_LclIp.getPort(),
			m_RmtIp.getPort(),
			m_RmtIp.getPort() );
#endif // DEBUG_SKT
	}

	return m_bIsConnected;
}


//============================================================================
void VxSktConnectSimple::setTxCryptoPassword( const char* data, int len )
{
	m_TxKey.m_bIsSet = true;
	m_TxCrypto.setPassword( data, len );
}

//============================================================================
void VxSktConnectSimple::setRxCryptoPassword( const char* data, int len )
{
	m_RxKey.m_bIsSet = true;
	m_RxCrypto.setPassword( data, len );
}

//============================================================================
bool VxSktConnectSimple::isTxCryptoKeySet( void )
{
	return m_TxKey.m_bIsSet;
}

//============================================================================
bool VxSktConnectSimple::isRxCryptoKeySet( void )
{
	return m_RxKey.m_bIsSet;
}

//============================================================================
bool VxSktConnectSimple::encryptAndSendTxData( const char* pDataIn, int iDataLen, int timeoutMs )
{
    if( !pDataIn )
    {
        LogMsg( LOG_ERROR, "VxSktConnectSimple::encryptTxData null data");

        vx_assert( pDataIn );
        return false;
    }

    if( !iDataLen )
    {
        LogMsg( LOG_ERROR, "VxSktConnectSimple::encryptTxData invalid data len %d", iDataLen);

        vx_assert( pDataIn );
        return false;
    }

	if( !isConnected() )
	{
		LogMsg( LOG_VERBOSE, "VxSktConnectSimple::encryptTxData skt is already disconnected" );
		return false;
	}

	if( 0 != (iDataLen & 0x0f) )
	{
		LogMsg( LOG_ERROR, "VxSktConnectSimple::encryptTxData invalid pkt len %d (pkt type %d)", iDataLen, ((VxPktHdr*)pDataIn)->getPktType() );

        vx_assert( 0 == (iDataLen & 0x0f) );
        return false;
	}

    if( !m_TxCrypto.isKeyValid() )
    {
        LogMsg( LOG_ERROR, "VxSktConnectSimple::encryptTxData invalid crypto key");
        vx_assert( m_TxCrypto.isKeyValid() );

        return false;
    }

    bool status{ false };

	// make copy of data so data is not destroyed
	uint8_t * pu8Data = new uint8_t[ iDataLen ];
	memcpy( pu8Data, pDataIn, iDataLen );

	// encrypt
	int32_t rc =	m_TxCrypto.encrypt( pu8Data, iDataLen );
	if( rc )
	{
		LogMsg( LOG_ERROR, "VxSktConnectSimple::encryptTxData: crypto error %d", rc );
        setLastError( rc );
		vx_assert( 0 == rc );
	}
	else
	{
		// send
        rc = VxSendSktData( m_Socket, (const char *)pu8Data, iDataLen, timeoutMs );
	    if( rc || (INVALID_SOCKET == m_Socket) )
	    {
            setLastError( rc );
		    closeSkt(8690);		
	    }
        else
        {
            status = true;
        }
	}

	delete[] pu8Data;

	return status;
}


//============================================================================
bool VxSktConnectSimple::decryptReceiveData( char* data, int iDataLen )
{
	if( false == isRxEncryptionKeySet() )
	{
		// no key to decrypt with
		LogMsg( LOG_INFO, "%s No Rx Crypto Key Set", __func__ );
		return false;
	}

    uint32_t u32Datalen = iDataLen;
    if( 0 != (u32Datalen & 0x0f ) )
    {
        LogMsg( LOG_INFO, "%s Invalid data len not a multiple of 16", __func__ );
        return false;
    }

    if( !u32Datalen )
    {
        LogMsg( LOG_INFO, "%s Invalid data len is 0", __func__ );
        return false;
    }

	m_RxCrypto.decrypt( (uint8_t *)data, u32Datalen );

	return true;
}


//============================================================================
void VxSktConnectSimple::dumpConnectionInfo( void )
{
    if( isConnected() )
    {
        InetAddrAndPort  rmtAddr;
        InetAddrAndPort  lclAddr;

        VxGetRmtAddress( m_Socket, rmtAddr );
        uint16_t rmtPort = rmtAddr.getPort();
        std::string rmtIp = rmtAddr.toString();

        VxGetLclAddress( m_Socket, lclAddr );
        uint16_t lclPort = lclAddr.getPort();
        std::string lclIp = lclAddr.toString();

        LogMsg( LOG_INFO, "VxSktConnectSimple: Rmt ip %s port %d Lcl ip %s port %d thread 0x%x",
                rmtIp.c_str(),
                rmtPort,
                lclIp.c_str(),
                lclPort, VxGetCurrentThreadId() );

        rmtPort = m_RmtIp.getPort();
        rmtIp = m_RmtIp.toString();

        lclPort = m_LclIp.getPort();
        lclIp = m_LclIp.toString();
        LogMsg( LOG_INFO, "VxSktConnectSimple: 2 Rmt ip %s port %d Lcl ip %s port %d thread 0x%x",
            rmtIp.c_str(),
            rmtPort,
            lclIp.c_str(),
            lclPort, VxGetCurrentThreadId() );
    }
}

