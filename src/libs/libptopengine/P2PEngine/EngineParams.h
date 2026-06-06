#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <NetLib/NetSettings.h>
#include <PktLib/VxCommon.h>
#include <CoreLib/VxSettings.h>

class EngineParams : public VxSettings
{
public:

	EngineParams();
	virtual ~EngineParams();

	int32_t						engineParamsStartup( std::string& strDbFileName );
	void						engineParamsShutdown( void );

	void						setLastHostWebsiteUrl( std::string& strWebsiteUrl );
	void						getLastHostWebsiteUrl( std::string& strWebsiteUrl );
	void						setLastHostWebsiteResolvedIp( std::string& strWebsiteIp );
	void						getLastHostWebsiteResolvedIp( std::string& strWebsiteIp );

	void						setLastConnectTestUrl( std::string& strWebsiteUrl );
	void						getLastConnectTestUrl( std::string& strWebsiteUrl );
	void						setLastConnectTestResolvedIp( std::string& strWebsiteIp );
	void						getLastConnectTestResolvedIp( std::string& strWebsiteIp );

	void						setLastListenSocket( int64_t listenSocket );
	int64_t						getLastListenSocket( void );

private:
	bool						m_Initialized;
};
