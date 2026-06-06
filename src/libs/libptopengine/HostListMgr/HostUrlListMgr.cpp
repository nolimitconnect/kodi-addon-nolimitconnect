//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostUrlListMgr.h"
#include <GuiInterface/IDefs.h>
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>
#include <NetLib/VxSktBase.h>

//============================================================================
HostUrlListMgr::HostUrlListMgr( P2PEngine& engine )
    : m_Engine( engine )
{
}

//============================================================================
int32_t HostUrlListMgr::hostUrlListMgrStartup( std::string& dbFileName )
{
    int32_t rc = m_HostUrlListDb.hostUrlListDbStartup( HOST_URL_LIST_DB_VERSION, dbFileName.c_str() );
    m_HostUrlsList.clear();
    m_HostUrlListDb.getAllHostUrls( m_HostUrlsList );
    return rc;
}

//============================================================================
int32_t HostUrlListMgr::hostUrlListMgrShutdown( void )
{
    return m_HostUrlListDb.hostUrlListDbShutdown();
}

//============================================================================
void HostUrlListMgr::updateHostUrl( enum EHostType hostType, VxGUID& onlineId, std::string& hostUrl, int64_t timestampMs )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "HostUrlListMgr::updateDirectConnectIdent invalid id" );
        return;
    }

    bool wasUpdated = false;
    lockList();
    for( auto iter = m_HostUrlsList.begin(); iter != m_HostUrlsList.end(); ++iter )
    {
        if( iter->getHostType() == hostType && iter->getOnlineId() == onlineId )
        {
            iter->setHostUrl( hostUrl );
            if( timestampMs )
            {
                iter->setTimestamp( timestampMs );
                m_HostUrlListDb.saveHostUrl( *iter );
            }
            
            wasUpdated = true;
            break;
        }
    }

    if( !wasUpdated )
    {
        std::string emptyUrl;
        HostUrlInfo hostUrlInfo( hostType, onlineId, hostUrl, timestampMs );
        m_HostUrlsList.emplace_back( hostUrlInfo );
        if( timestampMs )
        {
            m_HostUrlListDb.saveHostUrl( hostUrlInfo );
        }
    }

    unlockList();
}

//============================================================================
bool HostUrlListMgr::getHostUrls( EHostType hostType, std::vector<HostUrlInfo>& retHostUrls )
{
    retHostUrls.clear();
    lockList();
    for( auto iter = m_HostUrlsList.begin(); iter != m_HostUrlsList.end(); ++iter )
    {
        if( iter->getHostType() == hostType )
        {
            retHostUrls.emplace_back( *iter );
        }
    }

    unlockList();

    return !retHostUrls.empty();
}


//============================================================================
bool HostUrlListMgr::getResolvedHostUrl( EHostType hostType, VxGUID& hostOnlineId, std::string& retHostUrl )
{
    retHostUrl.clear();
    lockList();
    for( auto urlInfo : m_HostUrlsList )
    {
        if( urlInfo.getHostType() == hostType && urlInfo.getOnlineId() == hostOnlineId )
        {
            retHostUrl = urlInfo.getHostUrl();
            break;
        }
    }

    unlockList();

    return !retHostUrl.empty();
}


//============================================================================
/// return false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
bool HostUrlListMgr::onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, EConnectReason connectReason ) 
{ 
    if( eConnectReasonRequestIdentity == connectReason )
    {
        BigListInfo* bigListInfo = m_Engine.getBigListMgr().findBigListInfo( onlineId );
        if( bigListInfo )
        {
            updateHostUrls( bigListInfo->getVxNetIdent(), sktBase->getLastActiveTimeMs() );
            m_Engine.getToGui().toGuiContactAdded( bigListInfo->getVxNetIdent() );
        }

        m_Engine.getConnectionMgr().doneWithConnection( sessionId, onlineId, this, connectReason );
    }

    return false; 
}

//============================================================================
void HostUrlListMgr::requestIdentity( std::string& url )
{
    VxPtopUrl ptopUrl( url );
    if( ptopUrl.isValid() )
    {
        // just make up any session.. we only care about the identity then will disconnect
        VxGUID sessionId;
        sessionId.initializeWithNewVxGUID();
        std::shared_ptr<VxSktBase> sktBase( nullptr );
        m_Engine.getConnectionMgr().requestConnection( sessionId, ptopUrl.getUrl(), ptopUrl.getOnlineId(), this, sktBase, eConnectReasonRequestIdentity );
    }
}

//============================================================================
void HostUrlListMgr::updateHostUrls( VxNetIdent* netIdent, int64_t timestampMs )
{
    if( !netIdent )
    {
        LogMsg( LOG_ERROR, "HostUrlListMgr::updateHostUrls null netIdent" );
        return;
    }

    if( netIdent->requiresRelay() )
    {
        removeClosedPortIdent( netIdent->getMyOnlineId() );
    }
    else
    {     
        std::string nodeUrl = netIdent->getMyOnlineUrl();
        for( int i = eHostTypeUnknown + 1; i < eMaxHostType; ++i )
        {
            EHostType hostType = ( EHostType )i;
            if( netIdent->canRequestJoin( hostType ) )
            {
                updateHostUrl( hostType, netIdent->getMyOnlineId(), nodeUrl, timestampMs );
            }
        }
    }
}

//============================================================================
void HostUrlListMgr::removeClosedPortIdent( VxGUID& onlineId )
{
    lockList();
    for( auto iter = m_HostUrlsList.begin(); iter != m_HostUrlsList.end(); )
    {
        if( iter->getOnlineId() == onlineId )
        {
            iter = m_HostUrlsList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockList();
    m_HostUrlListDb.removeClosedPortIdent( onlineId );
}
