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

#include <CoreLib/GroupieId.h>

class PktBlobEntry;

#pragma pack(push)
#pragma pack(1)

class ConnectId
{
public:
	ConnectId() = default;
    ConnectId( VxGUID& socketId );
    ConnectId( VxGUID& socketId, GroupieId& groupieId );
    ConnectId( VxGUID& socketId, VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType );
    ConnectId( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, EHostType hostType );
    ConnectId( VxGUID& groupieOnlineId, HostedId& hostedId );
    ConnectId( const ConnectId& rhs );
    //do not use virtuals because this object is packed in packets
	ConnectId&				    operator =( const ConnectId& rhs );
    bool						operator == ( const ConnectId& rhs ) const;
    bool						operator != ( const ConnectId& rhs ) const;
    bool						operator < ( const ConnectId& rhs ) const;
    bool						operator <= ( const ConnectId& rhs ) const;
    bool						operator > ( const ConnectId& rhs ) const;
    bool						operator >= ( const ConnectId& rhs ) const;

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    void						setSocketId( VxGUID& socketId )                     { m_SocketId = socketId; }
    VxGUID                      getSocketId( void ) const                           { return m_SocketId; }

    void						setGroupieId( GroupieId& groupieId )                { m_GroupieId = groupieId; }
    GroupieId&                  getGroupieId( void )                                { return m_GroupieId; }

    void						setUserOnlineId( VxGUID& onlineId )                 { m_GroupieId.setUserOnlineId( onlineId ); }
    VxGUID&					    getUserOnlineId( void )                             { return m_GroupieId.getUserOnlineId(); }

    void				        setHostedId( HostedId& hostedId )                   { m_GroupieId.setHostedId( hostedId ); }
    HostedId&                   getHostedId( void )                                 { return m_GroupieId.getHostedId(); }
    void				        setHostOnlineId( VxGUID& onlineId )                 { m_GroupieId.setHostOnlineId( onlineId ); }
    VxGUID&                     getHostOnlineId( void )                             { return m_GroupieId.getHostOnlineId(); }
    void			            setHostType( EHostType hostType )                   { m_GroupieId.setHostType( hostType ); }
    EHostType	                getHostType( void ) const                           { return m_GroupieId.getHostType(); }

    EPluginType	                getHostPluginType( void );
    EPluginType	                getClientPluginType( void );

    // returns 0 if equal else -1 if less or 1 if greater
    int							compareTo( ConnectId& guid );
    // returns true if guids are same value
    bool						isEqualTo( const ConnectId& guid );
    // get a description of the plugin id
    std::string                 describeConnectId( void ) const;

    bool                        isValid( void )                                     { return m_SocketId.isValid() && m_GroupieId.isValid();  }
    void                        clear( void )                                       { m_SocketId.clearVxGUID(); m_GroupieId.clear(); m_IsRelayed = true; }

    void                        setIsRelayed( bool isRelayed )                      { m_IsRelayed = isRelayed; }
    bool                        isRelayed( void ) const                             { return m_IsRelayed; }

protected:
	//=== vars ===//
    VxGUID					    m_SocketId;
    GroupieId					m_GroupieId;
    bool                        m_IsRelayed{ true };
};

#pragma pack(pop)
