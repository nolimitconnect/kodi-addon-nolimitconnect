//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

// DO NOT USE FOR NLC.. for local testing only.. NLC uses VirtStreamMgr with streaming support

#include "VFileMgr.h"

#include <CoreLib/VFile.h>
#include <CoreLib/VxFileUtil.h>

//============================================================================
VirtFileMgr& GetVirtFileMgr()
{
	static VFileMgr virtStreamMgr;
	return virtStreamMgr;
}
