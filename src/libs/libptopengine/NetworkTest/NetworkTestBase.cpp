//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetworkTestBase.h"

#include <P2PEngine/P2PEngine.h>
#include <NetServices/NetServiceHdr.h>
#include <NetServices/NetServiceUtils.h>

#include <Network/NetworkMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>

#include <CoreLib/VxSktUtil.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktConnectSimple.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

namespace
{
    //============================================================================
    void * NetworkTestBaseThreadFunc( void * pvContext )
    {
        VxThread* poThread = ( VxThread* )pvContext;
        poThread->setIsThreadRunning( true );
        NetworkTestBase * netTest = ( NetworkTestBase * )poThread->getThreadUserParam();
        if( netTest && false == poThread->isAborted() )
        {
            netTest->threadRunNetworkTest();
            netTest->networkTestComplete();
        }

        poThread->threadAboutToExit();
        return nullptr;
    }
}

//============================================================================
NetworkTestBase::NetworkTestBase( P2PEngine& engine, EngineSettings& engineSettings, NetServicesMgr& netServicesMgr, NetServiceUtils& netServiceUtils )
    : m_Engine( engine )
    , m_EngineSettings( engineSettings )
    , m_NetServicesMgr( netServicesMgr )
    , m_NetServiceUtils( netServiceUtils )
    , m_ClientTestName( "Client Test" )
    , m_HostTestName( "Host Test" )
{
}

//============================================================================
void NetworkTestBase::startNetworkTest( void )
{
    m_RunTestThread.abortThreadRun( true );
    while( m_RunTestThread.isThreadRunning() )
    {
        VxSleep( 200 );
    }

    m_RunTestThread.startThread( ( VX_THREAD_FUNCTION_T )NetworkTestBaseThreadFunc, this, "NetworkTestBaseThread" );
}

//============================================================================
void NetworkTestBase::stopNetworkTest( void )
{
    m_RunTestThread.abortThreadRun( true );
}

//============================================================================
void NetworkTestBase::sendRunTestStatus( ERunTestStatus eStatus, const char* msg, ... )
{
    std::array<char, 1024> as8Buf;
    va_list argList;
    va_start( argList, msg );
    vsnprintf( as8Buf.data(), as8Buf.size(), msg, argList);
    as8Buf[ as8Buf.size() - 1 ] = 0;
    va_end( argList );
    IToGui::getIToGui().toGuiRunTestStatus( getTestName().c_str(), eStatus, as8Buf.data() );
}

//============================================================================
void NetworkTestBase::sendTestLog( const char* msg, ... )
{
    std::array<char, 1024> as8Buf;
    va_list argList;
    va_start( argList, msg );
    vsnprintf( as8Buf.data(), as8Buf.size(), msg, argList);
    as8Buf[ as8Buf.size() - 1 ] = 0;
    va_end( argList );
    IToGui::getIToGui().toGuiRunTestStatus( getTestName().c_str(), eRunTestStatusLogMsg, as8Buf.data() );
}
