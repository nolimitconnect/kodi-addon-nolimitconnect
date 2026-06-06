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

#include <NetLib/VxServerMgr.h>

class IsPortOpenTest;

class PingResponseServer : public VxServerMgr 
{
public:
	PingResponseServer( IsPortOpenTest& isPortOpenTest );
	virtual ~PingResponseServer() = default;

	void						handleTcpSktCallback( std::shared_ptr<VxSktBase>& sktBase );

private:
	IsPortOpenTest&				m_IsPortOpenTest;
};

