//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxMutex.h"

#include "VxDebug.h"

#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
# include "VxThread.h"
#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)

//#define SHOW_LOCKS 1

//============================================================================
VxMutex::VxMutex( void )
{
#ifdef TARGET_OS_WINDOWS
	// windows event
	m_hAccessLock = CreateEvent(	NULL,		// security attributes 
		FALSE,		// manual reset
		TRUE,		// initial state signaled 
		NULL );		// name of event
#else // LINUX
	pthread_mutexattr_init( &m_MutexAttr );
	pthread_mutexattr_settype( &m_MutexAttr,  PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_Lock, &m_MutexAttr );
#endif
#ifdef DEBUG_VX_MUTEX
	m_uiLastLockThreadId = 0;
#endif // DEBUG_VX_MUTEX
}

//============================================================================
VxMutex::~VxMutex( void )
{
#ifdef TARGET_OS_WINDOWS
	if( m_hAccessLock )
	{
		CloseHandle( m_hAccessLock ); 
		m_hAccessLock = NULL;
	}
#else // LINUX
	pthread_mutex_destroy( &m_Lock );
#endif
}

//============================================================================
int VxMutex::lock( int iInstance )
{
#ifdef SHOW_LOCKS
	LogMsg( LOG_INFO, "Unlocking VxMutex ..instance %d", iInstance );
#endif
	return this->lock();
}

//============================================================================
int VxMutex::unlock( int iInstance )
{
#ifdef SHOW_LOCKS
	LogMsg( LOG_INFO, "Unlocking VxMutex ..instance %d", iInstance );
#endif
    m_IsLocked = false;
#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
    m_uiLastLockThreadId = 0;
#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
	return this->unlock();
}

//============================================================================
int VxMutex::lock( void )
{
#ifdef TARGET_OS_WINDOWS

#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
	vx_assert( m_uiLastLockThreadId != VxGetCurrentThreadId() );
    // timeout if cannot aquire lock in specified time
	DWORD dwResult = WaitForSingleObject( m_hAccessLock, 180000 );
#else
	// wait infinite time
	DWORD dwResult = WaitForSingleObject( m_hAccessLock, INFINITE	);
#endif
	if( 0 == dwResult )
	{
		// object signaled
		m_IsLocked = true;
		return 0;
	}

	if( 0xFFFFFFFF == dwResult )
	{
		LogMsg( LOG_INFO, "VxMutex Lock failure %d", GetLastError() );
	}
	else
	{
		LogMsg( LOG_INFO, "VxMutex Lock failure %d result %d", GetLastError(), dwResult );
	}

	vx_assert( false );
	return 0;
#else // LINUX
#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
    unsigned int curThreadId = VxGetCurrentThreadId();
    bool possibleDeadlock = m_uiLastLockThreadId == curThreadId;
    if( possibleDeadlock )
    {
        LogMsg( LOG_ERROR, "VxMutex::lock possible deadlock thread 0x%X", curThreadId );
        vx_assert( false );
    }

#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
	int iError;

#if defined(DEBUG_VX_MUTEX_DEADLOCK)
    // timeout if cannot aquire lock in specified time
    struct timespec locktimeout;
    clock_gettime(CLOCK_REALTIME, &locktimeout);  // Get the current time
    locktimeout.tv_sec += 180;  // Add seconds to the current time
    locktimeout.tv_nsec = 0;  // No additional nanoseconds
    if( 0 != (iError = pthread_mutex_timedlock( &m_Lock, &locktimeout ) ) )
#else
	if( 0 != (iError = pthread_mutex_lock( &m_Lock ) ) )
#endif // defined(DEBUG_VX_MUTEX_DEADLOCK)
	{
#ifdef DEBUG_VX_MUTEX
		DoMutexError( this, pthread_self(), iError );
#endif// DEBUG_VX_MUTEX
        LogMsg( LOG_ERROR, "VxMutex Lock failure %d", iError );
		if( iError != EDEADLK )
		{
            LogMsg( LOG_ERROR, "VxMutex was not Deadlocked");
		}
		vx_assert( false );
	}

#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
    if( possibleDeadlock && !iError )
    {
        LogMsg( LOG_ERROR, "VxMutex::lock possible deadlock false detection thread 0x%X", curThreadId );
    }
	m_uiLastLockThreadId = VxGetCurrentThreadId();
#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
	m_IsLocked = true;
	return iError;
#endif
}

//============================================================================
int VxMutex::unlock( void )
{
	m_IsLocked = false;
	#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
		m_uiLastLockThreadId = 0;
	#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
#ifdef TARGET_OS_WINDOWS
	if( false == SetEvent( m_hAccessLock ) )
	{
		int iLastErr = GetLastError();
        LogMsg( LOG_ERROR, "VxMutex Unlock failure %d", iLastErr );
		vx_assert( false );
		return iLastErr;
	}
	return 0;
#else // LINUX
	int err;
	if( 0 != (err = pthread_mutex_unlock( &m_Lock ) ) )
	{
        LogMsg( LOG_ERROR, "VxMutex Unlock failure %d", err );
		vx_assert( false );
	}
	return err;
#endif
}

//============================================================================
int VxMutex::lock( const char* file, int line )
{
#ifdef DEBUG_VX_MUTEX
	uint32_t u32ThreadId = GetCurrentThreadId();
	bool bFound = false;
	for( int i = 0; i < MAX_MUTEX_THREADS; i++ )
	{
		if( m_agMutexThreadIds[ i ].m_u32ThreadId == u32ThreadId )
		{
			bFound = true;
			VxMutexDebug * pgMutDbg = &m_agMutexThreadIds[ i ];
			if( 0 != pgMutDbg->m_iLockCnt )
			{
				LogMsg( LOG_INFO, "VxMutex: LockCnt %d file %s line %d", pgMutDbg->m_iLockCnt, file, line );
				LogMsg( LOG_INFO, "VxMutex: Prev lock file %s line %d", pgMutDbg->m_pLastLockFile, pgMutDbg->m_iLastLockLine );
				LogMsg( LOG_INFO, "VxMutex: Prev Unock file %s line %d", pgMutDbg->m_pLastUnlockFile, pgMutDbg->m_iLastUnlockLine );
				vx_assert( false );
			}
			pgMutDbg->m_iLockCnt++;
		}
	}

	if( false == bFound )
	{
		for( int i = 0; i < MAX_MUTEX_THREADS; i++ )
		{
			if( m_agMutexThreadIds[ i ].m_u32ThreadId == 0 )
			{
				m_agMutexThreadIds[ i ].m_u32ThreadId = u32ThreadId;
				VxMutexDebug * pgMutDbg = &m_agMutexThreadIds[ i ];
				pgMutDbg->m_iLockCnt = 1;
				pgMutDbg->m_pLastLockFile = file;
				pgMutDbg->m_iLastLockLine = line;
				bFound = true;
				break;
			}
		}
	}

	if( false == bFound )
	{
		LogMsg( LOG_INFO, "VxMutex: Ran out of thread ids file %s line %d", file, line );
		vx_assert( false );
	}
#endif // DEBUG_VX_MUTEX

	return this->lock();

}
//============================================================================
int VxMutex::unlock( const char* file, int line )
{
#ifdef DEBUG_VX_MUTEX
	uint32_t u32ThreadId = GetCurrentThreadId();
	bool bFound = false;
	for( int i = 0; i < MAX_MUTEX_THREADS; i++ )
	{
		if( m_agMutexThreadIds[ i ].m_u32ThreadId == u32ThreadId )
		{
			bFound = true;
			VxMutexDebug * pgMutDbg = &m_agMutexThreadIds[ i ];
			if( 1 != pgMutDbg->m_iLockCnt )
			{
				LogMsg( LOG_INFO, "VxMutex: UnlockCnt %d file %s line %d", pgMutDbg->m_iLockCnt, file, line );
				LogMsg( LOG_INFO, "VxMutex: Prev lock file %s line %d", pgMutDbg->m_pLastLockFile, pgMutDbg->m_iLastLockLine );
				LogMsg( LOG_INFO, "VxMutex: Prev Unock file %s line %d", pgMutDbg->m_pLastUnlockFile, pgMutDbg->m_iLastUnlockLine );
				vx_assert( false );
			}
			pgMutDbg->m_u32ThreadId = 0;
			pgMutDbg->m_iLockCnt = 0;
			break;
		}
	}

	if( false == bFound )
	{
		LogMsg( LOG_INFO, "VxMutex: Unlock without lock attempted by thread 0x%x file %s line %d", u32ThreadId, file, line );
		vx_assert( false );
	}
#endif // DEBUG_VX_MUTEX
#if defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
    m_uiLastLockThreadId = 0;
#endif // defined(DEBUG_VX_MUTEX) || defined(DEBUG_VX_MUTEX_DEADLOCK)
    m_IsLocked = false;
	return this->unlock();
}

