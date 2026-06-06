#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "VxCommon.h"

#include <GuiInterface/IDefs.h>

#pragma pack(push)
#pragma pack(1)

class PktRelayUserDisconnect : public VxPktHdr
{
public:
	PktRelayUserDisconnect();

	void						setDestUserOnlineId( VxGUID& onlineId )	{ m_DestUserOnlineId = onlineId; }
	VxGUID&						getDestUserOnlineId( void )				{ return m_DestUserOnlineId; }

	void						setHostOnlineId( VxGUID& onlineId )		{ m_HostOnlineId = onlineId; }
	VxGUID&						getHostOnlineId( void )					{ return m_HostOnlineId; }

	void						setRelayError( ERelayErr relayErr )		{ m_RelayError = (uint8_t)relayErr; }
	ERelayErr					getRelayError( void )					{ return (ERelayErr)m_RelayError; }

	void						setRelayedPktType( uint16_t pktType )	{ m_RelayedPktType = pktType; }
	uint16_t					getRelayedPktType( void )				{ return m_RelayedPktType; }

	//=== vars ===//
	VxGUID						m_DestUserOnlineId;
	VxGUID						m_HostOnlineId;

private:
	uint8_t						m_RelayError{ 0 };
	uint8_t						m_u8Res1{ 0 };
	uint16_t					m_RelayedPktType{ 0 };
	uint32_t					m_u32Res3{ 0 };
};

#pragma pack(pop)

