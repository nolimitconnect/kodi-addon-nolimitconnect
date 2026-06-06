//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxFriendMatch.h"
#include <CoreLib/PktBlobEntry.h>

//============================================================================
FriendMatch::FriendMatch( const FriendMatch& rhs ) 
    : m_u8FriendMatch( rhs.m_u8FriendMatch )
{
}

//============================================================================
FriendMatch& FriendMatch::operator =( const FriendMatch& rhs )
{
    if( this != &rhs )
    {
        m_u8FriendMatch = rhs.m_u8FriendMatch;
    }

    return *this;
}

//============================================================================
bool FriendMatch::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_u8FriendMatch );
}

//============================================================================
bool FriendMatch::extractFromBlob( PktBlobEntry& blob )
{
    return blob.getValue( m_u8FriendMatch );
}
