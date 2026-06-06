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

#include <inttypes.h>

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;
//! 1 byte in size
class VxRelayFlags
{
public:
	VxRelayFlags() = default;
    VxRelayFlags( const VxRelayFlags& rhs );
    VxRelayFlags&               operator =( const VxRelayFlags& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	void						setRelayFlags( uint8_t u8RelayFlags );
	uint8_t						getRelayFlags( void );

	void						setHasRelay( bool bHasRelay );
	bool						hasRelay( void );

	void						setRequiresRelay( bool bRequiresRelay );
	bool						requiresRelay( void );

	//=== vars ===//
    uint8_t						m_u8RelayFlags{ 0 };
};

#pragma pack(pop)
