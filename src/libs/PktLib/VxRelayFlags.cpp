//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "VxRelayFlags.h"
#include <CoreLib/PktBlobEntry.h>

#define RC_PROXY_FLAG_REQUIRES_PROXY			0x01	// user requires proxy to connect to him
#define RC_PROXY_FLAG_HAS_PROXY					0x02	// user has at least one proxy that may be used to connect to him

//============================================================================
VxRelayFlags::VxRelayFlags( const VxRelayFlags& rhs )
: m_u8RelayFlags( rhs.m_u8RelayFlags )
{
}

//============================================================================
VxRelayFlags& VxRelayFlags::operator =( const VxRelayFlags& rhs )
{
    if( this != &rhs )
    {
        m_u8RelayFlags = rhs.m_u8RelayFlags;
    }

    return *this;
}

//============================================================================
bool VxRelayFlags::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_u8RelayFlags );
}

//============================================================================
bool VxRelayFlags::extractFromBlob( PktBlobEntry& blob )
{
    return blob.getValue( m_u8RelayFlags );
}

//============================================================================
void VxRelayFlags::setRelayFlags( uint8_t u8RelayFlags )
{
	m_u8RelayFlags = u8RelayFlags;
}

//============================================================================
uint8_t VxRelayFlags::getRelayFlags( void )
{
	return m_u8RelayFlags;
}

//============================================================================

bool VxRelayFlags::hasRelay( void )						
{ 
	return (RC_PROXY_FLAG_HAS_PROXY & m_u8RelayFlags)?1:0; 
}

//============================================================================
void VxRelayFlags::setHasRelay( bool bHasRelay )				
{
	if( bHasRelay )
		m_u8RelayFlags |= RC_PROXY_FLAG_HAS_PROXY;
	else
		m_u8RelayFlags &= ~RC_PROXY_FLAG_HAS_PROXY;
}

//============================================================================
bool VxRelayFlags::requiresRelay( void )					
{ 
	return (RC_PROXY_FLAG_REQUIRES_PROXY & m_u8RelayFlags) ? true : false; 
}

//============================================================================
void VxRelayFlags::setRequiresRelay( bool bRequiresRelay )				
{
	if( bRequiresRelay )
		m_u8RelayFlags |= RC_PROXY_FLAG_REQUIRES_PROXY;
	else
		m_u8RelayFlags &= ~RC_PROXY_FLAG_REQUIRES_PROXY;
}
