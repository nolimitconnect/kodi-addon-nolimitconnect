#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <inttypes.h>

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;
//! 1 byte in size
class VxOnlineStatusFlags
{
public:
	VxOnlineStatusFlags() = default;
    VxOnlineStatusFlags( const VxOnlineStatusFlags &rhs );
    VxOnlineStatusFlags&        operator =( const VxOnlineStatusFlags &rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	void						setIsAutomatedHost( bool bIsNearby );
	bool						isAutomatedHost( void );

	void						setHasTextOffers( bool hasOffers );
	bool						getHasTextOffers( void );

	void						setIsFromSearchPkt( bool bIsFromSearch );
	bool						isFromSearchPkt( void );

	//=== vars ===//
    uint8_t						m_u8OnlineStatusFlags{ 0 };
};

#pragma pack(pop)


