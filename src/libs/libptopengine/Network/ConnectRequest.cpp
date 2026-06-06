//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectRequest.h"

#include <CoreLib/VxGlobals.h>

#include <memory.h>

//============================================================================
ConnectRequest::ConnectRequest( EConnectReason eConnectReason )
: m_eConnectReason( eConnectReason )
{
}

//============================================================================
ConnectRequest::ConnectRequest( VxConnectInfo& connectInfo, EConnectReason eConnectReason )
: m_eConnectReason( eConnectReason )
{
	m_ConnectInfo = connectInfo;
}

//============================================================================
ConnectRequest::ConnectRequest( const ConnectRequest &rhs )
{
	*this = rhs;
}

//============================================================================
ConnectRequest& ConnectRequest::operator =( const ConnectRequest& rhs )
{
	if( this != &rhs )
	{
		memcpy( this, &rhs, sizeof( ConnectRequest ) );
	}

	return *this;
}

//============================================================================
void ConnectRequest::setConnectReason( EConnectReason eConnectReason )
{
	m_eConnectReason = eConnectReason;
}

//============================================================================
EConnectReason ConnectRequest::getConnectReason( void )
{
	return m_eConnectReason;
}

//============================================================================
void ConnectRequest::setTimeLastConnectAttemptMs( uint64_t u32TimeSec )
{
	m_ConnectInfo.setTimeLastConnectAttemptMs( u32TimeSec );	
}

//============================================================================
uint64_t ConnectRequest::getTimeLastConnectAttemptMs( void )
{
	return m_ConnectInfo.getTimeLastConnectAttemptMs();	
}

//============================================================================
bool ConnectRequest::isTooSoonToAttemptConnectAgain( void )
{
	return m_ConnectInfo.isTooSoonToAttemptConnectAgain();
}


