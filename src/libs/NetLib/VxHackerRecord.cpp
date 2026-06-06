//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxHackerRecord.h"

#include <CoreLib/VxTime.h>

//============================================================================
VxHackerRecord::VxHackerRecord( EHackerLevel hackLevel, EHackerReason hackReason, std::string& ipAddr, VxGUID signatureId )
: m_HackerLevel( hackLevel )
, m_HackerReason( hackReason )
, m_IpAddr( ipAddr )
, m_SignatureId( signatureId )
{
	m_TimeMs = GetGmtTimeMs();
}

//============================================================================
VxHackerRecord::VxHackerRecord( const VxHackerRecord& rhs )
	: m_HackerLevel( rhs.m_HackerLevel )
	, m_HackerReason( rhs.m_HackerReason )
	, m_IpAddr( rhs.m_IpAddr )
	, m_TimeMs( rhs.m_TimeMs )
	, m_SignatureId( rhs.m_SignatureId )
	, m_HackCount( rhs.m_HackCount )
{
}

//============================================================================
VxHackerRecord& VxHackerRecord::operator=( const VxHackerRecord& rhs )
{
	if( this != &rhs )
	{
		m_HackerLevel = rhs.m_HackerLevel;
		m_HackerReason = rhs.m_HackerReason;
		m_IpAddr = rhs.m_IpAddr;
		m_TimeMs = rhs.m_TimeMs;
		m_SignatureId = rhs.m_SignatureId;
		m_HackCount = rhs.m_HackCount;
	}

	return *this;
}
