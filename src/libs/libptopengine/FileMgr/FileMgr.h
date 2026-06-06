#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxSemaphore.h>

#include <stdint.h>

class P2PEngine;
class VxGUID;

class FileMgr
{
public:
	void           				fromGuiScanFolderForMedia( P2PEngine& engine, VxGUID& appInstId, std::string dirToScan, uint8_t fileTypeFilter );
	void           				fromGuiScanItemReceived( VxGUID& appInstId );
	void           				fromGuiScanFolderCancel( VxGUID& appInstId );

protected:
	void						waitForReceiveAck( void );

	bool						m_WasCanceled{ false };
	VxGUID						m_AppInstId;
	VxSemaphore					m_RxSemaphore;
};

