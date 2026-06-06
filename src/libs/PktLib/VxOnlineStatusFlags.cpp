//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxOnlineStatusFlags.h"
#include <CoreLib/PktBlobEntry.h>

#define ONLINE_STATUS_AUTOMATED_HOST		0x01
#define ONLINE_STATUS_FROM_SEARCH_PKT		0x02 // is from search list.. never connected to
#define ONLINE_STATUS_HAS_TEXT_OFFERS		0x04 

//============================================================================
VxOnlineStatusFlags::VxOnlineStatusFlags( const VxOnlineStatusFlags& rhs )
    : m_u8OnlineStatusFlags( rhs.m_u8OnlineStatusFlags )
{
}

//============================================================================
VxOnlineStatusFlags& VxOnlineStatusFlags::operator =( const VxOnlineStatusFlags& rhs )
{
    if( this != &rhs )
    {
        m_u8OnlineStatusFlags = rhs.m_u8OnlineStatusFlags;
    }

    return *this;
}

//============================================================================
bool VxOnlineStatusFlags::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_u8OnlineStatusFlags );
}

//============================================================================
bool VxOnlineStatusFlags::extractFromBlob( PktBlobEntry& blob )
{
    return blob.getValue( m_u8OnlineStatusFlags );
}

//============================================================================
void		VxOnlineStatusFlags::setIsAutomatedHost( bool bIsFromSearch )	{ if( bIsFromSearch )(m_u8OnlineStatusFlags |= ONLINE_STATUS_AUTOMATED_HOST); else m_u8OnlineStatusFlags &= (~ONLINE_STATUS_AUTOMATED_HOST); }
bool		VxOnlineStatusFlags::isAutomatedHost( void )					{ return (m_u8OnlineStatusFlags & ONLINE_STATUS_AUTOMATED_HOST)?1:0; }

//============================================================================
void		VxOnlineStatusFlags::setHasTextOffers( bool hasOffers )			{ if( hasOffers )(m_u8OnlineStatusFlags |= ONLINE_STATUS_HAS_TEXT_OFFERS); else m_u8OnlineStatusFlags &= (~ONLINE_STATUS_HAS_TEXT_OFFERS); }
bool		VxOnlineStatusFlags::getHasTextOffers( void )					{ return (m_u8OnlineStatusFlags & ONLINE_STATUS_HAS_TEXT_OFFERS)?1:0; }

//============================================================================
void		VxOnlineStatusFlags::setIsFromSearchPkt( bool bIsFromSearch )	{ if( bIsFromSearch )(m_u8OnlineStatusFlags |= ONLINE_STATUS_FROM_SEARCH_PKT); else m_u8OnlineStatusFlags &= (~ONLINE_STATUS_FROM_SEARCH_PKT); }
bool		VxOnlineStatusFlags::isFromSearchPkt( void )					{ return (m_u8OnlineStatusFlags & ONLINE_STATUS_FROM_SEARCH_PKT)?1:0; }

