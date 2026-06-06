#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/ConnectId.h>

#include <CoreLib/VxGUID.h>

class PktBlobEntry;

#pragma pack(push)
#pragma pack(1)

class HostUserSessionId : public ConnectId
{
public:
    HostUserSessionId() = default;
    HostUserSessionId( VxGUID& socketId );
    HostUserSessionId( VxGUID& socketId, GroupieId& groupieId );
    HostUserSessionId( VxGUID& socketId, VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType );
    HostUserSessionId( VxGUID& userOnlineId, VxGUID& hostOnlineId, EHostType hostType );
    HostUserSessionId( VxGUID& userOnlineId, HostedId& hostedId );
    HostUserSessionId( VxGUID& socketId, GroupieId& groupieId, VxGUID& sessionId );
    HostUserSessionId( const HostUserSessionId& rhs );
    //do not use virtuals because this object is packed in packets
    HostUserSessionId&		    operator =( const HostUserSessionId& rhs );
    bool						operator == ( const HostUserSessionId& rhs ) const;
    bool						operator != ( const HostUserSessionId& rhs ) const;
    bool						operator < ( const HostUserSessionId& rhs ) const;
    bool						operator <= ( const HostUserSessionId& rhs ) const;
    bool						operator > ( const HostUserSessionId& rhs ) const;
    bool						operator >= ( const HostUserSessionId& rhs ) const;

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    void						setSessionId( VxGUID& socketId )                     { m_SessionId = socketId; }
    VxGUID&                     getSessionId( void )                                 { return m_SessionId; }

    // returns 0 if equal else -1 if less or 1 if greater
    int							compareTo( HostUserSessionId& guid );
    // returns true if guids are same value
    bool						isEqualTo( const HostUserSessionId& guid );
    // get a description of the plugin id
    std::string                 describeHostUserSessionId( void ) const;

    bool                        isValid( void )                                 { return m_SessionId.isValid() && ConnectId::isValid(); }
    void                        clear( void )                                   { m_SessionId.clearVxGUID(); ConnectId::clear(); }

protected:
	//=== vars ===//
    VxGUID					    m_SessionId;
};

#pragma pack(pop)
