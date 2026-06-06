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

#include "NetworkTestBase.h"

class QueryHostIdTest : public NetworkTestBase
{
public:
    QueryHostIdTest( P2PEngine& engine, EngineSettings& engineSettings, NetServicesMgr& netServicesMgr, NetServiceUtils& netServiceUtils );
	virtual ~QueryHostIdTest() = default;


	virtual void				fromGuiRunQueryHostIdTest( void );
	void						runTestShutdown( void );

	void						threadRunNetworkTest( void ) override;

private:
    ERunTestStatus			    doRunTest( std::string& nodeUrl );
    ERunTestStatus			    doRunTestFailed( void );
    ERunTestStatus			    doRunTestSuccess( void );

	//=== vars ===//
    bool                        m_TestIsRunning{ false };

private:
	QueryHostIdTest() = delete; // don't allow default constructor
	QueryHostIdTest(const QueryHostIdTest&) = delete; // don't allow copy constructor
};

