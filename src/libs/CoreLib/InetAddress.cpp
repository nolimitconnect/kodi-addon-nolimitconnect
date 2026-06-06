//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "InetAddress.h"

#include "InetAddressParse.h"
#include "IsBigEndianCpu.h"

#include "PktBlobEntry.h"
#include "VxDebug.h"
#include "VxParse.h"
#include "VxSktUtil.h"

#include <string>
#include <array>

#ifdef TARGET_OS_WINDOWS
	#include <Winsock2.h>
	#include <Ws2tcpip.h>  
#else
    #include <arpa/inet.h>
    #include <net/if.h>
    #include <netdb.h>        /* getaddrinfo(3) et al.                       */
    #include <netinet/in.h>   /* sockaddr_in & sockaddr_in6 definition.      */
	#include <unistd.h> 
    #include <sys/socket.h>
    #include <sys/types.h>

	#ifndef TARGET_OS_ANDROID
		#include <ifaddrs.h>
	#endif
#if defined(TARGET_OS_ANDROID)
    #include "ifaddrs-android-impl.h"
#endif
    #include <stdio.h>
    #include <sys/types.h>
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <string.h>
    #include <arpa/inet.h>
#endif

#include <memory.h>
#include <stdio.h>

namespace 
{
  const uint64_t IP4_BINARY_INDICATOR = 0xffffffffffffffffULL;
};

//============================================================================
InetAddress::InetAddress( const char* pIpAddress )
: m_u64AddrHi(0)
, m_u64AddrLo(0)
{
	fromString( pIpAddress );
}

//============================================================================
InetAddress::InetAddress( uint32_t u32IpAddr )
{
	setIp( u32IpAddr );
}

//============================================================================
InetAddress::InetAddress( const InetAddress& rhs )
    : m_u64AddrHi( rhs.m_u64AddrHi )
    , m_u64AddrLo( rhs.m_u64AddrLo )
{
}

//============================================================================
bool InetAddress::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_u64AddrHi );
    result &= blob.setValue( m_u64AddrLo );
    return result;
}

//============================================================================
bool InetAddress::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_u64AddrHi );
    result &= blob.getValue( m_u64AddrLo );
    return result;
}

//============================================================================
uint16_t InetAddress::fromString( const char* pIpAddress )
{
	if( !pIpAddress || 
		( 0 == strlen( pIpAddress ) ) ||
		( 0 == strcmp( "0.0.0.0", pIpAddress ) ) )
	{
		LogMsg( LOG_WARN, "InetAddress::fromString invalid ip %s", pIpAddress ? pIpAddress : "NULL" );
		setToInvalid();
		return 0;
	}

	uint16_t port = 0;
	bool parsedIsIpv6 = false;
	uint8_t ipBinary[16];
	if( ParseIPv4OrIPv6( pIpAddress, ipBinary, port, parsedIsIpv6 ) )
	{
		// the parser uses net order so no need to change it
		if( parsedIsIpv6 )
		{
			memcpy( &m_u64AddrHi, ipBinary, sizeof( m_u64AddrHi ) );
			memcpy( &m_u64AddrLo, &ipBinary[8], sizeof( m_u64AddrLo ));
		}
		else
		{
			m_u64AddrLo = 0;
			memcpy( &m_u64AddrLo, ipBinary, sizeof( uint32_t ) );
			m_u64AddrHi = IP4_BINARY_INDICATOR; // indicator is a ipv4 address
		}

		return port;
	}

	setToInvalid();
	return 0;
}

//============================================================================
std::string InetAddress::toString( void ) const
{
	std::string retIpAddress;
	std::array<char, INET6_MAX_STR_LEN> as8Buf;
	as8Buf.data()[0] = 0; 

	if( !isValid() )
	{
		return retIpAddress;
	}

	if( isIPv4() )
	{
		uint32_t u32Ip = getIPv4AddressInNetOrder();
		return ipv4BinaryToString( (uint8_t *)&u32Ip );
	}
	else
	{
		uint8_t ipBinary[16];
		memcpy( ipBinary, &m_u64AddrHi, sizeof( m_u64AddrHi ) );
		memcpy( &ipBinary[8], &m_u64AddrLo, sizeof(m_u64AddrLo) );
		return ipv6BinaryToString( ipBinary );
	}

	retIpAddress = as8Buf.data();
	if( retIpAddress.empty() )
	{
		retIpAddress = "";
	}

	return retIpAddress;
}

//============================================================================
EIpAddrType InetAddress::getIpAddrType( void )
{
	EIpAddrType addrType{ eIpAddrTypeUnknown };
	if( isValid() )
	{
		addrType = isIPv4() ? eIpAddrTypeIpv4 : eIpAddrTypeIpv6;
	}

	return addrType;
}

//============================================================================
InetAddress InetAddress::getDefaultIp( void )
{
	 std::vector<InetAddress> retAddresses;
	 getAllAddresses( retAddresses );
	 if( retAddresses.size() )
	 {
		 return retAddresses[0];
	 }
	 return InetAddress();
}

//============================================================================
int InetAddress::getAllAddresses( std::vector<InetAddress>& retAddresses )
{
#if defined(TARGET_OS_WINDOWS) || defined(TARGET_OS_ANDROID)
	// for unknown reasons this code that works on windows only return loopback in android
	// NO known fix and the linux version not viable because of missing ifaddr.h
	// TODO.. if android fetch addresses from JAVA
	char as8HostName[ 1025 ];
	//first get host name
	if( gethostname( as8HostName, sizeof( as8HostName ) ) )
	{
        LogMsg( LOG_ERROR, "getAllAddresses: Unable to get host name" );
		#ifdef TARGET_OS_WINDOWS
			return WSAGetLastError();
		#else
			return VxGetLastError();
		#endif // TARGET_OS_WINDOWS
	}

	struct addrinfo Hints;
	struct addrinfo * AI;
	struct addrinfo * AddrInfo;

	memset(&Hints, 0, sizeof(Hints));

    Hints.ai_family   = PF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_protocol = IPPROTO_TCP;
    //Hints.ai_flags    = AI_PASSIVE;
	Hints.ai_flags = AI_ADDRCONFIG;

	char as8Buf[16];
	sprintf( as8Buf, "%d", 65000 );

	int RetVal = getaddrinfo(as8HostName, "echo", &Hints, &AddrInfo);
	if (RetVal != 0)
	{
		//char * pErr = gai_strerror(RetVal);
		//printf("getaddrinfo() failed with error %d: %s\n", RetVal, pErr );
        LogMsg( LOG_ERROR, "InetAddress::getAllAddresses getaddrinfo error %d", RetVal );
		return RetVal;
	}

	for(  AI = AddrInfo; AI != NULL; AI = AI->ai_next )
	{
		if ((AI->ai_family != PF_INET) && (AI->ai_family != PF_INET6))
		{
			continue;
		}
		// Open a socket with the correct address family for this address.
		SOCKET oSkt = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);

		if( oSkt == INVALID_SOCKET )
		{
			continue;
		}

        struct sockaddr_storage * poSktAddr = (struct sockaddr_storage *)AI->ai_addr;
        InetAddress oTestAddr;
        VxSetSktAddressPort( poSktAddr, 0 );
        oTestAddr.setIp( *poSktAddr );
        std::string strTestIpAddress = oTestAddr.toString();        

        if( ( false == oTestAddr.isValid() ) ||
            ( oTestAddr.isLoopBack() ) )
        {
            if( ! oTestAddr.isLoopBack() )
            {
                LogMsg( LOG_INFO, "InetAddress::%s: invalid addr %s", __func__, strTestIpAddress.c_str() );
            }

            VxCloseSkt( oSkt );
            continue;
        }

		//LogMsg( LOG_INFO, "binding skt %d\n", oSkt );
		if( false == VxBindSkt( oSkt, poSktAddr ) )
        {
            LogMsg( LOG_INFO, "InetAddress::%s: could not bind addr %s", __func__, strTestIpAddress.c_str() );
            continue;
        }

        if( oTestAddr.isValid() &&
            ( false == oTestAddr.isLoopBack() ) )
        {
            retAddresses.push_back(oTestAddr);
        }

//#define TEST_CONNECTION
#ifdef TEST_CONNECTION
        if( false == VxTestConnection( oSkt, oTestAddr ) )
        {
            LogMsg( LOG_INFO, "Connection using local address %s OK", strTestIpAddress.c_str() );
        }
        else
        {
            LogMsg( LOG_INFO, "Connection using local address %s FAIL", strTestIpAddress.c_str() );
        }
#else
		//LogMsg( LOG_INFO, "closing skt %d\n", oSkt );
        VxCloseSkt( oSkt );
#endif // TEST_CONNECTION
	}

	//LogMsg( LOG_INFO, "freeing addr info\n" );
	freeaddrinfo( AddrInfo ); // free the linked list
#else
    struct ifaddrs *myaddrs, *ifa;
    struct sockaddr_storage * poSktAddr = NULL;
    char buf[64];

    if(getifaddrs(&myaddrs) != 0)
    {
        perror("InetAddress::getAllAddresses: getifaddrs");
        LogMsg( LOG_ERROR, "InetAddress::getAllAddresses: getifaddrs FAIL" );
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            case AF_INET6:
            {
                poSktAddr = (struct sockaddr_storage *)ifa->ifa_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, poSktAddr, buf, sizeof(buf)))
        {
           LogMsg( LOG_ERROR, "InetAddress::getAllAddresses: %s: inet_ntop failed!", ifa->ifa_name);
        }
        else
        {
            // Open a socket with the correct address family for this address.
            SOCKET oSkt = socket(ifa->ifa_addr->sa_family,
                                 SOCK_STREAM,
                                 0 );

            if( oSkt == INVALID_SOCKET )
            {
                continue;
            }

            //LogMsg( LOG_INFO, "InetAddress::getAllAddresses: %s: %s\n", ifa->ifa_name, buf);
            InetAddress oTestAddr;
            VxSetSktAddressPort(poSktAddr, 0);
            oTestAddr.setIp( *poSktAddr );
            std::string strTestIpAddress = oTestAddr.toString();

            if( ( false == oTestAddr.isValid() ) ||
                ( oTestAddr.isLoopBack() ) )
            {
                //if( oTestAddr.isLoopBack() )
                //{
                //    LogMsg( LOG_INFO, "InetAddress::getAllAddresses: loopback addr %s\n", strTestIpAddress.c_str() );
                //}
                //else
                //{
                //    LogMsg( LOG_INFO, "InetAddress::getAllAddresses: invalid addr %s\n", strTestIpAddress.c_str() );
                //}
                VxCloseSkt( oSkt );
                continue;
            }

            if( false == VxBindSkt( oSkt, poSktAddr ) )
            {
                LogMsg( LOG_INFO, "InetAddress::getAllAddresses: could not bind addr %s", strTestIpAddress.c_str() );
                VxCloseSkt( oSkt );
                continue;
            }

            if( oTestAddr.isValid() &&
                ( false == oTestAddr.isLoopBack() ) )
            {
                retAddresses.push_back(oTestAddr);
            }

    //#define TEST_CONNECTION
    #ifdef TEST_CONNECTION
            if( false == VxTestConnection( oSkt, oTestAddr ) )
            {
                LogMsg( LOG_INFO, "Connection using local address %s OK", strTestIpAddress.c_str() );
            }
            else
            {
                LogMsg( LOG_INFO, "Connection using local address %s FAIL", strTestIpAddress.c_str() );
            }
    #else
            VxCloseSkt( oSkt );
    #endif // TEST_CONNECTION
        }
    }

    freeifaddrs(myaddrs);
#endif // TARGET_OS_WINDOWS

#if defined(TARGET_OS_LINUX)
    if( retAddresses.empty() )
    {
        struct ifaddrs * ifAddrStruct=NULL;
        struct ifaddrs * ifa=NULL;
        void * tmpAddrPtr=NULL;

        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr)
            {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET)
            { // check it is IP4
                // is a valid IP4 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                //LogMsg( LOG_INFO, "%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                InetAddress oTestAddr;
                oTestAddr.setIp( *addressBuffer );
                if( oTestAddr.isValid() && ( false == oTestAddr.isLoopBack() ) )
                {
                    retAddresses.push_back(oTestAddr);
                }
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            { // check it is IP6
                // is a valid IP6 Address
                tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                //LogMsg( LOG_INFO, "%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                InetAddress oTestAddr;
                oTestAddr.setIp( *addressBuffer );
                if( oTestAddr.isValid() && ( false == oTestAddr.isLoopBack() ) )
                {
                    retAddresses.push_back(oTestAddr);
                }
            }
        }

        if (ifAddrStruct != nullptr)
        {
            freeifaddrs(ifAddrStruct);
        }
    }
#endif // defined(TARGET_OS_LINUX)
#if defined(TARGET_OS_ANDROID)
    if( retAddresses.empty() )
    {
        struct ifaddrs_android * ifAddrStruct=NULL;
        struct ifaddrs_android * ifa=NULL;
        void * tmpAddrPtr=NULL;

        getifaddrs_android(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr)
            {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET)
            { // check it is IP4
                // is a valid IP4 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                //LogMsg( LOG_INFO, "%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                InetAddress oTestAddr;
                oTestAddr.setIp( (const char*)(&addressBuffer[0]) );
                if( oTestAddr.isValid() && ( false == oTestAddr.isLoopBack() ) )
                {
                    retAddresses.push_back(oTestAddr);
                }
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            { // check it is IP6
                // is a valid IP6 Address
                tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                //LogMsg( LOG_INFO, "%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                InetAddress oTestAddr;
                oTestAddr.setIp( (const char*)(&addressBuffer[0]) );
                if( oTestAddr.isValid() && ( false == oTestAddr.isLoopBack() ) )
                {
                    retAddresses.push_back(oTestAddr);
                }
            }
        }

        if (ifAddrStruct != nullptr)
        {
            freeifaddrs_android(ifAddrStruct);
        }
    }

#endif // defined(TARGET_OS_ANDROID)

	return 0;
}

//============================================================================
void InetAddress::dumpAddresses( std::vector<InetAddress>& addressList )
{
	LogMsg( LOG_INFO, "InetAddress::dumpAddresses count %zu", addressList.size() );
    static int addrIdx = 0;
    for( InetAddress& addr : addressList )
    {
        addrIdx++;
        LogMsg( LOG_INFO, "Addr %d - %s isIPv4 %d isLoopback %d isLocal %d", addrIdx, addr.toString().c_str(), addr.isIPv4(), addr.isLoopBack(), addr.isLocalAddress() );
    }
}

//============================================================================
bool InetAddress::isLoopBack() const
{
	bool isLoopBack = true;
	if( isIPv4() )
	{
		if( 0x7f000001 != getIPv4AddressInNetOrder() )
		{
			isLoopBack = false;
		}
	}
	else
	{
		std::string ipAddr = toString();
		isLoopBack = ipAddr == "::1";
	}

	return isLoopBack;
}

//============================================================================
bool InetAddress::isLocalAddress( bool forLocalListening ) const
{
	if( false == isValid() )
	{
		LogMsg( LOG_ERROR, "InetAddrIPv4::isLocalAddress invalid address " );
		return false;
	}

	if( isIPv6() ) 
	{
		uint32_t * netOrder = (uint32_t *)&m_u64AddrHi;

		uint32_t ip0 = ntohl(netOrder[0]);
		uint32_t ip1 = ntohl(netOrder[1]);
		uint32_t ip2 = ntohl(netOrder[2]);
		uint32_t ip3 = ntohl(netOrder[3]);

		if( forLocalListening && !ip0 && !ip1 && !ip2 && !ip3 ) // :: *
		{
			return false;
		}

		if( !ip0 && !ip1 && !ip2 &&
			( 0x00000000 == (ip3 & 0xfffffffe) ) )  // ::/127
		{
			return true;
		}

		if( ( 0xfe800000 == ( ip0 & 0xffc00000 ) ) || // fe80  RFC4291
			( 0xfc000000 == ( ip0 & 0xfe000000 ) ) || // fc00  RFC4193
			( 0xfec00000 == ( ip0 & 0xffc00000 ) ) )   // fec0  RFC3879
		{
			return true;
		}

		return false;
	} 
	else 
	{
		uint32_t hostOrderIpv4 = getIPv4AddressInHostOrder();
		if( ( 0x00000000 == ( hostOrderIpv4 & 0xff000000 ) ) || // 0.
			( 0x7f000000 == ( hostOrderIpv4 & 0xff000000 ) ) || // 127.
			( 0x0a000000 == ( hostOrderIpv4 & 0xff000000 ) ) || // 10.		
			( 0xc0a80000 == ( hostOrderIpv4 & 0xffff0000 ) ) || // 192.168.
			( 0xa9fe0000 == ( hostOrderIpv4 & 0xffff0000 ) ) || // 169.254.
			( 0xac100000 == ( hostOrderIpv4 & 0xfff00000 ) ) )  // 172.16?
		{
			return true;
		}

		return false;
	}
}

//============================================================================
InetAddress& InetAddress::operator=(const InetAddress& oAddr) 
{
	if( this != &oAddr )
	{
		m_u64AddrHi = oAddr.m_u64AddrHi;
		m_u64AddrLo = oAddr.m_u64AddrLo;
	}

	return *this;
}

//============================================================================
bool InetAddress::operator == (const InetAddress& oAddr)  const
{
	return (m_u64AddrHi == oAddr.m_u64AddrHi) && (m_u64AddrLo == oAddr.m_u64AddrLo);
}

//============================================================================
bool InetAddress::operator != (const InetAddress& oAddr)  const
{
	return (m_u64AddrHi != oAddr.m_u64AddrHi) || (m_u64AddrLo != oAddr.m_u64AddrLo);
}

//============================================================================
bool InetAddress::isValid( void ) const
{
	return isIPv4() || !( 0 == m_u64AddrHi && 0 == m_u64AddrLo );
}

//============================================================================
void InetAddress::setToInvalid( void )
{
	m_u64AddrHi = 0;
	m_u64AddrLo = 0;
}

//============================================================================
bool InetAddress::isIPv4String( const char* pIpAddress ) const
{
	if( pIpAddress
		 && (strlen(pIpAddress) < 16 )
		 && strchr(pIpAddress, '.')
		 && ( 0 == strchr(pIpAddress, ':')) )
	{
		return true;
	}

	return false;
}

//============================================================================
bool InetAddress::isIPv4( void ) const
{
	if( IP4_BINARY_INDICATOR == m_u64AddrHi && 0 != m_u64AddrLo ) 
	{
		return true;
	}

	return false;
}

//============================================================================
bool InetAddress::isIPv6( void ) const
{
	return isValid() && !isIPv4();
}

//============================================================================
bool InetAddress::isIPv6GlobalAddress( void ) const
{
	// NOTE: bad assumption. better to check for locals instead.
	if( isIPv6() && !isLocalAddress() )
	{
		return true;
	}

	return false;
}

//============================================================================
uint32_t InetAddress::getIPv4AddressInHostOrder( void ) const
{
	return ntohl(*((uint32_t*)&m_u64AddrLo));
}

//============================================================================
uint32_t InetAddress::getIPv4AddressInNetOrder( void ) const
{
	return *((uint32_t*)&m_u64AddrLo);
}

//============================================================================
// note.. internally kept in network order instead of host order
void InetAddress::setIp( uint32_t u32IPv4Addr )
{
	if( u32IPv4Addr )
	{
		*((uint32_t*)&m_u64AddrLo) = u32IPv4Addr;
		m_u64AddrHi = IP4_BINARY_INDICATOR;
	}
	else
	{
		setToInvalid();
	}
}

//============================================================================
uint16_t InetAddress::setIp( const char* pIp )
{
	return fromString( pIp );
}

//============================================================================
//! returns port in host order
uint16_t InetAddress::setIp( struct sockaddr_in& ipv4Addr )
{
    setIp( *((uint32_t*)&ipv4Addr.sin_addr) );
	return ntohs( ipv4Addr.sin_port );
}

//============================================================================
//! returns port in host order
uint16_t InetAddress::setIp( struct sockaddr_in6& ipv6Addr )
{
	uint8_t* binaryIp = (uint8_t*)&ipv6Addr.sin6_addr;
	memcpy( &m_u64AddrHi, binaryIp, sizeof( m_u64AddrHi ) );
	memcpy( &m_u64AddrLo, &binaryIp[8], sizeof( m_u64AddrLo ) );

	return ntohs( ipv6Addr.sin6_port );
}

//============================================================================
//! returns port in host order
uint16_t InetAddress::setIp( struct sockaddr& ipAddr )
{
	if( AF_INET == ipAddr.sa_family )
	{
		return setIp( *((sockaddr_in *)&ipAddr) );
	}
	else if( AF_INET6 == ipAddr.sa_family )
	{
		return setIp( *((sockaddr_in6 *)&ipAddr) );
	}
	else
	{
		LogMsg( LOG_ERROR, "InetAddress::setIp unknown family" );
		return 0;
	}
}

//============================================================================
//! returns port in host order
uint16_t InetAddress::setIp( struct sockaddr_storage& oAddr )
{
	switch( oAddr.ss_family ) 
	{
	case AF_INET:
		return setIp(*((struct sockaddr_in *)&oAddr));
		break;

	case AF_INET6:
		return setIp(*((struct sockaddr_in6 *)&oAddr));
		break;

	default:
		//vx_assert(false);
		return 0;
	}
}

//============================================================================
//! fill address with this ip address and the given port
int InetAddress::fillAddress( struct sockaddr_storage& storageAddr, uint16_t u16Port )
{
    memset( &storageAddr, 0, sizeof( struct sockaddr_storage ) );
	if( isIPv4() )
	{
        return fillAddress( *((struct sockaddr_in*)&storageAddr), u16Port );
	}
	else
	{
        return fillAddress( *((struct sockaddr_in6*)&storageAddr), u16Port );
	}
}

//============================================================================
//! fill address with this ip address and the given port.. returns struct len
int InetAddress::fillAddress( struct sockaddr_in& ipv4Addr, uint16_t u16Port )
{
	// setup the address and port
	memset( &ipv4Addr, 0, sizeof( sockaddr_in ) );

	ipv4Addr.sin_family			= AF_INET;
	*((uint32_t*)&ipv4Addr.sin_addr) = getIPv4AddressInNetOrder();

	ipv4Addr.sin_port				= htons( u16Port );
	return (int)sizeof( struct sockaddr_in);
}

//============================================================================
//! fill address with this ip address and the given port.. returns struct len
int InetAddress::fillAddress( struct sockaddr_in6& ipv6Addr, uint16_t u16Port )
{
	// setup the address and port
	memset( &ipv6Addr, 0, sizeof( sockaddr_in6 ) );

	ipv6Addr.sin6_family			= AF_INET6;
	uint8_t* addrBuf = (uint8_t*) & ipv6Addr.sin6_addr;

	memcpy( addrBuf, &m_u64AddrHi, 8 );
	memcpy( &addrBuf[8], &m_u64AddrLo, 8 );

	ipv6Addr.sin6_port	= htons( u16Port );
	return (int)sizeof( struct sockaddr_in6);
}

//============================================================================
//! returns port in host order
uint16_t InetAddress::getIpFromAddr(const struct sockaddr *sa, std::string& retStr)
{
	uint16_t u16Port = 0;
    switch(sa->sa_family)
    {
		case AF_INET:
			retStr = ipv4BinaryToString( (uint8_t *) & (((struct sockaddr_in*)sa)->sin_addr) );
			u16Port = ntohs( (((struct sockaddr_in *)sa)->sin_port) );
			break;

		case AF_INET6:
			retStr = ipv6BinaryToString( (uint8_t *) & (((struct sockaddr_in6*)sa)->sin6_addr) );
			u16Port = ntohs( (((struct sockaddr_in6 *)sa)->sin6_port) );
			break;

		default:
			retStr = "Unknown AF";
            return 0;
	}

	return u16Port;
}

//============================================================================
std::string InetAddress::ipv4BinaryToString( uint8_t ipBinary[4] )
{
	char ipBuf[16];
	snprintf( ipBuf, sizeof( ipBuf ), "%d.%d.%d.%d", ipBinary[0], ipBinary[1], ipBinary[2], ipBinary[3] );
	return ipBuf;
}

//============================================================================
std::string InetAddress::ipv6BinaryToString( uint8_t ipBinary[16] )
{
	std::string ipAddr;
	std::vector<std::string> ipWords;
	bool firstQuadHasValue{ false };
	for( int i = 0; i < 16; i += 2 )
	{
		if( i < 8 && *((uint16_t*)&ipBinary[i]) )
		{
			firstQuadHasValue = true;
		}

		ipWords.emplace_back( BinaryToHexString( &ipBinary[i], 2, true ) );
	}

	int wordIdx{ 0 };
	for( auto hexStr : ipWords )
	{
		wordIdx++;
		ipAddr += removeLeadingZeros( hexStr, firstQuadHasValue && wordIdx < 4 );
		if( wordIdx < ipWords.size() )
		{
			if( ipAddr.size() < 2 || ipAddr.substr( ipAddr.size() - 2 ) != "::" )
			{
				ipAddr += ':';
			}
		}
	}

	return ipAddr;
}

//============================================================================
std::string InetAddress::removeLeadingZeros( std::string hexStr, bool leaveAtLeastOneZeroIfEmpty )
{
	int leadZeroCnt = 0;
	for( int i = 0; i < hexStr.size(); i++ )
	{
		if( hexStr.at( i ) == '0' )
		{
			leadZeroCnt++;
		}
		else
		{
			break;
		}
	}

	std::string retStr = hexStr.substr( leadZeroCnt );
	if( leaveAtLeastOneZeroIfEmpty && retStr.empty() )
	{
		retStr = "0";
	}

	return retStr;
}

//============================================================================
// InetAddrAndPort
//============================================================================

//============================================================================
InetAddrAndPort::InetAddrAndPort( const char* ipAddr )
: InetAddress( ipAddr )
, m_u16Port(0)
{
}

//============================================================================
InetAddrAndPort::InetAddrAndPort( const InetAddrAndPort& rhs )
    : InetAddress( rhs )
    , m_u16Port( rhs.m_u16Port )
{
}

//============================================================================
InetAddrAndPort::InetAddrAndPort( const InetAddress& rhs )
    : InetAddress( rhs )
    , m_u16Port( 0 )
{

}

//============================================================================
bool InetAddrAndPort::addToBlob( PktBlobEntry& blob )
{
    bool result = InetAddress::addToBlob( blob );
    result &= blob.setValue( m_u16Port );
    return result;
}

//============================================================================
bool InetAddrAndPort::extractFromBlob( PktBlobEntry& blob )
{
    bool result = InetAddress::extractFromBlob( blob );
    result &= blob.getValue( m_u16Port );
    return result;
}

//============================================================================
bool InetAddrAndPort::fromString( const char* pIpAddress )
{
	setToInvalid();
	uint16_t port = InetAddress::fromString( pIpAddress );
	if( isValid() )
	{
		if( port )
		{
			m_u16Port = port;
		}

		return true;
	}

	return false;
}

//============================================================================
std::string	InetAddrAndPort::toString( bool includePort )
{
	std::string ipAddr;
	if( isValid() )
	{
		std::string ipAddrNoPort = InetAddress::toString();
		if( ipAddrNoPort.empty() )
		{
			LogMsg( LOG_ERROR, "InetAddrAndPort::%s empty addr", __func__ );
			return ipAddr;
		}

		if( includePort && m_u16Port )
		{
			if( isIPv6() )
			{
				ipAddr = "[";
				ipAddr += ipAddrNoPort;
				ipAddr = "]:";
			}
			else
			{
				ipAddr = ipAddrNoPort;
				ipAddr += ":";
			}

			ipAddr += std::to_string( m_u16Port );
		}
		else
		{
			ipAddr = ipAddrNoPort;
		}
	}
	
	return ipAddr;
}

//============================================================================
InetAddrAndPort::InetAddrAndPort( const char* ipAddr, uint16_t port )
: InetAddress( ipAddr )
, m_u16Port( port )
{
}

//============================================================================
InetAddrAndPort& InetAddrAndPort::operator=(const InetAddress& oAddr) 
{
	if( this != &oAddr )
	{
		m_u64AddrHi = oAddr.m_u64AddrHi;
		m_u64AddrLo = oAddr.m_u64AddrLo;
	}

	return *this;
}

//============================================================================
InetAddrAndPort& InetAddrAndPort::operator=(const InetAddrAndPort& oAddr) 
{
	if( this != &oAddr )
	{
		m_u64AddrHi = oAddr.m_u64AddrHi;
		m_u64AddrLo = oAddr.m_u64AddrLo;
		m_u16Port = oAddr.m_u16Port;
	}

	return *this;
}

//============================================================================
void InetAddrAndPort::setIpAndPort( struct sockaddr_storage& inetAddr )
{
	m_u16Port = setIp( inetAddr );
}

//============================================================================
void InetAddrAndPort::setIpAndPort( struct sockaddr& oAddr )
{
	m_u16Port = setIp( oAddr );
}

//============================================================================
void InetAddrAndPort::setIpAndPort( const char* ipAddr, uint16_t port )
{
    setIp( ipAddr );
    m_u16Port = port;
}

//============================================================================
void inet_addr_testcase ( const char* pszTest )
{
    unsigned char abyAddr[16];
    bool bIsIPv6;
    uint16_t nPort;
    bool bSuccess;

    LogMsg( LOG_DEBUG, "Inet Test case '%s'", pszTest );
    const char* pszTextCursor = pszTest;
    bSuccess = ParseIPv4OrIPv6( pszTest, abyAddr, nPort, bIsIPv6 );
    if ( ! bSuccess )
    {
        LogMsg( LOG_DEBUG, "parse failed, at about index %d; rest: '%s'", pszTextCursor - pszTest, pszTextCursor );
    }
    
    LogMsg( LOG_DEBUG, "addr:  %s", BinaryToHexString( abyAddr, bIsIPv6 ? 16 : 4 ).c_str() );

    if ( 0 == nPort )
        LogMsg( LOG_DEBUG, "port absent" );
    else
        LogMsg( LOG_DEBUG, "port:  %d", htons ( nPort ) );
    LogMsg( LOG_DEBUG, "\n" );
    
}

//============================================================================
void inet_to_from_testcase( std::string ipAddr )
{
	InetAddress inetAddr;
	inetAddr.setIp( ipAddr.c_str() );
	std::string ipResult1 = inetAddr.toString();
	if( ipResult1 != ipAddr )
	{
		LogMsg( LOG_DEBUG, "inet_to_from_testcase 1 failed %s %s", ipAddr.c_str(), ipResult1.c_str() );
	}

	struct sockaddr_storage addrStorage;
	inetAddr.fillAddress( addrStorage, 9 );
	struct sockaddr* sktAddr = reinterpret_cast<sockaddr*>(&addrStorage);
	inetAddr.setIp( *sktAddr );
	std::string ipResult2 = inetAddr.toString();
	if( ipResult2 != ipAddr )
	{
		LogMsg( LOG_DEBUG, "inet_to_from_testcase 2 failed %s %s", ipAddr.c_str(), ipResult2.c_str() );
	}

	if( !inetAddr.isValid() )
	{
		LogMsg( LOG_DEBUG, "inet_to_from_testcase 3 failed not valid ip %s", ipAddr.c_str() );
	}

	if( ipAddr == "::1" && !inetAddr.isLoopBack() )
	{
		LogMsg( LOG_DEBUG, "inet_to_from_testcase 4 failed is loopback ip %s", ipAddr.c_str() );
	}
}

//============================================================================
void TestInetAddress( void )
{  
	inet_to_from_testcase("2a07:85c0:0:122::");
	inet_to_from_testcase("::1");

    //The "localhost" IPv4 address
    inet_addr_testcase ( "127.0.0.1" );

	inet_addr_testcase ( "0000:0000:0000:0000:0000:0000:0000:0001/128" );
    
    //The "localhost" IPv4 address, with a specified port (80)
    inet_addr_testcase ( "127.0.0.1:80" );
    //The "localhost" IPv6 address
    inet_addr_testcase ( "::1" );
    //The "localhost" IPv6 address, with a specified port (80)
    inet_addr_testcase ( "[::1]:80" );
    //Rosetta Code's primary server's public IPv6 address
    inet_addr_testcase ( "2605:2700:0:3::4713:93e3" );
    //Rosetta Code's primary server's public IPv6 address, with a specified port (80)
    inet_addr_testcase ( "[2605:2700:0:3::4713:93e3]:80" );
    
    //ipv4 space
    inet_addr_testcase ( "::ffff:192.168.173.22" );
    //ipv4 space with port
    inet_addr_testcase ( "[::ffff:192.168.173.22]:80" );
    //trailing compression
    inet_addr_testcase ( "1::" );
    //trailing compression with port
    inet_addr_testcase ( "[1::]:80" );
    //'any' address compression
    inet_addr_testcase ( "::" );
    //'any' address compression with port
    inet_addr_testcase ( "[::]:80" );

    inet_addr_testcase( "2604:980:7003:e6:29e::1" );
    inet_addr_testcase( "[2604:980:7003:e6:29e::1]:45124" );
    

}
