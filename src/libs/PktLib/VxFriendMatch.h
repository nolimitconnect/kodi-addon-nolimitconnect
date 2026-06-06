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

#include <CoreLib/VxDefs.h>
#include <GuiInterface/IDefs.h>

#include <string>

#pragma pack(push)
#pragma pack(1)
class PktBlobEntry;

class FriendMatch
{
public:
	FriendMatch() = default;
    FriendMatch( const FriendMatch& rhs );
    FriendMatch&                operator =( const FriendMatch& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	//=== vars ===//
    uint8_t						m_u8FriendMatch{ 0x11 };
};

#pragma pack(pop)

