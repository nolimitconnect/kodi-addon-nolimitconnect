#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SendQueueDefs.h"

#include <CoreLib/GroupieId.h>

class SendQueInfo : public GroupieId
{
public:
    SendQueInfo() = default;
    SendQueInfo( const SendQueInfo& rhs );
    SendQueInfo( GroupieId& groupieId );
    SendQueInfo( GroupieId& groupieId, enum ESendQueState sendState, int64_t modTime = 0 );
    SendQueInfo( VxGUID& userOnlineId, VxGUID& hostOnlineId );
    SendQueInfo( VxGUID& userOnlineId, VxGUID& hostOnlineId, enum ESendQueState sendState, int64_t modTime = 0 );

    SendQueInfo&                operator = ( const SendQueInfo& rhs );
    bool						operator == ( const SendQueInfo& rhs ) const;
    bool						operator != ( const SendQueInfo& rhs ) const;
    bool						operator < ( const SendQueInfo& rhs ) const;
    bool						operator <= ( const SendQueInfo& rhs ) const;
    bool						operator > ( const SendQueInfo& rhs ) const;
    bool						operator >= ( const SendQueInfo& rhs ) const;

    GroupieId                   getGroupieId( void )                                { return *((GroupieId*)this); }

    VxGUID                      getAssetId( void )                                  { return getHostOnlineId(); }
    
    void			            setModTime( int64_t modTime )                       { m_ModTime = modTime; }
    int64_t	                    getModTime( void ) const                            { return m_ModTime; }

    void				        setSendQueState( enum ESendQueState sendState )     { m_SendState = sendState; }
    enum ESendQueState          getSendQueState( void ) const                       { return m_SendState; }

    // get a description of the plugin id
    std::string                 describeSendQueInfo( void ) const;

protected:
	//=== vars ===//
    enum ESendQueState          m_SendState{ eSendQueStateUnknown };
    int64_t					    m_ModTime{ 0 };
};
