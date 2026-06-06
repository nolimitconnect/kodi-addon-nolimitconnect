#pragma once
//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#define EIM_ALIVE_TIMEDOUT          20000

#define WEBSITE_CONNECT_TIMEOUT		12000
#define NETSERVICE_CONNECT_TIMEOUT	15000
#define NETSERVICE_IS_PORT_OPEN_TXRX_TIMEOUT	25000

#define SKT_CONNECT_TIMEOUT			6000
#define SKT_IPV6_CONNECT_TIMEOUT	4000
#define SKT_SEND_TIMEOUT			3000
#define SKT_RECEIVE_TIMEOUT			3000
#define ROUTER_RECIEVE_TIMEOUT		8000

// error codes
#define VX_NO_HOST_IPS_FOUND	    -1
#define VX_INVALID_SOCK_ADDRESS     -2

#define VX_MAX_HOST_IPS             10 // maximum host ips returned by VxGetLocalIps

const int INET6_MAX_STR_LEN = 68 + 8;  // plus 8 in case port is appended and ip is in brackets
const int INET6_MAX_BINARY_LEN = 16;   // 16 bytes

#include "VxDefs.h"

#ifdef __cplusplus

#include "InetAddress.h"

class ISktStatCallbackInterface;

//! initialize sockets
int32_t							VxSocketsStartup( void );

bool                            VxIsPortValid( uint16_t port );

EIpAddrType						VxGetIpAddrType( const char* ipAddr );

bool                            VxIsIpValid( std::string& ipAddr );
bool                            VxIsIpv6Address( std::string& ipAddr );
bool							VxIsIpv4Address( std::string& ipAddr );

bool							VxMakePtopUrl( std::string& ipAddr, uint16_t port, std::string& retPtopUrl );
bool							VxResolvePtopUrl( std::string ptopUrl, std::string& retIpAddr, uint16_t& retPort, bool preferIpv6 = false );

std::string						VxGetRemoteIpAddress( SOCKET skt );
std::string						VxGetRmtHostName( SOCKET& skt );
//! split host name from website file path
bool							VxSplitHostAndFile( const char* pFullUrl,			// full url.. example http://www.mysite.com/index.html or www.mysite.com/images/me.png
													std::string& strRetHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
													std::string& strRetFileName,	// return file name.. images/me.png
													uint16_t& u16RetPort );			// return port if specified else return 80 as default	
void							VxIpInNetOrderToString( uint32_t u32IpAddr, std::string& retIp );
uint32_t						VxStringToIpInNetOrder( std::string ip );
std::string						VxIpToString( struct sockaddr * addr );
void							VxFillHints( struct addrinfo& retHints, EIpAddrType addrType );
void							VxGetLocalIps( std::vector<InetAddress>& aRetIpAddress );

InetAddress						VxGetSelectedLocalIp( void );
InetAddress						VxGetMyGlobalIPv6Address( void );
InetAddress						VxGetDefaultIPv4Address( void );
InetAddress						VxGetDefaultIPv6Address( void );
bool							VxTestConnectionOnSpecificLclAddress( InetAddress &oLclAddr );
bool							VxResolveUrl( const char* pUrl, uint16_t u16Port, InetAddress& oRetAddr, EIpAddrType addrType );
bool							VxResolveUrl( std::string& urlIn, uint16_t& retPort, std::string& retIpAddr, EIpAddrType addrType );
bool							VxResolveUrl( const char* pUrl, uint16_t u16Port, std::string& resolvedIp, EIpAddrType addrType ); // assumes pUrl is just host name

//! return true if ip is in list of local ips
bool							VxLocalIpExists( std::string& strIpAddress );
// connects to website and returns socket.. if fails returns INVALID_SOCKET
SOCKET							VxConnectToWebsite( InetAddrAndPort&	oLclIp,			// ip of adapter to use
													InetAddrAndPort&	oRmtIp,			// return ip and port url resolves to
													const char*			pWebsiteUrl,
													std::string&		strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
													std::string&		strFile,		// return file name.. images/me.png
                                                    uint16_t&			u16Port,		// return port
													EIpAddrType			addrType,
													int					iConnectTimeoutMs,
													int32_t*				retErrorCode = nullptr );

bool							VxBindSkt( SOCKET sktHandle, InetAddress & oLclAddr, uint16_t u16Port );
SOCKET							VxConnectTo( InetAddrAndPort& oLclIp, InetAddrAndPort& oRmtIp, uint16_t u16Port, int iTimeoutMs = SKT_CONNECT_TIMEOUT, int32_t * retSktErr = nullptr );
SOCKET							VxConnectTo( InetAddrAndPort&	oLclIp,
											 InetAddrAndPort&	oRmtIp,
											 const char*		pIpOrUrl,				// remote ip or url
                                             uint16_t			u16Port,				// port to connect to
											 EIpAddrType		addrType,
                                             int				iTimeoutMilliSeconds,	// milli seconds before connect attempt times out
                                             int32_t*			    retSktErr = 0 );		// return connect error if retSktErr is not null
SOCKET                          VxConnectToAddr(SOCKET sktHandle, struct sockaddr* sktAddr, socklen_t sktAddrLen, int iConnectTimeoutMs = SKT_CONNECT_TIMEOUT, int32_t * retSktErr = nullptr);
std::string                     VxSktAddrToString( struct sockaddr* sktAddr, int sktAddrLen, bool includePort = true );

int32_t							VxGetLclAddress( SOCKET sktHandle, InetAddrAndPort& oRetAddr );
std::string						VxGetLclIpAddress( SOCKET sktHandle, uint16_t *	retPort = nullptr );
int32_t							VxGetRmtAddress( SOCKET sktHandle, InetAddrAndPort& oRetAddr, bool isSimpleSkt = false );

bool							VxIsIPv6Address( const char*addr );
bool							VxIsIPv4Address( const char*addr, bool checkNoLocal = false );
int								VxGetIPv6ScopeID( const char*addr );

void							VxRefreshDefaultIps( void );
bool							VxCanConnectUsingIPv6( void );
SOCKET							VxConnectToIPv6( const char* ipv6, uint16_t u16Port, int iTimeoutMs = SKT_IPV6_CONNECT_TIMEOUT, int32_t * retSktErr = nullptr );
//! receive data.. if timeout is set then will keep trying till buffer is full or error or timeout expires
int32_t							VxReceiveSktData( SOCKET& oSkt,
												  char* pRetBuf,				// buffer to receive data into
												  int	iBufLenIn,				// length of buffer
												  int*  iRetBytesReceived,		// number of bytes actually received
												  int	iTimeoutMilliSeconds = SKT_RECEIVE_TIMEOUT );	// milliseconds before receive attempt times out ( 0 = do not wait )

bool							VxBindSkt( SOCKET oSocket, struct sockaddr_storage * poAddr );
bool							VxIsIpPortInUse( uint16_t u16Port, const char* pLocalIp = nullptr, bool useBind = false);

bool							VxMakeBroadcastIp( std::string localIp, std::string& retBroadcastIp );

void							VxSetSktStatCallback( ISktStatCallbackInterface* sktStatCallback );
ISktStatCallbackInterface*		VxGetSktStatCallback( void );


socklen_t						VxSktAddrInit( bool ipv6, struct sockaddr_storage& sockAddr, uint16_t sktPort = 0 );
socklen_t						VxSktAddrInit( bool ipv6, struct sockaddr_storage& sockAddr, std::string ipAddr, uint16_t sktPort );
bool							VxSktAddrGetParams( bool ipv6, struct sockaddr_storage& sockAddr, std::string& retIp, uint16_t& retPort );

bool							VxGetDefaultLocalIp( bool ipv6, std::string& retLocalIp );

#endif // __cplusplus

//============================================================================
// C functions
//============================================================================
NLC_BEGIN_CDECLARES

int32_t							VxSetSktBlocking( SOCKET sktHandle, bool bBlock );

void							VxCloseSktNow( SOCKET& oSocket );
void							VxCloseSkt( SOCKET& oSocket );

socklen_t						VxGetSktAddressLength( struct sockaddr_storage * poAddr );
void							VxSetSktAddressPort( struct sockaddr_storage * poAddr, uint16_t u16Port );
int32_t							VxSendSktData( SOCKET			oSkt,
											   const char*		pData,					// data to send
											   int				iDataLen,				// length of data
											   int				iTimeoutSeconds ); // = SKT_SEND_TIMEOUT // seconds before send attempt times out

bool							VxSetSktAllowReuseAddress( SOCKET skt );

uint16_t						VxGetRmtPort( SOCKET skt );

const char*						VxDescribeSktError( int iErr );

bool							VxIsFatalSktError( int iErr );

NLC_END_CDECLARES
