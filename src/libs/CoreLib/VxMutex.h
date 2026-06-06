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

#include "VxDefs.h"

#ifdef TARGET_OS_WINDOWS
	#include <WinSock2.h>
	#include <windows.h>
#else
	#include <libpthread/pthread.h>
#endif // TARGET_OS_WINDOWS

#if defined(DEBUG) || defined(_DEBUG)
// defining DEBUG_VX_MUTEX_DEADLOCK causes lock to timeout after a several minutes
#define DEBUG_VX_MUTEX_DEADLOCK 1
#endif // defined(DEBUG) || defined(_DEBUG)

// uncomment for additional mutex lock debugging
//#define DEBUG_VX_MUTEX 1

#ifdef DEBUG_VX_MUTEX
#define MAX_MUTEX_THREADS 50
class VxMutexDebug
{
public:
	uint32_t		m_u32ThreadId{ 0 };
	const char*		m_pLastLockFile{ "NOLOCKCALLED" };
	int				m_iLastLockLine{ 0 };
	const char*		m_pLastUnlockFile{ "NOUNLOCKCALLED" };
	int				m_iLastUnlockLine{ 0 };
	int				m_iLockCnt{ 0 };
};
#endif // DEBUG_VX_MUTEX

class VxMutex
{
public:
	VxMutex();
	virtual ~VxMutex();

	int							lock( void );
	int							unlock( void );
	int							lock( int iInstance );
	int							unlock( int iInstance );
	int							lock( const char* file, int line );
	int							unlock( const char* file, int line );

#ifdef DEBUG_VX_MUTEX
	VxMutexDebug				m_agMutexThreadIds[ MAX_MUTEX_THREADS ];
#endif //DEBUG_VX_MUTEX
#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK) 
	unsigned int				m_uiLastLockThreadId;
#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)

	//=== vars ===//
	#ifdef TARGET_OS_WINDOWS
		HANDLE					m_hAccessLock;
	#else // LINUX
		pthread_mutex_t			m_Lock;
		pthread_mutexattr_t		m_MutexAttr;
	#endif
	bool						m_IsLocked{ false };

private:
	VxMutex(const VxMutex& rhs);// don't allow copy constructor
	VxMutex& operator=(const VxMutex& rhs);// don't allow assign operation
};




