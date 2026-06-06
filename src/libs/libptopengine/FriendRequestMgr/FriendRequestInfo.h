#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxGUID.h>
#include <PktLib/VxCommon.h>

#include <memory>
#include <string>

class FriendRequestInfo
{
public:
	FriendRequestInfo() = default;
	FriendRequestInfo( const FriendRequestInfo& rhs );
    FriendRequestInfo( VxGUID& onlineId, VxGUID& requestId, std::string& requestText, EFriendRequestState requestState, std::shared_ptr<VxNetIdent> netIdent, int64_t timeModified );
    virtual ~FriendRequestInfo() = default;

	FriendRequestInfo&			operator=( const FriendRequestInfo& rhs ); 

    bool                        isFriendRequestValid( void );

    void				        setUserOnlineId( VxGUID& onlineId )                 { m_OnlineId = onlineId; }
    VxGUID&                     getUserOnlineId( void )                             { return m_OnlineId; }

    void				        setRequestId( VxGUID& onlineId )                    { m_RequestId = onlineId; }
    VxGUID&                     getRequestId( void )                                { return m_RequestId; }

    void				        setRequestText( std::string requestText )           { m_RequestText = requestText; }
    std::string&                getRequestText( void )                              { return m_RequestText; }

    void				        setRequestState( EFriendRequestState requestState ) { m_RequestState = requestState; }
    EFriendRequestState         getRequestState( void )                             { return m_RequestState; }

    std::string                 getFriendRequestUrl( void )                         { return m_NetIdent->getMyOnlineUrl(); }
    
    void			            setRequestTimestamp( int64_t timestampMs )          { m_RequestTimestampMs = timestampMs; }
    int64_t                     getRequestTimestamp( void )                         { return m_RequestTimestampMs; }

    void			            setNetIdent( std::shared_ptr<VxNetIdent> netIdent ) { m_OnlineId = netIdent->getMyOnlineId(); m_NetIdent = netIdent; }
    std::shared_ptr<VxNetIdent>& getNetIdent( void )                                { return m_NetIdent; }

protected:
	//=== vars ===//
    VxGUID                      m_OnlineId;
    VxGUID                      m_RequestId;
    std::string                 m_RequestText;
    EFriendRequestState         m_RequestState{ eFriendRequestUnknown }; 
    std::string                 m_FriendRequestUrl;
    int64_t                     m_RequestTimestampMs{ 0 };
    std::shared_ptr<VxNetIdent> m_NetIdent;
};
