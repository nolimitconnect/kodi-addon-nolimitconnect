//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktsMembership.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PktMembershipReq::PktMembershipReq()
{
	setPktType( PKT_TYPE_MEMBERSHIP_REQ );
}

//============================================================================
PktMembershipReply::PktMembershipReply()
{
	vx_assert( 0 == ( sizeof( PktMembershipReply ) & 0x0f ) );
	setPktType( PKT_TYPE_MEMBERSHIP_REPLY );
	setPktLength( sizeof( PktMembershipReply ) );
}

//============================================================================
void PktMembershipReply::setHostMembership( EHostType hostType, EMembershipState membership )
{
	switch( hostType )
	{
	case eHostTypeNetwork:
		m_NetworkMembership = ( uint8_t )membership;
		break;
	case eHostTypeConnectTest:
		m_ConnectTestMembership = ( uint8_t )membership;
		break;
	case eHostTypeGroup:
		m_GroupMembership = ( uint8_t )membership;
		break;
	case eHostTypeChatRoom:
		m_ChatRoomMembership = ( uint8_t )membership;
		break;
	case eHostTypeRandomConnect:
		m_RandomConnectMembership = ( uint8_t )membership;
		break;
	default:
		vx_assert( false );
	}
}

//============================================================================
EMembershipState PktMembershipReply::getHostMembership( EHostType hostType )
{
	switch( hostType )
	{
	case eHostTypeNetwork:
		return ( EMembershipState )m_NetworkMembership;
		break;
	case eHostTypeConnectTest:
		return ( EMembershipState )m_ConnectTestMembership;
		break;
	case eHostTypeGroup:
		return ( EMembershipState )m_GroupMembership;
		break;
	case eHostTypeChatRoom:
		return ( EMembershipState )m_ChatRoomMembership;
		break;
	case eHostTypeRandomConnect:
		return ( EMembershipState )m_RandomConnectMembership;
		break;
	default:
		vx_assert( false );
		return eMembershipStateNone;
	}
}