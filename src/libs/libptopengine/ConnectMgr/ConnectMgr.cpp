//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectMgr.h"
#include "ConnectInfo.h"
#include "ConnectInfoDb.h"
#include "ConnectCallbackInterface.h"

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsFileList.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>
#include <CoreLib/VxPtopUrl.h>

#include <time.h>

//============================================================================
ConnectMgr::ConnectMgr( P2PEngine& engine, const char* dbName, const char* stateDbName )
: m_Engine( engine )
, m_ConnectInfoDb( engine, *this, dbName )
{
}

//============================================================================
void ConnectMgr::fromGuiUserLoggedOn( void )
{
    // dont call HostBaseMgr::fromGuiUserLoggedOn because we never generate sha hash for thumbnails
    if( !m_Initialized )
    {
        m_Initialized = true;
        // user specific directory should be set
        std::string dbFileName = VxGetSettingsDirectory();
        dbFileName += m_ConnectInfoDb.getDatabaseName(); 
        lockResources();
        m_ConnectInfoDb.dbShutdown();
        m_ConnectInfoDb.dbStartup( CONNECT_MGR_DB_VERSION, dbFileName );

        clearConnectInfoList();
        m_ConnectInfoDb.getAllConnects( m_ConnectInfoList );

        m_ConnectListInitialized = true;
        unlockResources();
    }
}

//============================================================================
bool ConnectMgr::isConnectedToHost( enum EHostType hostType, VxPtopUrl& hostUrl, std::shared_ptr<VxSktBase>& sktBase )
{
    bool isConnected = false;
    if( hostUrl.getOnlineId() == m_Engine.getMyOnlineId() )
    {
        // we are connecting to ourself
        sktBase = m_Engine.getSktLoopback();
        return true;
    }
    else
    {
        // lookup connection
        lockResources();
        for( auto iter = m_ConnectInfoList.begin(); iter != m_ConnectInfoList.end(); ++iter )
        {
            if( ( *iter )->getHostUrl() == hostUrl.getUrl() )
            {
                isConnected = true;
                break;
            }
        }

        unlockResources();
    }

    return isConnected;
}

//============================================================================
void ConnectMgr::addConnectMgrClient( ConnectCallbackInterface * client, bool enable )
{
    lockClientList();
    if( enable )
    {
        m_ConnectClients.emplace_back( client );
    }
    else
    {
        for( auto iter = m_ConnectClients.begin(); iter != m_ConnectClients.end(); ++iter )
        {
            if( *iter == client )
            {
                m_ConnectClients.erase( iter );
                break;
            }
        }
    }

    unlockClientList();
}

//============================================================================
void ConnectMgr::announceConnectAdded( ConnectInfo * assetInfo )
{
    ConnectInfo * userHostInfo = dynamic_cast<ConnectInfo *>( assetInfo );
    if( userHostInfo )
    {
	    LogMsg( LOG_INFO, "ConnectMgr::%s start", __func__ );
	
	    lockClientList();
	    for( auto& client : m_ConnectClients )
	    {
		    client->callbackConnectAdded( userHostInfo );
	    }

	    unlockClientList();
	    LogMsg( LOG_INFO, "ConnectMgr::%s done", __func__ );
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectMgr::%s dynamic_cast failed", __func__ );
    }
}

//============================================================================
void ConnectMgr::announceConnectUpdated( ConnectInfo * assetInfo )
{
    ConnectInfo * userHostInfo = dynamic_cast<ConnectInfo *>( assetInfo );
    if( userHostInfo )
    {
        lockClientList();
	    for( auto& client : m_ConnectClients )
	    {
            client->callbackConnectUpdated( userHostInfo );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "ConnectMgr::%s dynamic_cast failed", __func__ );
    }
}

//============================================================================
void ConnectMgr::announceConnectRemoved( VxGUID& hostOnlineId )
{
	lockClientList();
	for( auto& client : m_ConnectClients )
	{
		client->callbackConnectRemoved( hostOnlineId );
	}

	unlockClientList();
}

//============================================================================
void ConnectMgr::announceConnectOfferState( VxGUID& hostOnlineId, enum EOfferState userHostOfferState )
{
	LogMsg( LOG_INFO, "ConnectMgr::%s state %d start", __func__, userHostOfferState );
	lockClientList();
	for( auto& client : m_ConnectClients )
	{
		client->callbackConnectOfferState( hostOnlineId, userHostOfferState );
	}

	unlockClientList();
	LogMsg( LOG_INFO, "ConnectMgr::%s state %d done", __func__, userHostOfferState );
}

//============================================================================
void ConnectMgr::announceConnectOnlineState( VxGUID& hostOnlineId, enum EOnlineState onlineState, VxGUID& connectionId )
{
    LogMsg( LOG_INFO, "ConnectMgr::%s state %d start", __func__, onlineState );
    lockClientList();
	for( auto& client : m_ConnectClients )
	{
        client->callbackConnectOnlineState( hostOnlineId, onlineState, connectionId );
    }

    unlockClientList();
    LogMsg( LOG_INFO, "ConnectMgr::%s state %d done", __func__, onlineState );
}

//============================================================================
void ConnectMgr::clearConnectInfoList( void )
{
    for( auto iter = m_ConnectInfoList.begin(); iter != m_ConnectInfoList.end(); ++iter )
    {
        delete (*iter);
    }

    m_ConnectInfoList.clear();
}
