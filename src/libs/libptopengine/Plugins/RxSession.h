#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginSessionBase.h"

#include <CoreLib/VxSemaphore.h>

class PktVideoFeedPic;

class RxSession : public PluginSessionBase
{
public:
	RxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	RxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	virtual ~RxSession();

	bool						waitForResponse( int iTimeoutMs );
	void						signalResponseRecieved( void )			{ m_Semaphore.signal(); }

	PktVideoFeedPic *			getVideoFeedPkt( void )						{ return m_PktVideoFeedPic; }
	void						setVideoFeedPkt( PktVideoFeedPic * pkt )	{ m_PktVideoFeedPic = pkt; }

protected:
	//=== vars ===//
	VxSemaphore					m_Semaphore;
	PktVideoFeedPic *			m_PktVideoFeedPic;
};
