//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxConnectId.h"

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>

#include <memory.h>

//============================================================================
VxConnectId::VxConnectId( const VxConnectId &rhs )
: VxGUID(rhs)
, m_OnlineIp( rhs.m_OnlineIp )
, m_u16OnlinePort( rhs.m_u16OnlinePort )
, m_IpAddrType( rhs.m_IpAddrType )
, m_u8Reseved1( rhs.m_u8Reseved1 )
, m_u32Reseved2( rhs.m_u32Reseved2 )
{
}

//============================================================================
bool VxConnectId::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( *( (VxGUID *)this ) );
    result &= m_OnlineIp.addToBlob( blob );
    result &= blob.setValue( m_u16OnlinePort );
	result &= blob.setValue( m_IpAddrType );
	result &= blob.setValue( m_u8Reseved1 );
	result &= blob.setValue( m_u32Reseved2 );
    return result;
}

//============================================================================
bool VxConnectId::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( *( (VxGUID *)this ) );
    result &= m_OnlineIp.extractFromBlob( blob );
    result &= blob.getValue( m_u16OnlinePort );
	result &= blob.getValue( m_IpAddrType );
	result &= blob.getValue( m_u8Reseved1 );
	result &= blob.getValue( m_u32Reseved2 );
    return result;
}

//============================================================================
VxConnectId& VxConnectId::operator =( const VxConnectId &rhs )
{
	if( this != &rhs )
	{
        *((VxGUID*)this) = *((VxGUID*)&rhs);
        m_OnlineIp = rhs.m_OnlineIp;
        m_u16OnlinePort = rhs.m_u16OnlinePort;
		m_IpAddrType = rhs.m_IpAddrType;
		m_u8Reseved1 = rhs.m_u8Reseved1;
		m_u32Reseved2 = rhs.m_u32Reseved2;
	}

	return *this;
}

//============================================================================
bool VxConnectId::operator ==( const VxConnectId &rhs )  const
{
	return ( 0 == memcmp( this, &rhs, sizeof( VxConnectId )) );
}

//============================================================================
bool VxConnectId::operator !=( const VxConnectId &rhs )  const
{
	return ( 0 != memcmp( this, &rhs, sizeof( VxConnectId )) );
}

//============================================================================
bool VxConnectId::setIpAddress( std::string ipAddr, bool* retIpHasChanged  )		
{
	if( ipAddr.empty() )
	{
		LogMsg( LOG_ERROR, "VxConnectId::%s: bad param empty ipAddr", __func__ );
		return false;
	}

	EIpAddrType addrType = VxGetIpAddrType( ipAddr.c_str() );
	if( addrType == eIpAddrTypeUnknown )
	{
		LogMsg( LOG_ERROR, "VxConnectId::%s: bad param ipAddr %s", __func__, ipAddr.c_str() );
		return false;
	}

	if( retIpHasChanged )
	{
		*retIpHasChanged = false;
	}

	EIpAddrType curAddrType{ eIpAddrTypeUnknown };
	std::string curIp;
	if( getIpAddress( curIp, curAddrType ) )
	{
		if( curAddrType == addrType && curIp == ipAddr )
		{
			// no change
			return true;
		}
	}

	m_OnlineIp.fromString( ipAddr.c_str() );
	bool valid = m_OnlineIp.isValid();
	setIpAddressType( addrType );

	vx_assert( valid );
    return valid;
};

//============================================================================
bool VxConnectId::setIpAddress( InetAddress& ipAddr )
{
	m_OnlineIp = ipAddr;
	m_IpAddrType = m_OnlineIp.getIpAddrType();
	return m_OnlineIp.isValid();
}

//============================================================================
bool VxConnectId::getIpAddress( std::string& retString, EIpAddrType& retIpAddrType )
{
	bool result = false;
	EIpAddrType addrType = m_OnlineIp.getIpAddrType();
	if( addrType != eIpAddrTypeUnknown )
	{
		retString = m_OnlineIp.toString();
		retIpAddrType = addrType;
		return true;
	}

	retString.clear();
	retIpAddrType = eIpAddrTypeUnknown;
	return false;
};

//============================================================================
void VxConnectId::clear( void )
{
	clearVxGUID();
	m_u16OnlinePort = 0;
	m_u8Reseved1 = 0;
	m_u32Reseved2 = 0;
	m_OnlineIp.setToInvalid();
	m_IpAddrType = 0;
}

//============================================================================
void VxConnectId::clearIpAddress( void )
{
	m_OnlineIp.setToInvalid();
	m_IpAddrType = 0;
}