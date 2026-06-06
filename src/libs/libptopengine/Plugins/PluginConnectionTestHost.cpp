//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginConnectionTestHost.h"

#include <P2PEngine/P2PEngine.h>
#include <NetServices/NetServiceHdr.h>
#include <NetServices/NetServicesMgr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER


//============================================================================
PluginConnectionTestHost::PluginConnectionTestHost( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseNetworkService( engine, pluginMgr, myIdent, pluginType )
, m_NetServicesMgr( engine.getNetServicesMgr() )
{
    setPluginType( ePluginTypeHostConnectTest );
}

//============================================================================
void PluginConnectionTestHost::testIsMyPortOpen( void )
{
    m_NetServicesMgr.addNetActionToQueue( eNetActionIsPortOpen );
}

//============================================================================
int32_t PluginConnectionTestHost::handlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr )
{
    //if( false == m_NetServiceUtil.isAllHttpContentArrived( sktBase ) )
    //{
    //	LogMsg( LOG_ERROR, "PluginNetServices::handlePtopConnection: not all of http content arrived\n" );
    //	return 0; // don't error.. we should get more later
    //}
    int32_t rc = 0;
    if( sktBase->isConnected() )
    {
        rc = internalHandlePtopConnection( sktBase, netServiceHdr );

        // flush the socket
        sktBase->getSktReadBuf();// so lock.. will unlock with sktBufAmountRead
        sktBase->sktBufAmountRead( netServiceHdr.m_TotalDataLen );
    }

    return rc;
}

//============================================================================
int32_t PluginConnectionTestHost::internalHandlePtopConnection( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr )
{
    switch( netServiceHdr.m_NetCmdType )
    {
    //case eNetCmdAboutMePage:
    //    LogMsg( LOG_ERROR, "PluginNetServices::handlePtopConnection: invalid cmd  eNetCmdAboutMePage" );
    //    return -1;

    //case eNetCmdStoryboardPage:
    //    LogMsg( LOG_ERROR, "PluginNetServices::handlePtopConnection: invalid cmd  eNetCmdStoryboardPage" );
    //    return -1;

    case eNetCmdClientPing:
        //LogMsg( LOG_INFO, "PluginNetServices::handlePtopConnection: eNetCmdPing\n" );
        return m_NetServicesMgr.handleNetCmdPing( sktBase, netServiceHdr );

    case eNetCmdClientPong:
        //LogMsg( LOG_INFO, "PluginNetServices::handlePtopConnection: eNetCmdPong\n" );
        return m_NetServicesMgr.handleNetCmdPong( sktBase, netServiceHdr );

    case eNetCmdIsMyPortOpenReq:
        return m_NetServicesMgr.handleNetCmdIsMyPortOpenReq( sktBase, netServiceHdr );

    case eNetCmdIsMyPortOpenReply:
        return m_NetServicesMgr.handleNetCmdIsMyPortOpenReply( sktBase, netServiceHdr );

    //case eNetCmdHostReq:
    //    return m_NetServicesMgr.handleNetCmdHostReq( sktBase, netServiceHdr );

    //case eNetCmdHostReply:
    //    return m_NetServicesMgr.handleNetCmdHostReply( sktBase, netServiceHdr );

    case eNetCmdUnknown:
    default:
        LogMsg( LOG_ERROR, "PluginNetServices::handlePtopConnection: unknown cmd type\n" );
        return -1;
    }

    return 0;
}
