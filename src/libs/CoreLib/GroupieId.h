#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostedId.h"

class PktBlobEntry;

#pragma pack(push)
#pragma pack(1)

class GroupieId // size 16 + 17 = 33
{
public:
	GroupieId() = default;
    GroupieId( VxGUID& userOnlineId, VxGUID& hostOnlineId, EHostType hostType );
    GroupieId( VxGUID& userOnlineId, HostedId& hostedId );
    GroupieId( const GroupieId& rhs );
    //do not use virtuals because this object is packed in packets
	GroupieId&				    operator =( const GroupieId& rhs );
    bool						operator == ( const GroupieId& rhs ) const;
    bool						operator != ( const GroupieId& rhs ) const;
    bool						operator < ( const GroupieId& rhs ) const;
    bool						operator <= ( const GroupieId& rhs ) const;
    bool						operator > ( const GroupieId& rhs ) const;
    bool						operator >= ( const GroupieId& rhs ) const;

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    void						setUserOnlineId( VxGUID& onlineId )                 { m_UserOnlineId = onlineId; }
    VxGUID&					    getUserOnlineId( void )                             { return m_UserOnlineId; }
    VxGUID					    getUserOnlineId( void ) const                       { return m_UserOnlineId; }
    void					    getUserOnlineId( VxGUID& onlineId ) const           { onlineId = m_UserOnlineId; }
    bool						isUserOnlineId( VxGUID& onlineId ) const            { return m_UserOnlineId == onlineId; }  

    void				        setHostedId( HostedId& hostedId )                   { m_HostedId = hostedId; }
    HostedId&                   getHostedId( void )                                 { return m_HostedId; }
    bool                        isHostedIdValid( void )                             { return m_HostedId.isValid(); }
    
    void				        setHostOnlineId( VxGUID& onlineId )                 { m_HostedId.setHostOnlineId( onlineId ); }
    VxGUID&                     getHostOnlineId( void )                             { return m_HostedId.getHostOnlineId(); }
    bool						isHostOnlineId( VxGUID& onlineId ) const            { return m_HostedId.isHostOnlineId( onlineId ); }

    void			            setHostType( EHostType hostType )                   { m_HostedId.setHostType( hostType ); }
    EHostType	                getHostType( void ) const                           { return m_HostedId.getHostType(); }
    bool						isHostType( EHostType hostType ) const              { return m_HostedId.isHostType( hostType ); }

    EPluginType                 getHostPluginType( void );
    EPluginType                 getClientPluginType( void );

    // returns 0 if equal else -1 if less or 1 if greater
    int							compareTo( GroupieId& guid );
    // returns true if guids are same value
    bool						isEqualTo( const GroupieId& guid );
    // get a description of the plugin id
    std::string                 describeGroupieId( void ) const;

    bool                        isValid( void )                                 { return m_UserOnlineId.isValid() && m_HostedId.isValid();  }
    void                        clear( void )                                   { m_UserOnlineId.clearVxGUID(); m_HostedId.clear(); }

protected:
	//=== vars ===//
    VxGUID					    m_UserOnlineId;
    HostedId					m_HostedId;
};

#pragma pack(pop)
