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

#ifndef TARGET_OS_WINDOWS
#include <libpthread/pthread.h>
#endif // TARGET_OS_WINDOWS

#define VX_FLAG_THREAD_ABORT		0x01
#define VX_FLAG_THREAD_RUNNING		0x02
#define VX_FLAG_THREAD_DONE			0x04
#define VX_FLAG_THREAD_CREATED		0x08

typedef void * 	(*VX_THREAD_FUNCTION_T)(void *);
typedef void 	(*VX_THREAD_START_CALLBACK_T)(unsigned int,const char*);
typedef void 	(*VX_THREAD_EXIT_CALLBACK_T)(unsigned int, bool,const char*);

// protected by #ifdef __cplusplus so can be included in .c code
#ifdef __cplusplus

#include <string>

class VxThread
{
public:
	VxThread(void);
	virtual ~VxThread(void);

	const char* 				getThreadName( void );
    unsigned int				getThreadId( void )				    { return m_uiThreadId; } // In android this returns the TID you see in eclipse
	unsigned int				getThreadTid( void )				{ return m_ThreadTid; } // In android this returns the TID you see in eclipse
	void * 						getThreadUserParam( void );

	void						dumpThreadInfo( void );

	void						setIsThreadCreated( bool bIsCreated );
	bool						isThreadCreated( void );

	void						setIsThreadRunning( bool bIsRunning, bool calledFromStartedThread = true );
	bool						isThreadRunning( void );

	void 						abortThreadRun( bool bAbort );
	bool 						isAborted( void );

	void						setIsThreadDone( bool bIsDoneExit );
	bool						isThreadDone( void );

	void						setIsThreadEndCallbackLocked( bool bIsLocked );
	bool						isThreadEndCallbackLocked( void );

	int32_t						startThread(	VX_THREAD_FUNCTION_T 	pfuncThreadFunc,				// function that thread calls
												void * 					pvUserParam,					// user defined param
                                                const char* 			pThreadName = nullptr,          // thread name
												int 					iExtraStackSpace = 0);			// will be added to minimum stack size

	int32_t						killThread( void );

	//! Thread calls this just before exit
	virtual void 				threadAboutToExit( bool bExitThreadNow = true );

	//! set function that will be called when any thread is started
	static void 				setThreadStartCallback( VX_THREAD_START_CALLBACK_T func );

	//! set function that will be called when any thread is about to exit
	static void 				setThreadExitCallback( VX_THREAD_EXIT_CALLBACK_T func );
	
	static int					getThreadsRunningCount( void );
	static void					dumpRunningThreads( void );

protected:
	//=== vars ===//
	bool						m_bIsExitCallbackLocked;
	void * 						m_pvUserParam; 		// user specified param store here so can be accessed from thread
	unsigned char 				m_u8ThreadFlags;    // thread state flags
	std::string 				m_strThreadName;	// thread user defined name
	static VX_THREAD_START_CALLBACK_T 	m_funcStartCallback;
	static VX_THREAD_EXIT_CALLBACK_T 	m_funcExitCallback;

#ifdef TARGET_OS_WINDOWS
    DWORD 						m_uiThreadId{0};		// thread ID
    HANDLE 						m_hThread{0};		    // handle to thread
#else
    unsigned int 				m_uiThreadId{0};		// thread ID
	pthread_t 					m_ThreadInfo;      // pthread info
	pthread_attr_t 				m_ThreadAttr;
#endif
    unsigned int				m_ThreadTid{0};
    uint64_t					m_ThreadStartTimeGmtMs{0};

private:
	VxThread(const VxThread& rhs) = delete;// don't allow copy constructor
	VxThread& operator=(const VxThread& rhs) = delete;// don't allow copy operation
};

std::string		VxGetThreadInfo( void ); 

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
//! get the thread id of current thread that is executing
unsigned int	VxGetCurrentThreadId( void );

//! set the thread id of qt application using current thread id
void			VxSetGuiThreadId( void );
//! get the thread id of qt application
unsigned int	VxGetGuiThreadId( void );
//! return true if current thraad is the qt gui thread
bool			VxIsGuiThreadId( void );

void            VxThreadDefaultStartCallback( unsigned int threadId, const char* threadName );
void            VxThreadDefaultStopCallback( unsigned int threadId, bool iIsExitCallbackLocked, const char* threadName );

#ifdef __cplusplus
}
#endif
