//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "EngineParams.h"
#include "EngineSettingsDefaultValues.h"
#include <NetLib/VxGetRandomPort.h>

#define ENGINE_PARAMS_DBVERSION 1
#define MY_SETTINGS_KEY "RCKEY"

//============================================================================
EngineParams::EngineParams()
: VxSettings( "EngineParamsDb" )
, m_Initialized( false )
{
}

//============================================================================
EngineParams::~EngineParams()
{
	engineParamsShutdown();
}

//============================================================================
int32_t EngineParams::engineParamsStartup( std::string& strDbFileName )
{
	if( m_Initialized )
	{
		engineParamsShutdown();
	}

	int32_t rc = dbStartup(ENGINE_PARAMS_DBVERSION, strDbFileName.c_str());
	if( 0 == rc )
	{
		m_Initialized = true;
	}

	return rc;
}

//============================================================================
void EngineParams::engineParamsShutdown( void )
{
	if( m_Initialized )
	{
		dbShutdown();
		m_Initialized = false;
	}
}


//============================================================================
void EngineParams::setLastHostWebsiteUrl( std::string& strWebsiteUrl )
{
	setIniValue( "HostUrl", strWebsiteUrl );
}

//============================================================================
void EngineParams::getLastHostWebsiteUrl( std::string& strWebsiteUrl )
{
	getIniValue( "HostUrl", strWebsiteUrl, "" );
}

//============================================================================
void EngineParams::setLastHostWebsiteResolvedIp( std::string& strWebsiteIp )
{
	setIniValue( "HostIp", strWebsiteIp );
}

//============================================================================
void EngineParams::getLastHostWebsiteResolvedIp( std::string& strWebsiteIp )
{
	getIniValue( "HostIp", strWebsiteIp, "" );
}

//============================================================================
void EngineParams::setLastConnectTestUrl( std::string& strWebsiteUrl )
{
	setIniValue( "NetServiceUrl", strWebsiteUrl );
}

//============================================================================
void EngineParams::getLastConnectTestUrl( std::string& strWebsiteUrl )
{
	getIniValue( "NetServiceUrl", strWebsiteUrl, "" );
}

//============================================================================
void EngineParams::setLastConnectTestResolvedIp( std::string& strWebsiteIp )
{
	setIniValue( "NetServiceIp", strWebsiteIp );
}

//============================================================================
void EngineParams::getLastConnectTestResolvedIp( std::string& strWebsiteIp )
{
	getIniValue( "NetServiceIp", strWebsiteIp, "" );
}

//============================================================================
void EngineParams::setLastListenSocket( int64_t listenSocket )
{
	setIniValue( "LastListenSkt", listenSocket );
}

//============================================================================
int64_t EngineParams::getLastListenSocket( void )
{
	int64_t listenSkt{ 0 };
	getIniValue( "LastListenSkt", listenSkt, 0 );
	return listenSkt;
}
