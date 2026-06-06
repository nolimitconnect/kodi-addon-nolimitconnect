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

#include "PktTypes.h"
#include <GuiInterface/IDefs.h>

#pragma pack(push)
#pragma pack(1)

#define MAX_FRIEND_OFFER_MSG_LEN 2048

class PktFriendOfferReq : public VxPktHdr
{
public:
    PktFriendOfferReq();

    void                        setHostType( enum EHostType hostType )   { m_HostType = (uint8_t)hostType; }
    EHostType                   getHostType( void )                 { return (EHostType)m_HostType; }

	void						setOfferMsg( const char* msg );
	const char*				    getOfferMsg( void );

private:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_Res1{ 0 };
    uint16_t					m_StrLen{ 0 };					
    uint32_t					m_Res3{ 0 };	
    uint64_t					m_TimeRequestedMs{ 0 };		
    uint64_t					m_Res4{ 0 };
    uint8_t						m_au8Data[ MAX_FRIEND_OFFER_MSG_LEN + 16 ];								
};

class PktFriendOfferReply : public VxPktHdr
{
public:
    PktFriendOfferReply();

    void                        setHostType( enum EHostType hostType )   { m_HostType = (uint8_t)hostType; }
    EHostType                   getHostType( void )                 { return (EHostType)m_HostType; }

    void						setOfferMsg( const char* msg );
    const char*				    getOfferMsg( void );

private:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_Res1{ 0 };
    uint16_t					m_StrLen{ 0 };					
    uint32_t					m_Res3{ 0 };	
    uint64_t					m_TimeRequestedMs{ 0 };		
    uint64_t					m_Res4{ 0 };
    uint8_t						m_au8Data[ MAX_FRIEND_OFFER_MSG_LEN + 16 ];	
};

#pragma pack(pop)
