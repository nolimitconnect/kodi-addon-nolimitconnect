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

#include "PktTypes.h"

#include <GuiInterface/IDefs.h>
#include <CoreLib/PktBlobEntry.h>

#pragma pack(push)
#pragma pack(1)

#define MAX_FRIEND_REQUEST_MSG_LEN 1024

class PktFriendRequestBase : public VxPktHdr
{
public:
    PktFriendRequestBase();

    void                        setRequestState( enum EFriendRequestState hostType )    { m_RequestState = (uint8_t)hostType; }
    EFriendRequestState         getRequestState( void )                                 { return (EFriendRequestState)m_RequestState; }

	void						setRequestMsg( const char* msg );
	const char*				    getRequestMsg( void );

    void						setRequestId( VxGUID& requestId ) { m_RequestId = requestId; }
    VxGUID				        getRequestId( void ) { return m_RequestId; }

    PktBlobEntry&               getBlobEntry( void )                                        { return m_BlobEntry; }

    void                        calcPktLen( void );

private:
    uint8_t					    m_RequestState{ 0 };
    uint8_t					    m_Res1{ 0 };
    uint16_t					m_StrLen{ 0 };					
    uint32_t					m_Res3{ 0 };	
    uint64_t					m_TimeRequestedMs{ 0 };		
    uint64_t					m_Res4{ 0 };
    VxGUID                      m_RequestId;
    uint8_t						m_au8Data[ MAX_FRIEND_REQUEST_MSG_LEN + 16 ];			

    PktBlobEntry                m_BlobEntry;	        // size 14352
};

class PktFriendRequestReq : public PktFriendRequestBase
{
public:
    PktFriendRequestReq();
};

class PktFriendRequestReply : public PktFriendRequestBase
{
public:
    PktFriendRequestReply();
};

#pragma pack(pop)
