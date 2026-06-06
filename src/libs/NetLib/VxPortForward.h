#pragma once
//============================================================================
// Copyright (C) 2023 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <stdint.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>

class VxPortForward
{
public:
	VxPortForward();
	~VxPortForward();

	static void shutdownPortForward( void );

	static void setEnablePortForward( bool enable );
	static bool getEnablePortForward( void );

	static void setUseIpv6( bool enable );
	static bool getUseIpv6( void );

	static bool addPortForward( bool ipv6, std::string ipAddr, uint16_t port, bool runInThread = false );
	static bool removePortForward( bool ipv6, uint16_t port, bool runInThread = false );

	static bool listPortForward( std::string ipAddr, bool ipv6 = false, bool runInThread = false );

protected:
	static bool doAddPortForward( void );
	static bool doRemovePortForward( void );
	static bool doListPortForward( void );

	static void waitForThreadToFinish( void );
	static void killThread( void );

	static bool m_ForwardEnable;

	static bool m_IsShutdown;
	static bool m_IsIpv6;
	static bool m_UseIpv6; // ipv6 is primary configureation
	static std::string m_IpAddr;
	static uint16_t m_Port;

	static std::mutex runCmdMutex;
	static std::thread runUpnpThread;
};
