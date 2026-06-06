#pragma once
//============================================================================
// Copyright (C) 2012 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PSession.h"

#include <PktLib/PktsTodGame.h>

class TodGameSession : public P2PSession
{
public:
	TodGameSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EPluginType pluginType );

	PktTodGameStats		m_PktGameStats;
};
