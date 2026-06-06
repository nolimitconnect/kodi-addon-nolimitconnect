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

class VxHackerRecord
{
public:
	VxHackerRecord() = default;
	VxHackerRecord( EHackerLevel hackLevel, EHackerReason hackReason, std::string& ipAddr, VxGUID signatureId = VxGUID::nullVxGUID() );
	VxHackerRecord( const VxHackerRecord& rhs );
	virtual ~VxHackerRecord() = default;

	VxHackerRecord&				operator=( const VxHackerRecord& rhs );

	EHackerLevel				getHackerLevel( void )			{ return m_HackerLevel; };
	EHackerReason				getHackerReason( void )			{ return m_HackerReason; };
	std::string&				getIpAddr( void )				{ return m_IpAddr; }
	int64_t						getHackTimestampMs( void )		{ return m_TimeMs; }
	VxGUID&						getSignature( void )			{ return m_SignatureId; }
	int							getOffenseCount( void )			{ return m_HackCount; }

	void						incrementHackCount( void )		{ m_HackCount++; }

protected:
	EHackerLevel				m_HackerLevel{ eHackerLevelUnknown };
	EHackerReason				m_HackerReason{ eHackerReasonUnknown };
	std::string					m_IpAddr;
	int64_t						m_TimeMs{ 0 };
	VxGUID						m_SignatureId;
	int							m_HackCount{ 1 };
};

