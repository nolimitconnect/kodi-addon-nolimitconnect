//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "config_corelib.h"
#include "AppErr.h"
#include "VxDebug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MAX_APP_ERR_MSG_SIZE 1024

void default_app_err_handler( void * userData, EAppErr eAppErr, char * errMsg );

namespace
{
	APP_ERR_FUNCTION g_pfuncAppErrHandler = default_app_err_handler;
	void * g_pvUserData = 0;
}

//============================================================================
void VxSetAppErrHandler( APP_ERR_FUNCTION pfuncAppErrHandler, void * userData )
{
	g_pfuncAppErrHandler = pfuncAppErrHandler;
	g_pvUserData = userData;
}

//============================================================================
void default_app_err_handler( void * userData, EAppErr eAppErr, char * errMsg )
{
    NLC_UNUSED( userData );
	LogMsg( LOG_ERROR, "AppErr: %d %s\n", eAppErr, errMsg );
}

//============================================================================
void AppErr( EAppErr eAppErr, const char* errMsg, ...)
{
	char as8Buf[ MAX_APP_ERR_MSG_SIZE ];
	va_list argList;
	va_start( argList, errMsg );
	vsnprintf( as8Buf, sizeof( as8Buf ), errMsg, argList );
	as8Buf[sizeof( as8Buf ) - 1] = 0;
	va_end( argList );
	g_pfuncAppErrHandler( g_pvUserData, eAppErr, as8Buf );
}

