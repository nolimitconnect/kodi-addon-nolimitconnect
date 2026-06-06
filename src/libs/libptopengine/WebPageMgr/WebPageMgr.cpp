//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "WebPageMgr.h"
#include "WebPageCallbackInterface.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>

//============================================================================
WebPageMgr::WebPageMgr( P2PEngine& engine )
: m_Engine( engine )
{
    LogMsg( LOG_VERBOSE, "WebPageMgr::WebPageMgr" );
}

//============================================================================
void WebPageMgr::fromGuiUserLoggedOn( void )
{
    // dont call HostBaseMgr::fromGuiUserLoggedOn because we never generate sha hash for thumbnails
    if( !m_Initialized )
    {
        m_Initialized = true;
    }
}

//============================================================================
void WebPageMgr::wantWebPageMgrCallbacks( WebPageCallbackInterface * client, bool enable )
{
    lockClientList();
    std::vector<WebPageCallbackInterface*>::iterator iter;
    for( iter = m_WebPageClients.begin(); iter != m_WebPageClients.end(); ++iter )
    {
        if( *iter == client )
        {
            m_WebPageClients.erase( iter );
            break;
        }
    }

    if( enable )
    {
        m_WebPageClients.push_back( client );
    }

    unlockClientList();
}

//============================================================================
void WebPageMgr::announceWebDownloadStarted( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, int fileNum )
{
    lockClientList();
    std::vector<WebPageCallbackInterface*>::iterator iter;
    for( iter = m_WebPageClients.begin(); iter != m_WebPageClients.end(); ++iter )
    {
        WebPageCallbackInterface* client = *iter;
        client->callbackWebDownloadStarted( webPageType, onlineId, fileName, fileNum );
    }

    unlockClientList();
}

//============================================================================
void WebPageMgr::announceWebDownloadProgress( EWebPageType webPageType, VxGUID& onlineId, int fileNum, int progress )
{
    lockClientList();
    std::vector<WebPageCallbackInterface *>::iterator iter;
    for( iter = m_WebPageClients.begin(); iter != m_WebPageClients.end(); ++iter )
    {
        WebPageCallbackInterface * client = *iter;
        client->callbackWebDownloadProgress( webPageType, onlineId, fileNum, progress );
    }

    unlockClientList();
}

//============================================================================
void WebPageMgr::announceWebDownloadComplete( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName )
{
    lockClientList();
    std::vector<WebPageCallbackInterface*>::iterator iter;
    for( iter = m_WebPageClients.begin(); iter != m_WebPageClients.end(); ++iter )
    {
        WebPageCallbackInterface* client = *iter;
        client->callbackWebDownloadComplete( webPageType, onlineId, fileName );
    }

    unlockClientList();
}

//============================================================================
void WebPageMgr::announceWebDownloadFailed( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, enum EXferError xferErr )
{
	lockClientList();
	std::vector<WebPageCallbackInterface *>::iterator iter;
	for( iter = m_WebPageClients.begin();	iter != m_WebPageClients.end(); ++iter )
	{
		WebPageCallbackInterface * client = *iter;
        client->callbackWebDownloadFailed( webPageType, onlineId, fileName, xferErr );
	}

	unlockClientList();
}
