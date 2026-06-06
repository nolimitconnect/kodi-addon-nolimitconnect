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

#include <CoreLib/config_corelib.h>

#ifdef TARGET_OS_WINDOWS
    #include <WinSock2.h>
    #include <windows.h>
#else
    #include <semaphore.h>
#endif

class VxSemaphore
{
public:
	VxSemaphore();
	~VxSemaphore();

	//! wait until another thread calls Signal
	//! if iTimeoutMilliseconds == 0 then wait infinite time
	//! return true if was signaled else return false if timed out
	bool						wait( int iTimeoutMilliseconds = 0 );
	//! signal waiting thread to run
	int							signal( void );

	//=== vars ===//
#ifdef TARGET_OS_WINDOWS 
	HANDLE						m_hAccessLock; // windows handle to semaphore
#else
	sem_t						m_Semaphore; // linux semaphore
#endif

private:
	VxSemaphore(const VxSemaphore& rhs);// don't allow copy constructor
	VxSemaphore& operator=(const VxSemaphore& rhs);// don't allow copy operation
};
