//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

//#define USE_WEB_FILESHARE 1  // define this to enable sharing files through web page
#include <ptop_src/ptop_engine_src/WebServer/RcWebPageSettings.h>
#include <ptop_src/ptop_engine_src/WebServer/RcWebSkt.h>
#include <ptop_src/ptop_engine_src/WebServer/RcWebServer.h>
#ifdef USE_WEB_FILESHARE
	#include <ptop_src/ptop_engine_src/WebServer/RcWebPageBuilder.h>
#endif //USE_WEB_FILESHARE
