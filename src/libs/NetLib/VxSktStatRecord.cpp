//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktStatRecord.h"

//============================================================================
VxSktStatRecord::VxSktStatRecord( SOCKET skt, std::string& ipAddr, VxGUID& connectionId, int64_t txedBytes, int64_t rxedBytes, int64_t lastActive, bool isTemp, VxGUID& peerOnlineId )
: m_SktHandle( skt )
, m_IpAddr( ipAddr )
, m_ConnectId( connectionId )
, m_TxedBytes( txedBytes )
, m_RxedBytes( rxedBytes )
, m_LastActiveTimeMs( lastActive )
, m_IsTemp( isTemp )
, m_PeerOnlineId( peerOnlineId )
{
}

//============================================================================
VxSktStatRecord::VxSktStatRecord( const VxSktStatRecord& rhs )
	: m_SktHandle( rhs.m_SktHandle )
	, m_IpAddr( rhs.m_IpAddr )
	, m_ConnectId( rhs.m_ConnectId )
	, m_TxedBytes( rhs.m_TxedBytes )
	, m_RxedBytes( rhs.m_RxedBytes )
	, m_LastActiveTimeMs( rhs.m_LastActiveTimeMs )
	, m_IsTemp( rhs.m_IsTemp )
	, m_PeerOnlineId( rhs.m_PeerOnlineId )
{
}

//============================================================================
VxSktStatRecord& VxSktStatRecord::operator=( const VxSktStatRecord& rhs )
{
	if( this != &rhs )
	{
		m_SktHandle = rhs.m_SktHandle;
		m_IpAddr = rhs.m_IpAddr;
		m_ConnectId = rhs.m_ConnectId;
		m_TxedBytes = rhs.m_TxedBytes;
		m_RxedBytes = rhs.m_RxedBytes;
		m_LastActiveTimeMs = rhs.m_LastActiveTimeMs;
		m_IsTemp = rhs.m_IsTemp;
		m_PeerOnlineId = rhs.m_PeerOnlineId;
	}

	return *this;
}
