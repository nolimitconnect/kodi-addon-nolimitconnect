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

#include <CoreLib/VxGUID.h>

#include <CoreLib/InetAddress.h>

#include <string>

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;

//! derived from 16 bytes
//! size 
//	  16 bytes VxGUID
// +  16 bytes m_IPOnlineIp
// +   2 bytes m_u16OnlinePort
// +   2 bytes m_u16ResConnectId1
// +   2 bytes m_u32ResConnectId2
// =  40 bytes total
class VxConnectId : public VxGUID
{
public:
	VxConnectId() = default;
	VxConnectId( const VxConnectId& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	VxConnectId&                operator = ( const VxConnectId& rhs );
	bool                        operator != (const VxConnectId& rhs ) const;
	bool                        operator == (const VxConnectId& rhs ) const;

	void						setPort( uint16_t port )			{ m_u16OnlinePort = htons( port ); }
	uint16_t					getPort( void )						{ return ntohs( m_u16OnlinePort ); }

	bool						setIpAddress( std::string ipAddr, bool* retIpHasChanged = nullptr ); 
	bool						getIpAddress( std::string& retIpAddr, EIpAddrType& retIpAddrType );

	bool						setIpAddress( InetAddress& ipAddr );
	InetAddress&				getIpAddress( void )						{ return m_OnlineIp; }

	void						setIpAddressType( EIpAddrType addrType )	{ m_IpAddrType = (uint8_t)addrType; }
	EIpAddrType					getIpAddressType( void )					{ return (EIpAddrType)m_IpAddrType; }

	bool						isIpAddressValid( void )			{ return m_IpAddrType != eIpAddrTypeUnknown; }

	void						clear( void );
	void						clearIpAddress( void );

	VxGUID&						getOnlineId( void )					{ return *this; }

	//=== vars ===//
	InetAddress					m_OnlineIp;							// users ip address

protected:
	uint16_t					m_u16OnlinePort{ 0 };				// users incoming connection port
	uint8_t						m_IpAddrType{ 0 };
	uint8_t						m_u8Reseved1{ 0 };					// reserved for possible future use
	uint32_t					m_u32Reseved2{ 0 };					// reserved for possible future use
};

#pragma pack(pop)

