#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <PktLib/VxConnectInfo.h>

#include <GuiInterface/IDefs.h>

class ConnectRequest
{
public:
	ConnectRequest( EConnectReason eConnectReason = eConnectReasonAnnouncePing );
	ConnectRequest( VxConnectInfo& connectInfo, EConnectReason eConnectReason = eConnectReasonAnnouncePing );
	ConnectRequest( const ConnectRequest& rhs );

	ConnectRequest& operator =( const ConnectRequest& rhs );

	VxConnectInfo&				getConnectInfo( void )				{ return m_ConnectInfo; }
	VxGUID&						getMyOnlineId()						{ return m_ConnectInfo.getMyOnlineId(); }

	void						setConnectReason( EConnectReason eConnectReason );
	EConnectReason				getConnectReason( void );

	void						setTimeLastConnectAttemptMs( uint64_t u32TimeSec );
	uint64_t					getTimeLastConnectAttemptMs( void );
	bool						isTooSoonToAttemptConnectAgain( void );

protected:
	//=== vars ===//
	VxConnectInfo				m_ConnectInfo;
	EConnectReason				m_eConnectReason;
};


