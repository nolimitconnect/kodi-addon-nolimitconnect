//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxGUID.h>

#include <string>

class VxSktStatRecord
{
public:
	VxSktStatRecord() = default;
	VxSktStatRecord( SOCKET skt, std::string& ipAddr, VxGUID& connectionId, int64_t txedBytes, int64_t rxedBytes, int64_t lastActive, bool isTemp, VxGUID& peerOnlineId );
	VxSktStatRecord( const VxSktStatRecord& rhs );
	virtual ~VxSktStatRecord() = default;

	VxSktStatRecord&			operator=( const VxSktStatRecord& rhs );

	SOCKET						getSktHandle( void )				{ return m_SktHandle; };
	std::string&				getIpAddr( void )					{ return m_IpAddr; }
	VxGUID&						getConnectionId( void )				{ return m_ConnectId; }
	int64_t						getTxedBytes( void )				{ return m_TxedBytes; }
	int64_t						getRxedBytes( void )				{ return m_RxedBytes; }
	int64_t						getLastActiveTimestampMs( void )	{ return m_LastActiveTimeMs; }
	VxGUID&						getPeerOnlineId( void )				{ return m_PeerOnlineId; }
	bool						getIsTemporary( void )				{ return m_IsTemp; }

protected:
	SOCKET						m_SktHandle{ INVALID_SOCKET };
	std::string					m_IpAddr;
	VxGUID						m_ConnectId;
	int64_t						m_TxedBytes{ 0 };
	int64_t						m_RxedBytes{ 0 };
	int64_t						m_LastActiveTimeMs{ 0 };
	bool						m_IsTemp{ false };
	VxGUID						m_PeerOnlineId;

};

