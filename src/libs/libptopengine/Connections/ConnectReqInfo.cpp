//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectReqInfo.h"

#include <CoreLib/VxGlobals.h>

#include <memory.h>

//============================================================================
ConnectReqInfo::ConnectReqInfo( EConnectReason eConnectReason )
: m_eConnectReason( eConnectReason )
{
}

//============================================================================
ConnectReqInfo::ConnectReqInfo( VxConnectInfo& connectInfo, EConnectReason eConnectReason )
: m_eConnectReason( eConnectReason )
{
	m_ConnectInfo = connectInfo;
}

//============================================================================
ConnectReqInfo::ConnectReqInfo( const ConnectReqInfo &rhs )
{
	*this = rhs;
}

//============================================================================
ConnectReqInfo& ConnectReqInfo::operator =( const ConnectReqInfo& rhs )
{
	if( this != &rhs )
	{
		memcpy( this, &rhs, sizeof( ConnectReqInfo ) );
	}

	return *this;
}

//============================================================================
void ConnectReqInfo::setConnectReason( EConnectReason eConnectReason )
{
	m_eConnectReason = eConnectReason;
}

//============================================================================
EConnectReason ConnectReqInfo::getConnectReason( void )
{
	return m_eConnectReason;
}

//============================================================================
void ConnectReqInfo::setTimeLastConnectAttemptMs( uint64_t u32TimeSec )
{
	m_ConnectInfo.setTimeLastConnectAttemptMs( u32TimeSec );	
}

//============================================================================
uint64_t ConnectReqInfo::getTimeLastConnectAttemptMs( void )
{
	return m_ConnectInfo.getTimeLastConnectAttemptMs();	
}

//============================================================================
bool ConnectReqInfo::isTooSoonToAttemptConnectAgain( void )
{
	return m_ConnectInfo.isTooSoonToAttemptConnectAgain();
}


