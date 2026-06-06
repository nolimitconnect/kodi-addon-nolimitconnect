#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <stdint.h> 

#include <string>
#include <vector>

enum EIpAddrType
{
	eIpAddrTypeUnknown,
	eIpAddrTypeIpv4,
	eIpAddrTypeIpv6
};

#pragma pack(push)
#pragma pack(1)

class InetAddress;
class InetAddrAndPort;
class PktBlobEntry;

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_storage;

//============================================================================
// size 16 bytes
// contains internet address in network order. can be IPv4 or IPv6
class InetAddress
{
public:
	InetAddress() = default;	
	InetAddress( const char* pIpAddress );
	InetAddress( uint32_t u32IpAddr );
    InetAddress( const InetAddress& rhs );

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	// set ip from string.. returns port if is part of addr.. example 1.1.1.1:80 returns 80
	uint16_t                    fromString( const char* ipAddress );
	std::string					toString( void ) const;
	EIpAddrType					getIpAddrType( void );

    InetAddress&                operator=(const InetAddress& inetAddr);
	bool                        operator == (const InetAddress& inetAddr) const;
    bool                        operator != (const InetAddress& inetAddr) const;

    bool                        isValid( void ) const;
    void                        setToInvalid( void );
    bool                        isIPv4( void ) const;
	bool                        isIPv6( void ) const;
    bool                        isLoopBack( void ) const;
	bool					    isLocalAddress( bool forLocalListening = false ) const;
	bool                        isIPv6GlobalAddress( void ) const;

	uint16_t                    setIp( const char* ipAddress );
	// note.. ipv4Addr must be in network order
    void                        setIp( uint32_t ipv4Addr );
	//! returns port in host order
    uint16_t                    setIp( struct sockaddr_storage& ipAddr );
	//! returns port in host order
    uint16_t                    setIp( struct sockaddr& ipAddr );
	
	//! fill address with this ip address and the given port.. returns struct len
    int                         fillAddress( struct sockaddr_storage& addrStorage, uint16_t port );

    uint32_t                    getIPv4AddressInHostOrder( void ) const;
    uint32_t                    getIPv4AddressInNetOrder( void ) const;

	//! returns port in host order
	static uint16_t				getIpFromAddr(const struct sockaddr *sa, std::string& retStr);
	static InetAddress		    getDefaultIp( void );
    static int				    getAllAddresses( std::vector<InetAddress>& retAddress );
    static void                 dumpAddresses( std::vector<InetAddress>& addressList );

	static std::string			ipv4BinaryToString( uint8_t ipBinary[4] );
	static std::string			ipv6BinaryToString( uint8_t ipBinary[16] );
	static std::string			removeLeadingZeros( std::string hexStr, bool leaveAtLeastOneZeroIfEmpty );
protected:
	//! returns port in host order
    uint16_t                    setIp( struct sockaddr_in& ipv4Addr );
	//! returns port in host order
    uint16_t                    setIp( struct sockaddr_in6& ipv6Addr );
	//! fill address with this ip address and the given port.. returns struct len
    int                         fillAddress( struct sockaddr_in& ipv4Addr, uint16_t port );
	//! fill address with this ip address and the given port.. returns struct len
    int                         fillAddress( struct sockaddr_in6& ipv6Addr, uint16_t port );

    bool                        isIPv4String( const char* pIpAddress ) const;

public:
	uint64_t					m_u64AddrHi{ 0 };
	uint64_t					m_u64AddrLo{ 0 };
};

//============================================================================
class InetAddrAndPort : public InetAddress
{
public:
    InetAddrAndPort() = default;
	InetAddrAndPort( const char* ipAddr );
	InetAddrAndPort( const char* ipAddr, uint16_t port );
    InetAddrAndPort( const InetAddrAndPort& rhs );
    InetAddrAndPort( const InetAddress& rhs );

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	// set ip from string
	bool                        fromString( const char* ipAddress );
	std::string					toString( bool includePort = false );

	InetAddrAndPort&            operator=(const InetAddress& inetAddr);
	InetAddrAndPort&            operator=(const InetAddrAndPort& inetAddr);

    uint16_t                    getPort( void )	const			                    { return m_u16Port; }
    void                        setPort( uint16_t u16Port )			                { m_u16Port = u16Port; }

    void                        setIpAndPort( struct sockaddr_storage& storageAddr );
	void                        setIpAndPort( struct sockaddr& sockAddr );
    void                        setIpAndPort( const char* ipAddr, uint16_t port );

    // these shadow InetAddress on purpose to avoid issues with overrides changing the packet size that is sent over internet
	uint16_t                    setIp( const char* ipAddress )                              { return InetAddress::setIp( ipAddress ); };
	// ipv4Addr must be in network order
    void                        setIp( uint32_t ipv4Addr )									{ return InetAddress::setIp( ipv4Addr ); };
    //! returns port in host order
    uint16_t                    setIp( struct sockaddr_storage& ipAddr )                    { return InetAddress::setIp( ipAddr ); };
    //! returns port in host order
    uint16_t                    setIp( struct sockaddr& ipAddr )                            { return InetAddress::setIp( ipAddr ); };
    //! returns port in host order
    uint16_t                    setIp( struct sockaddr_in& ipv4Addr )                       { m_u16Port = InetAddress::setIp( ipv4Addr ); return m_u16Port; };
    //! returns port in host order
    uint16_t                    setIp( struct sockaddr_in6& ipv6Addr )                      { m_u16Port = InetAddress::setIp( ipv6Addr ); return m_u16Port; };


	//=== vars ===//
    uint16_t                    m_u16Port{ 0 };
};

#pragma pack(pop)

void TestInetAddress( void );