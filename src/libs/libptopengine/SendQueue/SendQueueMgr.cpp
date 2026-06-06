//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SendQueueMgr.h"

#include "SendQueueCallback.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginMessenger.h>
#include <Plugins/PluginMgr.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <algorithm>

namespace
{
    const int SEND_QUEUE_DB_VERSION = 1;
}

//============================================================================
void SendQueueMgr::sendQueueStartup( P2PEngine* engine, std::string dbName )
{
    auto weakPtr = std::make_shared<P2PEngine*>(engine);
    m_EnginePtr = weakPtr;
    m_SendQueueDb.sendQueueDbStartup( dbName );
}

//============================================================================
void SendQueueMgr::onEngineInitialized( void )
{
    PluginBase* messenger = GetPtoPEngine().getPluginMgr().getPlugin( ePluginTypeMessenger );
    if( messenger )
    {
        ((PluginMessenger*)messenger)->wantAssetXferCallbacks( this, true );
    }
}

//============================================================================
void SendQueueMgr::fromGuiUserLoggedOn( void )
{
    if( !m_SendQueueInitialized )
    {
        m_SendQueueInitialized = true;

        std::string dbFileName = VxGetSettingsDirectory();
        dbFileName += "SendQueue.db3";

        lockSendList();
        m_SendQueueDb.dbShutdown();
        m_SendQueueDb.dbStartup( SEND_QUEUE_DB_VERSION, dbFileName );
        m_SendQueueDb.getAllQueInfo( m_SendList );

        for( auto& sendQueInfo : m_SendList )
        {
            announceSendQueue( sendQueInfo );
        }

        unlockSendList();
    }
}

//============================================================================
void SendQueueMgr::removeAsset( VxGUID& assetId )
{
    std::vector<SendQueInfo> removedList;

    lockSendList();
    for( auto iter = m_SendList.begin(); iter != m_SendList.end(); )
    {
        if( iter->getAssetId() == assetId )
        {
            iter->setSendQueState( eSendQueStateSendRemove );
            removedList.emplace_back( *iter );
            m_SendQueueDb.removeSendQueueInfo( iter->getGroupieId() );
            iter = m_SendList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockSendList();

    lockClientList();
    for( auto& sendQueInfo : removedList )
    {
        for( auto sendClient : m_MemberClients )
        {
            sendClient->callbackSendQueInfo( sendQueInfo );
        }
    }

    unlockClientList();
}

//============================================================================
void SendQueueMgr::removeSentAsset( VxGUID& onlinId, VxGUID& assetId )
{
    std::vector<SendQueInfo> sentList;

    lockSendList();
    for( auto iter = m_SendList.begin(); iter != m_SendList.end(); )
    {
        if( iter->getAssetId() == assetId && iter->getUserOnlineId() == onlinId )
        {
            iter->setSendQueState( eSendQueStateSendRemove );
            sentList.emplace_back( *iter );
            m_SendQueueDb.removeSendQueueInfo( iter->getGroupieId() );
            iter = m_SendList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockSendList();

    lockClientList();
    for( auto& sendQueInfo : sentList )
    {
        for( auto sendClient : m_MemberClients )
        {
            sendClient->callbackSendQueInfo( sendQueInfo );
        }
    }

    unlockClientList();
}

//============================================================================
bool SendQueueMgr::haveQueuedAssetToSend( VxGUID& onlineId )
{
    lockSendList();
    auto iter = std::find_if(m_SendList.begin(), m_SendList.end(),
                              [&](SendQueInfo& x) { return x.getUserOnlineId() == onlineId; });
    bool needToSend = iter != m_SendList.end();
    unlockSendList();

    return needToSend;
}

//============================================================================
bool SendQueueMgr::updateSendQueue( VxGUID& onlineId, VxGUID& assetId, enum ESendQueState sendState )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "SendQueueMgr::%s invalid online id", __func__ );
        return false;
    }

    if( !assetId.isValid() )
    {
        LogMsg( LOG_ERROR, "SendQueueMgr::%s invalid asset id", __func__ );
        return false;
    }

    LogMsg( LOG_VERBOSE, "SendQueueMgr::%s asset %s to %s", __func__, assetId.toHexString().c_str(),
            GetPtoPEngine().describeUser( onlineId ).c_str() );

    SendQueInfo sendQueInfo( onlineId, assetId, sendState );
    bool wasUpdated = false;
    lockSendList();

    bool found{ false };
    bool wasRemoved{ false };
    for( auto iter = m_SendList.begin(); iter != m_SendList.end(); ++iter )
    {
        if( iter->getAssetId() == assetId && iter->getUserOnlineId() == onlineId )
        {
            found = true;
            if( eSendQueStateSendRemove == sendState )
            {
                GroupieId groupieId( onlineId, assetId, eHostTypePeerUser );
                m_SendQueueDb.removeSendQueueInfo( groupieId );
                m_SendList.erase( iter );
                wasUpdated = true;
                wasRemoved = true;
            }
            else if( iter->getSendQueState() != sendState )
            {
                iter->setSendQueState( sendState );
                m_SendQueueDb.updateSendQueueInfo( sendQueInfo );
                wasUpdated = true;
            }

            break;
        }
    }

    if( eSendQueStateSendRemove != sendState && !found )
    {
        m_SendList.emplace_back( sendQueInfo );
        if( 0 == m_SendQueueDb.updateSendQueueInfo( sendQueInfo ) )
        {
            wasUpdated = true;
        }
    }

    unlockSendList();

    if( wasUpdated )
    {
        announceSendQueue( sendQueInfo );
    }

    return true;
}

//============================================================================
bool SendQueueMgr::isInSendQueue( VxGUID& onlineId, VxGUID& assetId )
{
    lockSendList();
    auto iter = std::find_if(m_SendList.begin(), m_SendList.end(),
                             [&](SendQueInfo& x) { return x.getUserOnlineId() == onlineId &&
                                                    x.getAssetId() == assetId; });
    bool isActive = iter != m_SendList.end();
    unlockSendList();

    return isActive;
}

//============================================================================
void SendQueueMgr::wantSendQueueCallbacks( SendQueueCallback* client, bool enable )
{
    lockClientList();

    bool found{ false };
    for( auto iter = m_MemberClients.begin(); iter != m_MemberClients.end(); ++iter )
    {
        if( *iter == client )
        {
            found = true;
            if( !enable )
            {
                m_MemberClients.erase( iter );
            }
            else
            {
                LogMsg( LOG_ERROR, "SendQueueMgr::%s ignored because already in list", __func__ );
            }

            break;
        }
    }
    
    if( !found && enable )
    {
        m_MemberClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void SendQueueMgr::announceSendQueue( SendQueInfo& sendQueInfo )
{   
    lockClientList();

    for( auto sendClient : m_MemberClients )
    {
        sendClient->callbackSendQueInfo( sendQueInfo );
    }

    unlockClientList();
}

//============================================================================
void SendQueueMgr::pktAnnRecieved( VxGUID& onlineId )
{
    sendNextAsset( onlineId );
}

//============================================================================
void SendQueueMgr::callbackAssetXferReadyToSend( VxGUID& sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    sendNextAsset( sendToId );
}

//============================================================================
void SendQueueMgr::sendNextAsset( VxGUID& onlineId )
{
    SendQueInfo sendQueInfo;
    bool foundToSend{ false };
    AssetBaseInfo assetInfo;
    std::vector<SendQueInfo> toRemoveList;

    VxMutex& assetLock = GetPtoPEngine().getAssetMgr().getResourceMutex();
    lockSendList();
    for( auto iter = m_SendList.begin(); iter != m_SendList.end(); )
    {
        if( iter->getUserOnlineId() == onlineId && 
            iter->getSendQueState() == eSendQueStateWaiting || iter->getSendQueState() == eSendQueStateSendFailed )
        {
            VxGUID assetId = iter->getAssetId();
            assetLock.lock();
            AssetBaseInfo* assetInfoPtr = GetPtoPEngine().getAssetMgr().findAsset( assetId );
            if( assetInfoPtr && assetInfoPtr->isValid() )
            {
                assetInfo = *assetInfoPtr;
                assetInfo.setPluginType( ePluginTypeMessenger );
                assetInfo.setDestUserId( onlineId );
                foundToSend = true;
            }

            assetLock.unlock();
            if( foundToSend )
            {
                break;
            }
            else
            {
                // no longer a valid asset
                toRemoveList.emplace_back( *iter );
                iter = m_SendList.erase( iter );
            }
        }
        else
        {
            iter++;
        }
    }

    unlockSendList();

    if( foundToSend )
    {
        GetPtoPEngine().fromGuiSendAsset( assetInfo );
    }

    if( toRemoveList.size() )
    {
        // remove invalid asset sends
        lockSendList();
        for( auto sendInfo : toRemoveList )
        {
            m_SendQueueDb.removeSendQueueInfo( sendInfo.getGroupieId() );
        }

        unlockSendList();
    }
}

//============================================================================
void SendQueueMgr::callbackXferState( VxGUID& sendToId, VxGUID& assetId, enum EAssetSendState sendState, int param )
{
    if( eAssetSendStateTxSuccess == sendState )
    {
        removeSentAsset( sendToId, assetId );
    }
    else if( eAssetSendStateTxFail == sendState )
    {
        updateSendQueue( sendToId, assetId, eSendQueStateSendFailed );
    }
}
