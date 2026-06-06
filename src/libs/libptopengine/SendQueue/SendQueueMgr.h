#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SendQueueCallback.h"
#include "SendQueueDb.h"

#include <AssetBase/AssetXferCallback.h>

#include <CoreLib/VxMutex.h>
#include <CoreLib/GroupieId.h>

#include <vector>
#include <memory>

class SendQueueCallback;
class P2PEngine;

// class to manage assets that need to be send once we are connected again
// NOTE: for this class GroupieId::getUserOnlineId is who to send to and
//       GroupieId::getHostOnlineId is the asset id

class SendQueueMgr : public AssetXferCallback
{
public:
    SendQueueMgr() = default;
    virtual ~SendQueueMgr() = default;

    void                        sendQueueStartup( P2PEngine* engine, std::string dbName );
    void                        onEngineInitialized( void );
    void                        fromGuiUserLoggedOn( void );

    void                        removeAsset( VxGUID& assetId );
    void                        removeSentAsset( VxGUID& onlineId, VxGUID& assetId );
    bool                        updateSendQueue( VxGUID& onlineId, VxGUID& assetId, enum ESendQueState sendState );

    bool                        isInSendQueue( VxGUID& onlineId, VxGUID& assetId );

    bool                        haveQueuedAssetToSend( VxGUID& onlineId );

    void                        pktAnnRecieved( VxGUID& onlineId );

    void                        wantSendQueueCallbacks( SendQueueCallback* client, bool enable );

    std::vector<SendQueInfo>&   getSendQueueList( void )        { return m_SendList; };

protected:
    void				        callbackAssetXferReadyToSend( VxGUID& sendToId, std::shared_ptr<VxSktBase>& sktBase ) override;
    void				        callbackXferState( VxGUID& sendToId, VxGUID& assetId, enum EAssetSendState sendState, int param ) override;

    void                        lockSendList( void )            { m_SendListMutex.lock(); }
    void                        unlockSendList( void )          { m_SendListMutex.unlock(); }

    void                        lockClientList( void )          { m_MemberClientsMutex.lock(); }
    void                        unlockClientList( void )        { m_MemberClientsMutex.unlock(); }

    void                        announceSendQueue( SendQueInfo& sendQueInfo );

    void                        sendNextAsset( VxGUID& onlineId );
    
    std::weak_ptr<P2PEngine*>   m_EnginePtr;
    SendQueueDb                 m_SendQueueDb;

    VxMutex                     m_SendListMutex;
    std::vector<SendQueInfo>    m_SendList;

    VxMutex                     m_MemberClientsMutex;
    std::vector<SendQueueCallback*> m_MemberClients;

    bool                        m_SendQueueInitialized{ false };
};

