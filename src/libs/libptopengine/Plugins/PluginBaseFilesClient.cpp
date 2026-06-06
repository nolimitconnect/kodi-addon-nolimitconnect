//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseFilesClient.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <ConnectIdListMgr/ConnectIdListMgr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsFileInfo.h>

//============================================================================
PluginBaseFilesClient::PluginBaseFilesClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, std::string fileInfoDbName )
: PluginBaseFiles( engine, pluginMgr, myIdent, pluginType, m_FileInfoClientFilesMgr )
, m_FileInfoClientFilesMgr( engine, *this, fileInfoDbName )
{
}

//============================================================================
bool PluginBaseFilesClient::connectForWebPageDownload( VxGUID& onlineId )
{
    bool result{ false };

    m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgConnecting, "" );

    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( onlineId );	// id of friend to look for
    if( netIdent )
    {
        std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
        if( sktBase )
        {
            result = onConnectForWebPageDownload( sktBase, onlineId );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseFilesClient::connectForWebPageDownload online id not found" );
    }

    if( result )
    {
        m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgRetrieveInfo, "" );
    }
    else
    {
        m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgConnectFailed, "" );
    }

    return result;
}

//============================================================================
bool PluginBaseFilesClient::onConnectForWebPageDownload( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    m_HisOnlineId = onlineId;
    m_SearchSessionId.initializeWithNewVxGUID();

    PktFileInfoSearchReq pktReq;
    pktReq.setHostOnlineId( m_HisOnlineId );
    pktReq.setSearchSessionId( m_SearchSessionId );
    m_SktConnectionId = sktBase->getSocketId();

    return txPacket( m_HisOnlineId, sktBase, &pktReq );
}

//============================================================================
bool PluginBaseFilesClient::connectForFileListDownload( VxGUID& onlineId )
{
    bool result{ false };
    m_HisOnlineId = onlineId;
    m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgConnecting, "" );

    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( onlineId );	// id of friend to look for
    if( netIdent )
    {
        std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
        if( sktBase )
        {
            result = onConnectForFileListDownload( sktBase, onlineId );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "PluginBaseFilesClient::connectForWebPageDownload online id not found" );
    }

    if( result )
    {
        m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgRetrieveInfo, "" );
    }
    else
    {
        m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgConnectFailed, "" );
    }

    return result;
}

//============================================================================
bool PluginBaseFilesClient::onConnectForFileListDownload( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    m_HisOnlineId = onlineId;
    m_SktConnectionId = sktBase->getSocketId();
    if( !m_SearchSessionId.isValid() )
    {
        m_SearchSessionId.initializeWithNewVxGUID();
    }
    
    PktFileInfoSearchReq pktReq;
    pktReq.setHostOnlineId( m_HisOnlineId );
    pktReq.setSearchSessionId( m_SearchSessionId );
    pktReq.setSearchFileTypes( getSearchFileTypes() );

    return txPacket( m_HisOnlineId, sktBase, &pktReq );
}

