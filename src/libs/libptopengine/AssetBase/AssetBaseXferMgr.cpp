//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "AssetBaseXferMgr.h"

#include "AssetBaseInfo.h"
#include "AssetBaseMgr.h"
#include "AssetXferCallback.h"

#include "../Plugins/PluginBase.h"
#include "../Plugins/PluginMgr.h"
#include "../Plugins/PluginMessenger.h"
#include "AssetBaseTxSession.h"
#include "AssetBaseRxSession.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <AssetMgr/AssetMgr.h>
#include <ConnectIdListMgr/ConnectIdListMgr.h>
#include <Plugins/FileInfo.h>

#include <PktLib/PktsAssetXfer.h>
#include <PktLib/VxCommon.h>
#include <NetLib/VxSktBase.h>

#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/AppErr.h>
#include <CoreLib/VxFileUtil.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG_AUTOPLUGIN_LOCK 1

namespace
{
    const int MAX_OUTSTANDING_STREAM_PKTS = 25;

    //============================================================================
    static void * AssetBaseXferMgrThreadFunc( void * pvContext )
    {
        VxThread* poThread = (VxThread*)pvContext;
        poThread->setIsThreadRunning( true );
        AssetBaseXferMgr * poMgr = (AssetBaseXferMgr *)poThread->getThreadUserParam();
        if( poMgr )
        {
            poMgr->assetXferThreadWork( poThread );
        }

        poThread->threadAboutToExit();
        return nullptr;
    }
}


//============================================================================
AssetBaseXferMgr::AssetBaseXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface )
: m_Engine( engine )
, m_AssetBaseMgr( assetMgr )
, m_XferInterface( xferInterface )
, m_PluginMgr( engine.getPluginMgr() )
, m_AssetBaseXferDb( xferInterface.getAssetXferDbName() )
, m_WorkerThreadName( xferInterface.getAssetXferThreadName() )
{
}

//============================================================================
AssetBaseXferMgr::~AssetBaseXferMgr()
{
    clearRxSessionsList();
    clearTxSessionsList();
}

//============================================================================
void AssetBaseXferMgr::fromGuiUserLoggedOn( void )
{
    if( !m_Initialized )
    {
        m_Initialized = true;
        m_WorkerThread.startThread( (VX_THREAD_FUNCTION_T)AssetBaseXferMgrThreadFunc, this, m_WorkerThreadName.c_str() );
    }
}

//============================================================================
void AssetBaseXferMgr::assetXferThreadWork( VxThread* workThread )
{
    if( workThread->isAborted() )
        return;
    // user specific directory should be set
    std::string dbName = VxGetSettingsDirectory();
    dbName += m_AssetBaseXferDb.getDatabaseName();
    lockAssetBaseQue();
    m_AssetBaseXferDb.dbShutdown();
    m_AssetBaseXferDb.dbStartup( 1, dbName );
    unlockAssetBaseQue();
    if( workThread->isAborted() )
        return;

    std::vector<VxGUID> assetToSendList;
    m_AssetBaseXferDb.getAllAssets( assetToSendList );
    if( 0 == assetToSendList.size() )
    {
        // nothing to do
        return;
    }

    while( ( false == m_AssetBaseMgr.isAssetListInitialized() )
            && ( false == workThread->isAborted() ) )
    {
        // waiting for assets to be available
        VxSleep( 500 );
    }

    if( workThread->isAborted() )
        return;

    std::vector<VxGUID>::iterator iter;
    m_AssetBaseMgr.lockResources();
    lockAssetBaseQue();
    for( iter = assetToSendList.begin(); iter != assetToSendList.end(); ++iter )
    {
        AssetBaseInfo* assetInfo = m_AssetBaseMgr.findAsset( *iter );
        if( assetInfo )
        {
            m_AssetBaseSendQue.emplace_back( *assetInfo );
        }
        else
        {
            LogMsg( LOG_ERROR, "assetXferThreadWork removing asset not found in list" );
            m_AssetBaseXferDb.removeAsset( *iter );
        }
    }

    unlockAssetBaseQue();
    m_AssetBaseMgr.unlockResources();
}

//============================================================================
void AssetBaseXferMgr::fromGuiCancelDownload( VxGUID& lclSessionId )
{
    std::map<VxGUID, AssetBaseRxSession*>::iterator iter;
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelDownload AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelDownload AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    iter = m_RxSessions.find( lclSessionId );
    if( iter != m_RxSessions.end() )
    {
        AssetBaseRxSession* xferSession = iter->second;
        if( xferSession->getLclSessionId() == lclSessionId )
        {
            if(LogEnabled(eLogFileXfer))LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s erasing lcl session id %s",
                          __func__, DescribePluginType( getPluginType() ), xferSession->getLclSessionId().toHexString().c_str() );
            m_RxSessions.erase( iter );
            xferSession->cancelDownload( lclSessionId );
            delete xferSession;
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelDownload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::fromGuiCancelUpload( VxGUID& lclSessionId )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelUpload AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelUpload AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* xferSession = ( *iter );
        if( xferSession->getLclSessionId() == lclSessionId )
        {
            m_TxSessions.erase( iter );
            xferSession->cancelUpload( lclSessionId );
            delete xferSession;
#ifdef DEBUG_AUTOPLUGIN_LOCK
            LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelUpload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
            return;
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fromGuiCancelUpload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}


//============================================================================
void AssetBaseXferMgr::clearRxSessionsList( void )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::clearRxSessionsList AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::clearRxSessionsList AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    for( auto iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
    {
        AssetBaseRxSession* xferSession = iter->second;
        delete xferSession;
    }

    m_RxSessions.clear();
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::clearRxSessionsList AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::clearTxSessionsList( void )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktBaseSendReq AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktBaseSendReq AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* xferSession = (*iter);
        delete xferSession;
    }

    m_TxSessions.clear();
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktBaseSendReq AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::fileAboutToBeDeleted( std::string& fileName )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fileAboutToBeDeleted AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fileAboutToBeDeleted AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* xferSession = ( *iter );
        if( xferSession->getXferInfo().getLclFileName() == fileName )
        {
            m_TxSessions.erase( iter );
            xferSession->cancelUpload( xferSession->getXferInfo().getLclSessionId() );
            delete xferSession;
#ifdef DEBUG_AUTOPLUGIN_LOCK
            LogMsg( LOG_INFO, "AssetBaseXferMgr::fileAboutToBeDeleted AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
            return;
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::fileAboutToBeDeleted AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onConnectionLost AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onConnectionLost AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    bool erasedSession = true;
    while( erasedSession )
    {
        erasedSession = false;
        for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
        {
            AssetBaseTxSession* xferSession = ( *iter );
            if( xferSession->getSkt() == sktBase )
            {
                m_TxSessions.erase( iter );
                xferSession->cancelUpload( xferSession->getXferInfo().getLclSessionId() );
                delete xferSession;
                erasedSession = true;
                break;
            }
        }
    }

    erasedSession = true;
    while( erasedSession )
    {
        erasedSession = false;
        for( auto oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
        {
            AssetBaseRxSession* xferSession = oRxIter->second;
            if( xferSession->getSkt() == sktBase )
            {
                if(LogEnabled(eLogFileXfer))LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s erasing lcl session id %s",
                              __func__, DescribePluginType( getPluginType() ), xferSession->getLclSessionId().toHexString().c_str() );
                m_RxSessions.erase( oRxIter );
                xferSession->cancelDownload( xferSession->getXferInfo().getLclSessionId() );
                delete xferSession;
                erasedSession = true;
                break;
            }
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onConnectionLost AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
bool AssetBaseXferMgr::requireFileXfer( EAssetType assetType )
{
    return ( ( eAssetTypePhoto == assetType )
        || ( eAssetTypeAudio == assetType )
        || ( eAssetTypeVideo == assetType )
        || ( eAssetTypeDocument == assetType )
        || ( eAssetTypeArchives == assetType )
        || ( eAssetTypeExe == assetType )
        || ( eAssetTypeOtherFiles == assetType )
        || ( eAssetTypeThumbnail == assetType )
        || ( eAssetTypeCamRecord == assetType ) );
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    EXferError xferErr = eXferErrorNone;
    PktBaseGetReq * pktGetReq = (PktBaseGetReq *)pktHdr;

    VxGUID assetUniqueId = pktGetReq->getUniqueId();
    EAssetType assetType = (EAssetType)pktGetReq->getAssetType();

    VxGUID rmtSessionId = pktGetReq->getLclSessionId();
    VxGUID lclSessionId;
    lclSessionId.initializeWithNewVxGUID();
    if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s rmt session id %s lcl session id %s", __func__,
                  DescribePluginType( getPluginType() ), rmtSessionId.toHexString().c_str(), lclSessionId.toHexString().c_str() );
    int64_t startOffs = pktGetReq->getAssetOffset();

    PktBaseGetReply* pktReply = createPktBaseGetReply();
    pktReply->setIsStream( pktGetReq->getIsStream() );
    pktReply->setAssetType( pktGetReq->getAssetType() );
    pktReply->setUniqueId( pktGetReq->getUniqueId() );
    pktReply->setSendToId( pktGetReq->getSendToId() );
    pktReply->setLclSessionId( lclSessionId );
    pktReply->setRmtSessionId( rmtSessionId );
    pktReply->setAssetOffset( startOffs );
    pktReply->setIsTemporary( pktGetReq->getIsTemporary() );

    if( !pktGetReq->isValidPktPrefix() )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetBaseGetReq Invalid Packet" );
        vx_assert( false );
        pktReply->setError( eXferErrorBadParam );
        m_XferInterface.txPacket( pktHdr->getSrcOnlineId(), sktBase, pktReply, m_XferInterface.getAssetOverridePluginType() );
        delete pktReply;
        return;
    }

    if( !(eAssetTypeThumbnail == assetType) && !netIdent->isHisAccessAllowedFromMe( m_XferInterface.getPluginType() ) )
    {
        pktReply->setError( eXferErrorPermission );
        m_XferInterface.txPacket( pktHdr->getSrcOnlineId(), sktBase, pktReply, m_XferInterface.getAssetOverridePluginType() );
        delete pktReply;
        return;
    }

    if( !assetUniqueId.isValid() || !rmtSessionId.isValid()  || !lclSessionId.isValid() )
    {
        pktReply->setError( eXferErrorBadParam );
        m_XferInterface.txPacket( pktHdr->getSrcOnlineId(), sktBase, pktReply, m_XferInterface.getAssetOverridePluginType() );
        delete pktReply;
        return;
    }

    AssetBaseInfo* assetInfo = m_AssetBaseMgr.findAsset( assetUniqueId );
    if( !assetInfo )
    {
        pktReply->setError( eXferErrorFileNotFound );
        m_XferInterface.txPacket( pktHdr->getSrcOnlineId(), sktBase, pktReply, m_XferInterface.getAssetOverridePluginType());
        delete pktReply;
        return;
    }

    if( !m_AssetBaseMgr.doesAssetExist( *assetInfo ) )
    {
        pktReply->setError( eXferErrorFileNotFound );
        m_XferInterface.txPacket( pktHdr->getSrcOnlineId(), sktBase, pktReply, m_XferInterface.getAssetOverridePluginType() );
        delete pktReply;
        return;
    }

    if( !pktReply->fillPktFromAsset( *assetInfo ) )
    {
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s fillPktFromAsset Error", __func__ );
        vx_assert( false );
        pktReply->setError( eXferErrorFileNotFound );
        m_XferInterface.txPacket( pktGetReq->getSrcOnlineId(), sktBase, pktReply );
        delete pktReply;
        return;
    }


    VxMutex& assetMutex = m_XferInterface.getAssetXferMutex();
    assetMutex.lock();

    AssetBaseTxSession* xferSession = findOrCreateTxSession( true, lclSessionId, pktGetReq->getSrcOnlineId(), sktBase);
    if( !xferSession )
    {
        pktReply->setError( eXferErrorBusy );
        m_XferInterface.txPacket( pktGetReq->getSrcOnlineId(), sktBase, pktReply);
        assetMutex.unlock();
        delete pktReply;
        return;
    }

    xferSession->setAssetBaseInfo( *assetInfo );
    std::string lclFileName = assetInfo->getAssetName();
    std::string lclFileNameAndPath = assetInfo->getAssetNameAndPath();
    std::string rmtFileName = lclFileName;

    if( lclFileNameAndPath.empty() || rmtFileName.empty() )
    {
        LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetBaseGetReq Asset File Name Error" );
        vx_assert( false );
        pktReply->setError( eXferErrorFileNotFound );
        m_XferInterface.txPacket( pktGetReq->getSrcOnlineId(), sktBase, pktReply);
        assetMutex.unlock();
        delete pktReply;
        return;
    }

    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    xferInfo.setLclSessionId( lclSessionId );
    xferInfo.setRmtSessionId( rmtSessionId );
    xferInfo.setAssetId( assetInfo->getAssetUniqueId() );
    xferInfo.setAssetType( assetInfo->getAssetType() );
    xferInfo.setFileHashId( pktGetReq->getFileHashId() );
    xferInfo.setFileOffset( pktGetReq->getAssetOffset() );
    xferInfo.setLclFileName( lclFileName.c_str() );
    xferInfo.setLclFileNameAndPath( lclFileNameAndPath.c_str() );
    xferInfo.setRmtFileName( rmtFileName.c_str() );
    addTxSession( xferSession );

    assetMutex.unlock();
    xferErr  = ( m_XferInterface.txPacket( pktGetReq->getSrcOnlineId(), sktBase, pktReply ) ) ? eXferErrorNone : eXferErrorDisconnected;
    assetMutex.lock();
    if( eXferErrorNone == xferErr )
    {
        xferErr = beginAssetBaseSend( xferSession );
        if( eXferErrorNone == xferErr )
        {
            // send first chunk
            assetMutex.unlock();
            if( xferSession->getIsStream() )
            {
                for( int i = 0; i < MAX_OUTSTANDING_STREAM_PKTS; i++ )
                {
                    // try to send more than acknowleged so video does not stutter so much
                    xferErr = txNextAssetBaseChunk( xferSession, 0, false );
                    if( eXferErrorNone != xferErr )
                    {
                        break;
                    }
                }
            }
            else
            {
                xferErr = txNextAssetBaseChunk( xferSession, 0, false );
            }
            
            assetMutex.lock();
        }
    }

    delete pktReply;
    if( eXferErrorNone != xferErr )
    {
        onAssetBaseUploadError( pktGetReq->getSrcOnlineId(), *assetInfo, xferErr);
        if( eXferErrorCanceled != xferErr )
        {
            endAssetBaseXferSession( xferSession, true, false );
        }
    }
    else
    {
        onAssetBaseBeginUpload( pktGetReq->getSrcOnlineId(), *assetInfo );
    }

    assetMutex.unlock();
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktBaseGetReply * pktGetReply = (PktBaseGetReply *)pktHdr;
    if( !pktGetReply->isValidPktPrefix() )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Invalid Packet", __func__ );
        vx_assert( false );
        return;
    }

    EAssetType assetType = (EAssetType)pktGetReply->getAssetType();
    if( !VxIsValidAssetType( assetType ) )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Invalid Asset Type %d", __func__, assetType );
        vx_assert( false );
        return;
    }

    EXferError xferErr = (EXferError)pktGetReply->getError();
    if( xferErr != eXferErrorNone )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Xfer Error %d", __func__, xferErr );// %s, DescribeXferError( xferErr ) );
        vx_assert( false );
        return;
    }

    // thumbnails always allowed
    if( !(eAssetTypeThumbnail == assetType) && !netIdent->isHisAccessAllowedFromMe( m_XferInterface.getPluginType() ) )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Permission Error from %s asset type %s", __func__, netIdent->getOnlineName(), DescribeAssetType( assetType ) );
        vx_assert( false );
        return;
    }

    VxGUID assetUniqueId = pktGetReply->getUniqueId();
    bool needFileXfer = requireFileXfer( assetType );

    VxGUID rmtSessionId = pktGetReply->getLclSessionId();
    VxGUID lclSessionId = pktGetReply->getRmtSessionId();
    if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "%s %s rmt session id %s lcl session id %s", __func__, DescribePluginType( getPluginType() ),
           rmtSessionId.toHexString().c_str(), lclSessionId.toHexString().c_str() );
    int64_t	startOffs = pktGetReply->getAssetOffset();
    int64_t	endOffs = pktGetReply->getAssetLen();

    if( !assetUniqueId.isValid() || !rmtSessionId.isValid() || !lclSessionId.isValid() )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Invalid asset or session id", __func__ );
        vx_assert( false );
        return;
    }

    if( needFileXfer && !endOffs )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s No File Length", __func__ );
        vx_assert( false );
        return;
    }

    if( (eAssetTypeThumbnail != assetType) && !netIdent->isHisAccessAllowedFromMe( m_XferInterface.getPluginType() ) )
    {
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s: permission denied", __func__ );
        return;
    }

    if( false == needFileXfer )
    {
        // all we need is in the send request
        AssetBaseInfo assetInfo;
        pktGetReply->fillAssetFromPkt( assetInfo );
        // make history id his id
        assetInfo.setHistoryId( pktGetReply->getSrcOnlineId() );
        assetInfo.setAssetSendState( eAssetSendStateRxSuccess );
        AssetBaseInfo* createdAsset = nullptr;
        if( !m_AssetBaseMgr.addAsset( assetInfo, createdAsset ) )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s: failed add asset", __func__ );
        }

        sendToGuiAssetAction( eAssetActionRxSuccess, assetInfo.getAssetUniqueId(), 100 );
        sendToGuiAssetAction( eAssetActionRxNotifyNewMsg, assetInfo.getCreatorId(), 100 );
    }
    else
    {
        AssetBaseRxSession* xferSession = findOrCreateRxSession( true, lclSessionId, pktGetReply->getSrcOnlineId(), sktBase );
        if( xferSession )
        {
            AssetBaseInfo& assetInfo = xferSession->getAssetBaseInfo();
            pktGetReply->fillAssetFromPkt( assetInfo );
            // make history id his id
            assetInfo.setHistoryId( netIdent->getMyOnlineId() );
            assetInfo.setAssetSendState( eAssetSendStateRxProgress );

            xferSession->setRmtSessionId( pktGetReply->getLclSessionId() );
            pktGetReply->setLclSessionId( xferSession->getLclSessionId() );

            xferSession->setIsStream( pktGetReply->getIsStream() );

            EXferError xferErr = beginAssetBaseReceive( xferSession, assetInfo, rmtSessionId, startOffs );
            if( eXferErrorNone != xferErr )
            {
                //sendToGuiUpdateAssetDownload( xferSession->getLclSessionId(), 0, rc );
                endAssetBaseXferSession( xferSession, true );
            }
        }
        else
        {
            LogMsg(LOG_ERROR, "PluginAssetBaseOffer::%s: Could not create session", __func__ );
        }
    }
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    #ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReq AutoPluginLock start");
    #endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
    #ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReq AutoPluginLock done");
    #endif // DEBUG_AUTOPLUGIN_LOCK

    PktBaseSendReq* poPkt = (PktBaseSendReq *)pktHdr;
    if( !poPkt->isValidPktPrefix() )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendReq Invalid Packet" );
        vx_assert( false );
        return;
    }

    EAssetType assetType = (EAssetType)poPkt->getAssetType();
    if( !VxIsValidAssetType( assetType ) )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s Invalid Asset Type %d", __func__, assetType );
        vx_assert( false );
        return;
    }

    bool needFileXfer = requireFileXfer( assetType );
    VxGUID& assetUniqueId = poPkt->getUniqueId();
    VxGUID& sendToId = poPkt->getSendToId();
    if(LogEnabled(eLogAssets)) LogModule( eLogAssets, LOG_VERBOSE,
        "AssetBaseXferMgr::%s rx send-req asset %s src %s sendTo %s req lcl %s req rmt %s needFile %s",
        __func__,
        assetUniqueId.toHexString().c_str(),
        poPkt->getSrcOnlineId().toOnlineIdString().c_str(),
        sendToId.toOnlineIdString().c_str(),
        poPkt->getLclSessionId().toHexString().c_str(),
        poPkt->getRmtSessionId().toHexString().c_str(),
        needFileXfer ? "true" : "false" );

    PktBaseSendReply* pktReply = createPktBaseSendReply();
    pktReply->setIsStream( poPkt->getIsStream() );
    pktReply->setRequiresFileXfer( needFileXfer );
    pktReply->setError( 0 );
    pktReply->setRmtSessionId( poPkt->getLclSessionId() );
    pktReply->setLclSessionId( poPkt->getRmtSessionId() );
    pktReply->setUniqueId( assetUniqueId );
    pktReply->setSendToId( sendToId );
    if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s lcl id %s rmt id %s", __func__, DescribePluginType(getPluginType()),
            pktReply->getLclSessionId().toHexString().c_str(), pktReply->getRmtSessionId().toHexString().c_str());
    if( eAssetTypeThumbnail != assetType )
    {
        bool isAllowed = netIdent->isHisAccessAllowedFromMe( getPluginType() );
        if( !isAllowed )
        {
            LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReq: permission denied" );
            pktReply->setError( eXferErrorPermission );
            m_XferInterface.txPacket( poPkt->getSrcOnlineId(), sktBase, pktReply);
            delete pktReply;
            return;
        }
    }

    if( false == needFileXfer )
    {
        // all we need is in the send request
        AssetBaseInfo assetInfo;
        poPkt->fillAssetFromPkt( assetInfo );
        // make history id his id
        assetInfo.setAssetSendState( eAssetSendStateRxSuccess );
        AssetBaseInfo* createdAsset = nullptr;
        if( !m_AssetBaseMgr.addAsset( assetInfo, createdAsset ) )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendReq: failed add asset" );
        }

        rebroadcastReceivedChatRoomAsset( assetInfo, poPkt->getSrcOnlineId(), true );

        m_XferInterface.txPacket( poPkt->getSrcOnlineId(), sktBase, pktReply );
        sendToGuiAssetAction( eAssetActionRxSuccess, assetInfo.getAssetUniqueId(), 100 );
        sendToGuiAssetAction( eAssetActionRxNotifyNewMsg, assetInfo.getCreatorId(), 100 );
    }
    else
    {
        AssetBaseRxSession* xferSession = findOrCreateRxSession( true, poPkt->getRmtSessionId(), poPkt->getSrcOnlineId(), sktBase);
        if( xferSession )
        {
            AssetBaseInfo& assetInfo = xferSession->getAssetBaseInfo();
            poPkt->fillAssetFromPkt( assetInfo );
            // make history id his id
            assetInfo.setHistoryId( poPkt->getSrcOnlineId() );
            assetInfo.setAssetSendState( eAssetSendStateRxProgress );

            xferSession->setRmtSessionId( poPkt->getLclSessionId() );
            pktReply->setLclSessionId( xferSession->getLclSessionId() );
            if(LogEnabled(eLogAssets)) LogModule( eLogAssets, LOG_VERBOSE,
                "AssetBaseXferMgr::%s rx session created asset %s rx lcl %s rx rmt %s reply lcl %s reply rmt %s",
                __func__,
                assetInfo.getAssetUniqueId().toHexString().c_str(),
                xferSession->getLclSessionId().toHexString().c_str(),
                xferSession->getRmtSessionId().toHexString().c_str(),
                pktReply->getLclSessionId().toHexString().c_str(),
                pktReply->getRmtSessionId().toHexString().c_str() );
            EXferError xferErr = beginAssetBaseReceive( xferSession, assetInfo, poPkt, *pktReply );
            if( eXferErrorNone != xferErr )
            {
                //sendToGuiUpdateAssetDownload( xferSession->getLclSessionId(), 0, rc );
                endAssetBaseXferSession( xferSession, true );
            }
        }
        else
        {
            LogMsg(LOG_ERROR, "PluginAssetBaseOffer::onPktAssetSendReq: Could not create session");
            pktReply->setError( eXferErrorBadParam );
            m_XferInterface.txPacket( poPkt->getSrcOnlineId(), sktBase, pktReply );
        }
    }

    delete pktReply;

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReq AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::assetSendComplete( AssetBaseTxSession* xferSession )
{
    updateAssetMgrSendState( xferSession->getSendToId(), xferSession->getAssetBaseInfo().getAssetUniqueId(), eAssetSendStateTxSuccess, 100 );
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReply AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReply AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK

    PktBaseSendReply * poPkt = (PktBaseSendReply *)pktHdr;
    if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s lcl id %s rmt id %s", __func__, DescribePluginType(getPluginType()),
            poPkt->getRmtSessionId().toHexString().c_str(), poPkt->getLclSessionId().toHexString().c_str());

    if( getPluginType() == ePluginTypeHostChatRoom || getPluginType() == ePluginTypeClientChatRoom )
    {
        LogModule( eLogChatRoom, LOG_INFO,
                   "AssetBaseXferMgr::%s recv send-reply pktPlugin %s localPlugin %s overridePlugin %s src %s sendTo %s lclSess %s rmtSess %s err %u fileXfer %u",
                   __func__,
                   DescribePluginType( (EPluginType)pktHdr->getPluginNum() ),
                   DescribePluginType( getPluginType() ),
                   DescribePluginType( m_XferInterface.getAssetOverridePluginType() ),
                   pktHdr->getSrcOnlineId().toOnlineIdString().c_str(),
                   poPkt->getSendToId().toOnlineIdString().c_str(),
                   poPkt->getLclSessionId().toHexString().c_str(),
                   poPkt->getRmtSessionId().toHexString().c_str(),
                   poPkt->getError(),
                   poPkt->getRequiresFileXfer() );
    }

    VxGUID&	assetUniqueId =	poPkt->getUniqueId();
    VxGUID&	sendToId =	poPkt->getSendToId();
    if( sendToId != srcOnlineId )
    {
        LogMsg( LOG_WARN, "AssetBaseXferMgr::%s src id %s does not match sendToId %s", __func__,
                srcOnlineId.toOnlineIdString().c_str(), sendToId.toOnlineIdString().c_str() );
    }

    AssetBaseInfo* assetInfo = m_AssetBaseMgr.findAsset( assetUniqueId );
    if( 0 == assetInfo )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendReply failed to find asset id");
        updateAssetMgrSendState( sendToId, assetUniqueId, eAssetSendStateTxFail, 0 );
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReply AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
        return;
    }

    bool isFileXfer = (bool)poPkt->getRequiresFileXfer();
    uint32_t rxedErrCode = poPkt->getError();
    AssetBaseTxSession* xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );
    if( !xferSession && poPkt->getLclSessionId().isValid() )
    {
        xferSession = findTxSessionSessionId( true, poPkt->getLclSessionId() );
    }
    if( !xferSession )
    {
        // Some peers can echo a different session-id pairing; recover by matching
        // the in-flight asset and peer instead of failing the transfer immediately.
        for( auto txSession : m_TxSessions )
        {
            if( !txSession )
            {
                continue;
            }

            if( txSession->getAssetBaseInfo().getAssetUniqueId() != assetUniqueId )
            {
                continue;
            }

            VxGUID txSendToId = txSession->getSendToId();
            if( txSendToId == sendToId || txSendToId == srcOnlineId )
            {
                xferSession = txSession;
                LogMsg( LOG_WARN,
                    "AssetBaseXferMgr::%s recovered tx session by asset/sendTo (pkt rmt %s lcl %s src %s sendTo %s tx lcl %s tx rmt %s tx sendTo %s)",
                    __func__,
                    poPkt->getRmtSessionId().toHexString().c_str(),
                    poPkt->getLclSessionId().toHexString().c_str(),
                    srcOnlineId.toOnlineIdString().c_str(),
                    sendToId.toOnlineIdString().c_str(),
                    txSession->getLclSessionId().toHexString().c_str(),
                    txSession->getRmtSessionId().toHexString().c_str(),
                    txSendToId.toOnlineIdString().c_str() );
                break;
            }
        }
    }

    if( xferSession )
    {
        xferSession->setIsStream( poPkt->getIsStream() );
        xferSession->setRmtSessionId( poPkt->getLclSessionId() );
        if( 0 == rxedErrCode )
        {
            if( isFileXfer )
            {
                // we did txNextAssetBaseChunk in begin file send
                //int32_t rc = txNextAssetBaseChunk( xferSession );
                //if( rc )
                //{
                //	//sendToGuiUpdateAssetUpload( xferSession->getLclSessionId(), 0, rc );
                //	LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendReply beginAssetBaseSend returned error %d\n", rc );
                //	endAssetBaseXferSession( xferSession, true );
                //}
            }
            else
            {
                assetSendComplete( xferSession );
                endAssetBaseXferSession( xferSession, true, false );
            }
        }
        else
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendReply PktAssetSendReply returned error %d", poPkt->getError() );
            endAssetBaseXferSession( xferSession, true, false );
            updateAssetMgrSendState( sendToId, assetUniqueId, eAssetSendStateTxFail, rxedErrCode );
        }
    }
    else
    {
        if( isFileXfer )
        {
            LogMsg( LOG_WARN, "AssetBaseXferMgr::onPktAssetSendReply unmatched file send-reply ignored asset %s rmt %s lcl %s src %s sendTo %s",
                assetUniqueId.toHexString().c_str(),
                poPkt->getRmtSessionId().toHexString().c_str(),
                poPkt->getLclSessionId().toHexString().c_str(),
                srcOnlineId.toOnlineIdString().c_str(),
                sendToId.toOnlineIdString().c_str() );
        }
        else
        {
            updateAssetMgrSendState( sendToId, assetUniqueId, rxedErrCode ? eAssetSendStateTxFail : eAssetSendStateTxSuccess, rxedErrCode );
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetSendReply AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    AssetBaseRxSession* xferSession = 0;
    if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s", __func__,
                  DescribePluginType( getPluginType() ) );
    PktBaseChunkReq* poPkt = (PktBaseChunkReq *)pktHdr;
    VxMutex& xferMutex = m_XferInterface.getAssetXferMutex();
    xferMutex.lock();
    if( poPkt->getRmtSessionId().isValid() )
    {
        xferSession = findRxSessionSessionId( true, poPkt->getRmtSessionId() );
        if( !xferSession )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s %s could not find rx session %s",
                   __func__, DescribePluginType( getPluginType() ), poPkt->getRmtSessionId().toHexString().c_str() );
            vx_assert(false);
        }
    }

    if( xferSession )
    {
        xferSession->setIsStream( poPkt->getIsStream() );

        xferMutex.unlock();
        EXferError xferErr = rxAssetBaseChunk( false, xferSession, poPkt );
        xferMutex.lock();

        if( eXferErrorNone != xferErr )
        {
            PktBaseChunkReply* pktReply = createPktBaseChunkReply();
            pktReply->setIsStream( poPkt->getIsStream() );
            pktReply->setLclSessionId( xferSession->getLclSessionId() );
            pktReply->setRmtSessionId( poPkt->getLclSessionId() );
            pktReply->setDataLen(0);
            pktReply->setError( xferErr );

            xferMutex.unlock();
            m_XferInterface.txPacket( poPkt->getSrcOnlineId(), sktBase, pktReply );
            xferMutex.lock();

            delete pktReply;

            sendToGuiAssetAction( eAssetActionRxError, xferSession->getAssetBaseInfo().getAssetUniqueId(), xferErr );

            endAssetBaseXferSession( xferSession, true );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetChunkReq failed to find session src %s lcl %s rmt %s",
            poPkt->getSrcOnlineId().toOnlineIdString().c_str(),
            poPkt->getLclSessionId().toHexString().c_str(),
            poPkt->getRmtSessionId().toHexString().c_str() );
        PktBaseChunkReply* pktReply = createPktBaseChunkReply();
        pktReply->setIsStream( poPkt->getIsStream() );
        pktReply->setLclSessionId( poPkt->getRmtSessionId() );
        pktReply->setRmtSessionId( poPkt->getLclSessionId() );
        pktReply->setDataLen(0);
        pktReply->setError( eXferErrorBadParam );

        xferMutex.unlock();
        m_XferInterface.txPacket( poPkt->getSrcOnlineId(), sktBase, pktReply );
        xferMutex.lock();

        delete pktReply;
    }

    xferMutex.unlock();
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktBaseChunkReply * poPkt = (PktBaseChunkReply *)pktHdr;
    AssetBaseTxSession* xferSession = 0;
static int cnt = 0;
    cnt++;
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetBaseChuckReply start %d", cnt );

    VxMutex& xferMutex = m_XferInterface.getAssetXferMutex();
    xferMutex.lock();

    if( poPkt->getRmtSessionId().isValid() )
    {
        xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );
    }

    if( xferSession )
    {
        xferSession->setIsStream( poPkt->getIsStream() );
        xferMutex.unlock();
        EXferError xferErr = txNextAssetBaseChunk( xferSession, poPkt->getError(), false );
        xferMutex.lock();

        if( eXferErrorNone != xferErr && eXferErrorCanceled != xferErr )
        {
            endAssetBaseXferSession( xferSession, true, false );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetBaseChuckReply failed to find session" );
    }

    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetBaseChuckReply done %d", cnt );

    xferMutex.unlock();
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_INFO, "AssetBaseXferMgr::%s",  __func__ );
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    pluginMutex.lock();

    PktBaseSendCompleteReq* poPkt = (PktBaseSendCompleteReq *)pktHdr;
    AssetBaseRxSession* xferSession = findRxSessionSessionId( true, poPkt->getRmtSessionId() );
    pluginMutex.unlock();

    //TODO check checksum
    if( xferSession )
    {
        xferSession->setIsStream( poPkt->getIsStream() );
        finishAssetBaseReceive( xferSession, poPkt, false );
    }
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    pluginMutex.lock();

    PktBaseSendCompleteReply * poPkt = (PktBaseSendCompleteReply *)pktHdr;
    AssetBaseTxSession* xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );
    if( xferSession )
    {
        xferSession->setIsStream( poPkt->getIsStream() );
        VxFileXferInfo xferInfo = xferSession->getXferInfo();
        AssetBaseInfo& assetInfo = xferSession->getAssetBaseInfo();

        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s Done Sending file %s", __func__, xferInfo.getLclFileNameAndPath().c_str() );

        onAssetBaseSent( poPkt->getSrcOnlineId(), sktBase, assetInfo, (EXferError)poPkt->getError(), true);
        endAssetBaseXferSession( xferSession, true, false );
    }
    else
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::onPktAssetSendCompleteReply failed to find session");
        updateAssetMgrSendState( srcOnlineId, poPkt->getAssetUniqueId(), eAssetSendStateTxSuccess, 100 );
    }

    pluginMutex.unlock();
}

//============================================================================
void AssetBaseXferMgr::onPktAssetBaseXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    LogMsg( LOG_INFO, "AssetBaseXferMgr::onPktAssetBaseXferErr");
    // TODO handle error
}

//============================================================================
void AssetBaseXferMgr::endAssetBaseXferSession( AssetBaseRxSession* poSessionIn, bool pluginIsLocked )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::endAssetBaseXferSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::endAssetBaseXferSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
    if( xferInfo.m_hFile )
    {
        VFileClose( xferInfo.m_hFile );
        xferInfo.m_hFile = nullptr;
    }

    std::string fileName = xferInfo.getDownloadIncompleteFileName();
    if( fileName.length() )
    {
        VxFileUtil::deleteFile( fileName.c_str() );
    }

    auto rxIter = m_RxSessions.begin();
    while( rxIter != m_RxSessions.end() )
    {
        AssetBaseRxSession* xferSession = rxIter->second;
        if( poSessionIn == xferSession )
        {
            if(LogEnabled(eLogFileXfer))LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s erasing lcl session id %s",
                          __func__, DescribePluginType( getPluginType() ), xferSession->getLclSessionId().toHexString().c_str() );
            m_RxSessions.erase( rxIter );
            delete xferSession;
            break;
        }
        else
        {
            ++rxIter;
        }
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::endAssetBaseXferSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }
}

//============================================================================
void AssetBaseXferMgr::endAssetBaseXferSession( AssetBaseTxSession* poSessionIn, bool pluginIsLocked, bool requeAsset )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
    }


    VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
    if( xferInfo.m_hFile )
    {
        VFileClose( xferInfo.m_hFile );
        xferInfo.m_hFile = NULL;
    }

    auto iter = m_TxSessions.begin();
    while( iter != m_TxSessions.end() )
    {
        AssetBaseTxSession* xferSession = (*iter);
        if( xferSession == poSessionIn )
        {
            m_TxSessions.erase( iter );
            if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s delete txSession lcl id %s rmt id %s",
                                                        __func__, DescribePluginType( getPluginType() ),
                                                        xferSession->getLclSessionId().toHexString().c_str(), xferSession->getRmtSessionId().toHexString().c_str() );
            delete xferSession;
            break;
        }
        else
        {
            ++iter;
        }
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__);
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }
}

//============================================================================
AssetBaseRxSession* AssetBaseXferMgr::findRxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    for( auto iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
    {
        AssetBaseRxSession* xferSession = iter->second;
        if( xferSession->getSendToId() == sendToId )
        {
            if( false == pluginIsLocked )
            {
#ifdef DEBUG_AUTOPLUGIN_LOCK
                LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
                pluginMutex.unlock();
            }

            return  xferSession;
        }
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return NULL;
}

//============================================================================
AssetBaseRxSession* AssetBaseXferMgr::findRxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId, bool logIfNotFound )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    auto iter = m_RxSessions.find( lclSessionId );
    if( iter != m_RxSessions.end() )
    {
        AssetBaseRxSession* rxSession = iter->second;
        if( false == pluginIsLocked )
        {
#ifdef DEBUG_AUTOPLUGIN_LOCK
            LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
            pluginMutex.unlock();
        }

        return rxSession;
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    if( logIfNotFound )
    {
        if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s could not find id %s",
                      __func__, DescribePluginType(getPluginType()), lclSessionId.toHexString().c_str() );
    }

    return nullptr;
}

//============================================================================
AssetBaseRxSession*	AssetBaseXferMgr::findOrCreateRxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    AssetBaseRxSession* xferSession = findRxSessionSendToId( true, sendToId );
    if( !xferSession )
    {
        xferSession = new AssetBaseRxSession( m_Engine, sktBase, sendToId );
        m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
    }
    else
    {
        xferSession->setSkt( sktBase );
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return xferSession;
}

//============================================================================
AssetBaseRxSession* AssetBaseXferMgr::findOrCreateRxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    if( false == lclSessionId.isValid() )
    {
        lclSessionId.initializeWithNewVxGUID();
    }

    AssetBaseRxSession* xferSession = findRxSessionSessionId( true, lclSessionId, false );
    if( nullptr == xferSession )
    {
        xferSession = new AssetBaseRxSession( m_Engine, lclSessionId, sktBase, sendToId );

        m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
        if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s %s insert lcl session id %s",
                      __func__, DescribePluginType(getPluginType()), lclSessionId.toHexString().c_str() );
    }
    else
    {
        xferSession->setSkt( sktBase );
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return xferSession;
}

//============================================================================
AssetBaseTxSession* AssetBaseXferMgr::findTxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* txSession = ( *iter );
        if( txSession->getSendToId() == sendToId )
        {
            if( false == pluginIsLocked )
            {
#ifdef DEBUG_AUTOPLUGIN_LOCK
                LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
                pluginMutex.unlock();
            }

            return txSession;
        }
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return NULL;
}

//============================================================================
AssetBaseTxSession* AssetBaseXferMgr::findTxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* txSession = ( *iter );
        if( txSession->getLclSessionId() == lclSessionId )
        {
            if( false == pluginIsLocked )
            {
#ifdef DEBUG_AUTOPLUGIN_LOCK
                LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
                pluginMutex.unlock();
            }

            return txSession;
        }
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s pluginMutex.unlock", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return nullptr;
}

//============================================================================
AssetBaseTxSession* AssetBaseXferMgr::createTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    AssetBaseTxSession* txSession = new AssetBaseTxSession( m_Engine, sktBase, sendToId );
    return txSession;
}

//============================================================================
AssetBaseTxSession* AssetBaseXferMgr::findOrCreateTxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    AssetBaseTxSession* xferSession = findTxSessionSendToId( true, sendToId );
    if( NULL == xferSession )
    {
        xferSession = createTxSession( sendToId, sktBase );
        if( false == xferSession->getLclSessionId().isValid() )
        {
            xferSession->getLclSessionId().initializeWithNewVxGUID();
        }

        addTxSession( xferSession );
    }
    else
    {
        xferSession->setSkt( sktBase );
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return xferSession;
}

//============================================================================
AssetBaseTxSession* AssetBaseXferMgr::findOrCreateTxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    AssetBaseTxSession* xferSession = 0;
    if( lclSessionId.isValid() )
    {
        xferSession = findTxSessionSessionId( true, lclSessionId );
    }
    else
    {
        xferSession = findTxSessionSendToId( true, sendToId );
    }

    if( nullptr == xferSession )
    {
        xferSession = new AssetBaseTxSession( m_Engine, lclSessionId, sktBase, sendToId );
        if( false == xferSession->getLclSessionId().isValid() )
        {
            xferSession->getLclSessionId().initializeWithNewVxGUID();
        }

        addTxSession( xferSession );
    }
    else
    {
        xferSession->setSkt( sktBase );
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::findOrCreateTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    return xferSession;
}

//============================================================================
bool AssetBaseXferMgr::fromGuiSendAssetBase( AssetBaseInfo& assetInfo )
{
    bool xferFailed = true;
    VxGUID sendToId = assetInfo.getSendToId();
    std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( sendToId, assetInfo.getPluginType() );
    if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
        "AssetBaseXferMgr::%s assetId %s assetType %s plugin %s sendTo %s creator %s admin %s skt %d ip %s connected %s",
        __func__,
        assetInfo.getAssetUniqueId().toHexString().c_str(),
        DescribeAssetType( assetInfo.getAssetType() ),
        DescribePluginType( assetInfo.getPluginType() ),
        m_Engine.describeUser( sendToId ).c_str(),
        m_Engine.describeUser( assetInfo.getCreatorId() ).c_str(),
        m_Engine.describeUser( assetInfo.getAdminId() ).c_str(),
        sktBase ? sktBase->getSktNumber() : -1,
        sktBase ? sktBase->getRemoteIpAddress() : "",
        ( sktBase && sktBase->isConnected() ) ? "true" : "false" );
    if( sktBase && sktBase->isConnected() )
    {
        EXferError xferError = createAssetTxSessionAndSend( false, assetInfo, sendToId, sktBase );
        if( xferError == eXferErrorNone )
        {
            xferFailed = false;
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s connection to %s not found", __func__, m_Engine.describeUser( sendToId ).c_str() );
        queAsset( assetInfo );
    }

    if( xferFailed )
    {
        onTxFailed( sendToId, assetInfo.getAssetUniqueId(), false );
    }

    return !xferFailed;
}

//============================================================================
bool AssetBaseXferMgr::fromGuiRequestAssetBase( AssetBaseInfo& assetInfo, std::shared_ptr<VxSktBase>& sktBase, bool tmpAsset )
{
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findBigListInfo( assetInfo.getOnlineId() );
    if( netIdent )
    {
        return fromGuiRequestAssetBase( netIdent, assetInfo, sktBase, tmpAsset );
    }

    return false;
}

//============================================================================
bool AssetBaseXferMgr::fromGuiRequestAssetBase( VxNetIdent* netIdent, AssetBaseInfo& assetInfo, std::shared_ptr<VxSktBase>& sktBaseIn, bool tmpAsset )
{
    if( !netIdent || !assetInfo.getAssetUniqueId().isValid() )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s invalid param", __func__ );
        vx_assert( false );
        return false;
    }

    if( sktBaseIn && isAssetRequested( assetInfo.getAssetUniqueId(), sktBaseIn->getSocketId() ) )
    {
        // already in transfer
        return true;
    }

    bool xferFailed = true;
    VxGUID sktConnectId;
    if( sktBaseIn && sktBaseIn->isConnected() )
    {
        sktConnectId = sktBaseIn->getSocketId();
        EXferError xferError = createAssetRxSessionAndReceive( false, assetInfo, netIdent->getMyOnlineId(), sktBaseIn, tmpAsset );
        if( xferError == eXferErrorNone )
        {
            xferFailed = false;
        }
    }
    else
    {
        // first try to connect and send.. if that fails then que and will send when next connected
        std::shared_ptr<VxSktBase> sktBase( nullptr );
        m_PluginMgr.pluginApiSktConnectTo( m_XferInterface.getPluginType(), netIdent, 0, sktBase );
        if( sktBase )
        {
            sktConnectId = sktBase->getSocketId();
            EXferError xferError = createAssetRxSessionAndReceive( false, assetInfo, netIdent->getMyOnlineId(), sktBase, tmpAsset );
            if( xferError == eXferErrorNone )
            {
                xferFailed = false;
            }
        }
        else
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::fromGuiRequestAssetBase Not connected to user" );
        }
    }

    if( xferFailed )
    {
        onRequestAssetFailed( netIdent->getMyOnlineId(), assetInfo, sktConnectId, false);
    }
    else
    {
        m_AssetRequestedListMutex.lock();
        m_AssetRequestedList.addGuidIfDoesntExist( assetInfo.getAssetUniqueId(), sktConnectId );
        m_AssetRequestedListMutex.unlock();
    }

    return !xferFailed;

}

//============================================================================
void AssetBaseXferMgr::onRequestAssetFailed( VxGUID sendToId, AssetBaseInfo& assetInfo, VxGUID& sktConnectId, bool pluginIsLocked )
{
    m_AssetRequestedListMutex.lock();
    m_AssetRequestedList.removeGuid( assetInfo.getAssetUniqueId(), sktConnectId );
    m_AssetRequestedListMutex.unlock();

    updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateRxFail, 0 );
}

//============================================================================
void AssetBaseXferMgr::onTxFailed( VxGUID& sendToId, VxGUID& assetUniqueId, bool pluginIsLocked )
{
    updateAssetMgrSendState( sendToId, assetUniqueId, eAssetSendStateTxFail, 0 );
}

//============================================================================
void AssetBaseXferMgr::onTxSuccess( VxGUID& sendToId, VxGUID& assetUniqueId, bool pluginIsLocked )
{
    updateAssetMgrSendState( sendToId, assetUniqueId, eAssetSendStateTxSuccess, 0 );
}

//============================================================================
void AssetBaseXferMgr::rebroadcastReceivedChatRoomAsset( AssetBaseInfo& assetInfo, const VxGUID& srcOnlineId, bool pluginIsLocked )
{
    if( getPluginType() != ePluginTypeHostChatRoom )
    {
        return;
    }

    HostedId hostedId( m_Engine.getMyOnlineId(), eHostTypeChatRoom );
    std::vector<VxGUID> onlineIdList;
    m_Engine.getConnectIdListMgr().getOnlineMembers( hostedId, onlineIdList );

    LogModule( eLogChatRoom, LOG_INFO, "AssetBaseXferMgr::%s chatroom rebroadcast asset %s src %s memberCount %u",
            __func__,
            assetInfo.getAssetUniqueId().toHexString().c_str(),
            srcOnlineId.toOnlineIdString().c_str(),
            (unsigned)onlineIdList.size() );

    for( auto& memberOnlineId : onlineIdList )
    {
        if( memberOnlineId == srcOnlineId || memberOnlineId == m_Engine.getMyOnlineId() )
        {
            LogModule( eLogChatRoom, LOG_INFO, "AssetBaseXferMgr::%s skip member %s (src/self)",
                    __func__, memberOnlineId.toOnlineIdString().c_str() );
            continue;
        }

        std::shared_ptr<VxSktBase> memberSkt = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( memberOnlineId, getPluginType() );
        if( !memberSkt || !memberSkt->isConnected() )
        {
            LogModule( eLogChatRoom, LOG_INFO, "AssetBaseXferMgr::%s skip member %s (no connected socket)",
                    __func__, memberOnlineId.toOnlineIdString().c_str() );
            continue;
        }

        AssetBaseInfo relayAsset( assetInfo );
        relayAsset.setDestUserId( memberOnlineId );
        relayAsset.setHistoryId( srcOnlineId );
        LogModule( eLogChatRoom, LOG_INFO, "AssetBaseXferMgr::%s rebroadcast asset %s to member %s",
                __func__,
                relayAsset.getAssetUniqueId().toHexString().c_str(),
                memberOnlineId.toOnlineIdString().c_str() );
        createAssetTxSessionAndSend( pluginIsLocked, relayAsset, memberOnlineId, memberSkt );
    }
}

//============================================================================
void AssetBaseXferMgr::addAssetXferInfoIfDoesNotExist( AssetBaseInfo& assetInfo )
{
    m_AssetBaseMgr.updateAsset( assetInfo );
}

//============================================================================
void AssetBaseXferMgr::updateAssetMgrSendState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState sendState, int param )
{
    m_AssetBaseMgr.updateAssetXferState( sendToId, assetUniqueId, sendState, param );
    announceXferState( sendToId, assetUniqueId, sendState, param );
}

//============================================================================
void AssetBaseXferMgr::queAsset( AssetBaseInfo& assetInfo )
{
    m_AssetBaseSendQueMutex.lock();
    bool foundAssetBase = false;

    for( auto iter = m_AssetBaseSendQue.begin(); iter != m_AssetBaseSendQue.end(); ++iter )
    {
        if( (*iter).getAssetUniqueId() == assetInfo.getAssetUniqueId() )
        {
            foundAssetBase = true;
            break;
        }
    }

    if( false == foundAssetBase )
    {
        m_AssetBaseSendQue.emplace_back( assetInfo );
    }

    m_AssetBaseSendQueMutex.unlock();
}

//============================================================================
EXferError AssetBaseXferMgr::createAssetTxSessionAndSend( bool pluginIsLocked, AssetBaseInfo& assetInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
        "AssetBaseXferMgr::%s assetId %s plugin %s sendTo %s skt %d ip %s pluginLocked %s",
        __func__,
        assetInfo.getAssetUniqueId().toHexString().c_str(),
        DescribePluginType( assetInfo.getPluginType() ),
        m_Engine.describeUser( sendToId ).c_str(),
        sktBase ? sktBase->getSktNumber() : -1,
        sktBase ? sktBase->getRemoteIpAddress() : "",
        pluginIsLocked ? "true" : "false" );

    VxMutex& pluginMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::createAssetTxSessionAndSend pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::createAssetTxSessionAndSend pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    }

    EXferError xferErr = eXferErrorNone;
    AssetBaseTxSession* txSession = createTxSession( sendToId, sktBase );
    if( false == txSession->getLclSessionId().isValid() )
    {
        txSession->getLclSessionId().initializeWithNewVxGUID();
    }

    if( false == txSession->getRmtSessionId().isValid() )
    {
        txSession->setRmtSessionId( txSession->getLclSessionId() );
    }

    txSession->setAssetBaseInfo( assetInfo );
    VxFileXferInfo& xferInfo = txSession->getXferInfo();
    xferInfo.setLclSessionId( txSession->getLclSessionId() );
    xferInfo.setRmtSessionId( txSession->getRmtSessionId() );
    xferInfo.setXferDirection( eXferDirectionTx );

    m_TxSessionsMutex.lock();
    addTxSession( txSession );
    m_TxSessionsMutex.unlock();

    addAssetXferInfoIfDoesNotExist( assetInfo );
    updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxProgress, 0 );

    bool isSinglePktSend{ false };
    if( assetInfo.hasFileName() )
    {
        // need to do first so file handle is set before get asset send reply back
        xferErr = beginAssetBaseSend( txSession );
    }
    else
    {
        // all data was in the request packet .. send and just wait for reply
        isSinglePktSend = true;
    }

    if( eXferErrorNone != xferErr )
    {
        // failed to open file
        updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxFail, xferErr );
        endAssetBaseXferSession( txSession, true, false );
        if( false == pluginIsLocked )
        {
#ifdef DEBUG_AUTOPLUGIN_LOCK
            LogMsg( LOG_INFO, "AssetBaseXferMgr::createAssetTxSessionAndSend pluginMutex.unlock\n");
#endif // DEBUG_AUTOPLUGIN_LOCK
            pluginMutex.unlock();
        }

        return xferErr;
    }

    PktBaseSendReq* sendReq = createPktBaseSendReq();
    sendReq->fillPktFromAsset( assetInfo );
    sendReq->setLclSessionId( xferInfo.getLclSessionId() );
    sendReq->setRmtSessionId( xferInfo.getRmtSessionId() );
    if( false == m_PluginMgr.pluginApiTxPacket( m_XferInterface.getPluginType(), sendToId, sktBase, sendReq, m_XferInterface.getAssetOverridePluginType() ) )
    {
        xferErr = eXferErrorDisconnected;
    }

    if( eXferErrorNone == xferErr )
    {
        if( requireFileXfer( assetInfo.getAssetType() ) )
        {
            xferErr = txNextAssetBaseChunk( txSession, eXferErrorNone, true );
        }
        else if( isSinglePktSend )
        {
            // tell gui asset was sent
            updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxProgress, 100 );
//			updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxSuccess, xferErr );
            endAssetBaseXferSession( txSession, true, false );
        }
    }
    else
    {
        // re que for try some other time
        updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxFail, xferErr );
        endAssetBaseXferSession( txSession, true, ((eXferErrorFileNotFound == xferErr) || (eXferErrorDisconnected == xferErr)) ? false : true );
    }

    if( false == pluginIsLocked )
    {
#ifdef DEBUG_AUTOPLUGIN_LOCK
        LogMsg( LOG_INFO, "AssetBaseXferMgr::createAssetTxSessionAndSend pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
        pluginMutex.unlock();
    }

    delete sendReq;
    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::createAssetRxSessionAndReceive( bool pluginIsLocked, AssetBaseInfo& assetInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, bool tmpAsset )
{
    EXferError xferErr = eXferErrorNone;
    PktBaseGetReq* pktReq = createPktBaseGetReq();
    if( pktReq )
    {
        pktReq->setAssetType( assetInfo.getAssetType() );
        pktReq->setUniqueId( assetInfo.getAssetUniqueId() );
        pktReq->setSendToId( assetInfo.getDestUserId() );
        pktReq->getLclSessionId().initializeWithNewVxGUID();

        pktReq->setIsTemporary( tmpAsset );

        if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "***AssetBaseXferMgr::%s %s lcl session id %s requesting %s", __func__, DescribePluginType( getPluginType() ),
               pktReq->getLclSessionId().toHexString().c_str(), assetInfo.getAssetName().c_str() );

        if( !m_XferInterface.txPacket( sendToId, sktBase, pktReq, m_XferInterface.getAssetOverridePluginType() ) )
        {
            xferErr = eXferErrorDisconnected;
        }
    }
    else
    {
        xferErr = eXferErrorBadParam;
    }

    delete pktReq;
    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::beginAssetBaseSend( AssetBaseTxSession* xferSession )
{
    EXferError xferErr = eXferErrorNone;
    xferSession->clearErrorCode();
    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    if( xferInfo.m_hFile )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::beginAssetBaseSend: ERROR: AssetBase transfer still in progress" );
        xferErr = eXferErrorAlreadyUploading;
    }

    if( eXferErrorNone == xferErr )
    {
        xferInfo.setXferDirection( eXferDirectionTx );
        xferInfo.setLclFileName( xferSession->getAssetBaseInfo().getAssetName().c_str() );
        xferInfo.setLclFileNameAndPath( xferSession->getAssetBaseInfo().getAssetNameAndPath().c_str() );
        xferInfo.setRmtFileName( xferSession->getAssetBaseInfo().getAssetName().c_str() );

        xferInfo.setLclSessionId( xferSession->getLclSessionId() );
        xferInfo.setRmtSessionId( xferSession->getRmtSessionId() );
        xferInfo.setFileHashId( xferSession->getFileHashId() );

        xferInfo.m_u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str() );
        if( 0 == xferInfo.m_u64FileLen )
        {
            // no file found to send
            LogMsg( LOG_INFO, "AssetBaseXferMgr::beginAssetBaseSend: AssetBase %s not found to send", xferInfo.getLclFileNameAndPath().c_str() );
            xferErr = eXferErrorFileNotFound;
        }
        else if( false == xferInfo.getFileHashId().isHashValid() )
        {
            // see if we can get hash from shared files
            //if( !m_SharedAssetBasesMgr.getAssetHashId( xferInfo.getLclFileNameAndPath(), xferInfo.getFileHashId() ) )
            //{
            //	// TODO.. que for hash
            //}
        }
    }

    if( eXferErrorNone == xferErr )
    {
        xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "rb" );
        if( NULL == xferInfo.m_hFile )
        {
            // open file failed
            xferInfo.m_hFile = NULL;
            LogMsg( LOG_INFO, "AssetBaseXferMgr::beginAssetBaseSend: Could not open AssetBase %s", xferInfo.getLclFileNameAndPath().c_str() );
            xferErr = eXferErrorFileOpenError;
            xferSession->setErrorCode( VxGetLastError() );
        }
    }

    if( eXferErrorNone == xferErr )
    {
        if( 0 != xferInfo.m_u64FileOffs )
        {
            if( xferInfo.m_u64FileLen < xferInfo.m_u64FileOffs )
            {
                VFileClose( xferInfo.m_hFile );
                xferInfo.m_hFile = NULL;
                LogMsg( LOG_INFO, "AssetBaseXferMgr::beginAssetBaseSend: AssetBase %s could not be resumed because too short",
                    xferInfo.getLclFileNameAndPath().c_str() );
                xferErr  = eXferErrorFileSeekError;
            }

            if( eXferErrorNone == xferErr )
            {
                int32_t rc = -1;
                // we have valid file so seek to end so we can resume if partial file exists
                if( 0 != (rc = VFileSeek64( xferInfo.m_hFile, xferInfo.m_u64FileOffs )) )
                {
                    // seek failed
                    VFileClose( xferInfo.m_hFile );
                    xferInfo.m_hFile = NULL;
                    LogMsg( LOG_INFO, "AssetBaseXferMgr::beginAssetBaseSend: could not seek to position %d in file %s",
                        xferInfo.m_u64FileOffs,
                        xferInfo.getLclFileNameAndPath().c_str() );
                    xferErr  = eXferErrorFileSeekError;
                    xferSession->setErrorCode( rc );
                }
            }
        }
    }

    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::beginAssetBaseReceive( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, PktBaseSendReq* poPkt, PktBaseSendReply& pktReply )
{
    if( nullptr == xferSession )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::beginAssetBaseReceive: NULL xferSession" );
        return eXferErrorBadParam;
    }

    EXferError xferErr = eXferErrorNone;
    if( poPkt->getError() )
    {
        xferErr = (EXferError)poPkt->getError();
        return xferErr;
    }

    VxFileXferInfo& xferInfo = xferSession->getXferInfo();

    xferErr = beginAssetBaseReceive( xferSession, assetInfo, poPkt->getLclSessionId(), poPkt->getAssetOffset() );
    if( eXferErrorNone == xferErr )
    {
        LogMsg( LOG_INFO, "AssetBaseXferMgr::(AssetBase Send) start recieving file %s",
            xferInfo.getLclFileNameAndPath().c_str() );
        poPkt->fillAssetFromPkt( xferSession->getAssetBaseInfo() );
    }

    pktReply.setError( xferErr );
    pktReply.setAssetOffset( xferInfo.m_u64FileOffs );
    if( false == m_XferInterface.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply ) )
    {
        xferErr = eXferErrorDisconnected;
    }

    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::beginAssetBaseReceive( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, VxGUID& rmtSessionId, int64_t startOffset )
{
    EXferError xferErr = eXferErrorNone;
    uint64_t u64FileLen;
    VxFileXferInfo& xferInfo = xferSession->getXferInfo();

    if( eXferErrorNone == xferErr )
    {
        if( xferInfo.m_hFile )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s: ERROR:(AssetBase Receive) receive transfer still in progress", __func__ );
            xferErr = eXferErrorAlreadyDownloading;
        }
    }

    if( eXferErrorNone == xferErr )
    {
        // get file information
        xferInfo.setFileHashId( assetInfo.getAssetHashId() );
        xferInfo.setRmtSessionId( rmtSessionId );
        if( false == xferInfo.getLclSessionId().isValid() )
        {
            xferInfo.getLclSessionId().initializeWithNewVxGUID();
        }

        xferInfo.setRmtFileName( assetInfo.getAssetName().c_str() );
        if( 0 == xferInfo.getRmtFileName().length() )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::beginAssetBaseReceive: ERROR: No file Name" );
            xferErr = eXferErrorBadParam;
        }
    }

    std::string strRmtPath;
    std::string strRmtAssetBaseNameOnly;
    VxFileUtil::seperatePathAndFile(		xferInfo.getRmtFileName(),
                                            strRmtPath,
                                            strRmtAssetBaseNameOnly );
    if( eXferErrorNone == xferErr )
    {
        // make full path
        if( 0 == strRmtAssetBaseNameOnly.length() )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::beginAssetBaseReceive: ERROR: NULL file Name %s",  xferInfo.getRmtFileName().c_str() );
            xferErr = eXferErrorBadParam;
        }
    }

    if( eXferErrorNone == xferErr )
    {
        xferInfo.setLclFileName( strRmtAssetBaseNameOnly.c_str() );
        VxFileUtil::makeFullPath( strRmtAssetBaseNameOnly.c_str(), VxGetIncompleteDirectory().c_str(), xferInfo.getLclFileNameAndPath() );
        std::string strPath;
        std::string strAssetBaseNameOnly;
        int32_t rc = VxFileUtil::seperatePathAndFile(	xferInfo.getLclFileNameAndPath(),
                                                    strPath,
                                                    strAssetBaseNameOnly );
        VxFileUtil::makeDirectory( strPath );
        xferInfo.m_u64FileLen = assetInfo.getAssetLength();
        xferInfo.m_u64FileOffs = startOffset;
        u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str(), false );

        if( 0 != xferInfo.m_u64FileOffs )
        {
            if( u64FileLen < xferInfo.m_u64FileOffs )
            {
                xferErr  = eXferErrorFileSeekError;
                LogMsg( LOG_INFO, "AssetBaseXferMgr: ERROR:(AssetBase Send) %d AssetBase %s could not be resumed because too short",
                        rc, xferInfo.getLclFileNameAndPath().c_str() );
            }
            else
            {
                xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "a+" ); // pointer to name of the file
                if( NULL == xferInfo.m_hFile )
                {
                    // failed to open file
                    xferInfo.m_hFile = NULL;
                    rc = VxGetLastError();
                    xferSession->setErrorCode( rc );
                    xferErr  = eXferErrorFileOpenError;

                    LogMsg( LOG_INFO, "AssetBaseXferMgr: ERROR:(AssetBase Send) %d AssetBase %s could not be created",
                            rc, xferInfo.getLclFileNameAndPath().c_str() );
                }
                else
                {
                    // we have valid file so seek to end so we can resume if partial file exists
                    if( 0 != (rc = VFileSeek64( xferInfo.m_hFile, xferInfo.m_u64FileOffs )) )
                    {
                        // seek failed
                        xferSession->setErrorCode( rc );
                        xferErr  = eXferErrorFileSeekError;
                        VFileClose( xferInfo.m_hFile );
                        xferInfo.m_hFile = NULL;
                        LogMsg( LOG_INFO, "AssetBaseXferMgr::%s ERROR: could not seek to position %d in file %s", __func__,
                                xferInfo.m_u64FileOffs, xferInfo.getLclFileNameAndPath().c_str() );
                    }
                }
            }
        }
        else
        {
            if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::%s opening file for recieve %s", __func__,
                    xferInfo.getLclFileNameAndPath().c_str() );
            // open file and truncate if exists
            xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "wb+" ); // pointer to name of the file
            if( NULL == xferInfo.m_hFile )
            {
                // failed to open file
                xferInfo.m_hFile = NULL;
                rc = VxGetLastError();
                xferSession->setErrorCode( rc );
                xferErr = eXferErrorFileCreateError;

                LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s ERROR: %d AssetBase %s could not be created",
                        __func__, rc, xferInfo.getLclFileNameAndPath().c_str() );
            }
        }
    }

    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::txNextAssetBaseChunk( AssetBaseTxSession* xferSession, uint32_t remoteErr, bool pluginIsLocked )
{
    if( 0 == xferSession )
    {
        return eXferErrorBadParam;
    }

    EXferError xferErr = eXferErrorNone;
    VxMutex& assetMutex = m_XferInterface.getAssetXferMutex();
    if( false == pluginIsLocked )
    {
        assetMutex.lock();
    }

    // fill the packet with data from the file
    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    if( !xferInfo.m_hFile )
    {
        // has already been sent
        if( false == pluginIsLocked )
        {
            assetMutex.unlock();
        }

        return eXferErrorNone;
    }

    std::shared_ptr<VxSktBase>& sktBase =  xferSession->getSkt();
    AssetBaseInfo& assetInfo = xferSession->getAssetBaseInfo();
    VxGUID assetId = assetInfo.getAssetUniqueId();
    VxGUID lclSessionId = xferSession->getLclSessionId();

    if( 0 != remoteErr )
    {
        // canceled download by remote user
        LogMsg( LOG_INFO, "AssetBaseXferMgr:: Cancel Sending file %s", xferInfo.getLclFileNameAndPath().c_str() );
        onAssetBaseSent( xferSession->getSendToId(), sktBase, assetInfo, eXferErrorCanceled, pluginIsLocked);
        endAssetBaseXferSession( xferSession, true, false );
        if( false == pluginIsLocked )
        {
            assetMutex.unlock();
        }

        return eXferErrorCanceled;
    }

    vx_assert( xferInfo.m_hFile );
    vx_assert( xferInfo.m_u64FileLen );
    if( xferInfo.m_u64FileOffs >= xferInfo.m_u64FileLen && xferInfo.m_hFile )
    {
        //we are done sending file
        if( xferInfo.m_hFile )
        {
            VFileClose( xferInfo.m_hFile );
            xferInfo.m_hFile  = NULL;
        }

        PktBaseSendCompleteReq* completeReq = createPktBaseSendCompleteReq();
        completeReq->setIsStream( xferSession->getIsStream() );
        completeReq->setLclSessionId( xferSession->getLclSessionId() );
        completeReq->setRmtSessionId( xferSession->getRmtSessionId() );
        completeReq->setAssetUniqueId( xferSession->getAssetBaseInfo().getAssetUniqueId() );
        completeReq->setSendToId( xferSession->getAssetBaseInfo().getSendToId() );
        if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr:: Done Sending file %s", xferInfo.getLclFileNameAndPath().c_str() );
        AssetBaseInfo baseInfo = xferSession->getAssetBaseInfo();

        // make copies before the session is ended and deleted
        std::shared_ptr<VxSktBase> sktBase = xferSession->getSkt();
        VxGUID sendToId = xferSession->getSendToId();

        onAssetBaseSent( sendToId, sktBase, baseInfo, eXferErrorNone, true);
        endAssetBaseXferSession( xferSession, true, false );

        if( false == pluginIsLocked )
        {
            assetMutex.unlock();
        }

        m_XferInterface.txPacket( sendToId, sktBase, completeReq );


        return eXferErrorNone;
    }

    PktBaseChunkReq* pktChunkReq = createPktBaseChunkReq();
    pktChunkReq->setIsStream( xferSession->getIsStream() );
    // see how much we can read
    uint32_t u32ChunkLen = (uint32_t)(xferInfo.m_u64FileLen - xferInfo.m_u64FileOffs);
    if( PKT_TYPE_ASSET_MAX_DATA_LEN < u32ChunkLen )
    {
        u32ChunkLen = PKT_TYPE_ASSET_MAX_DATA_LEN;
    }

    if( 0 == u32ChunkLen )
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::txNextAssetBaseChunk 0 len u32ChunkLen" );
        // what to do?
        if( false == pluginIsLocked )
        {
            assetMutex.unlock();
        }

        return eXferErrorNone;
    }

    // read data into packet
    uint32_t u32BytesRead = (uint32_t)VFileRead(	pktChunkReq->m_au8AssetChunk,
                                                    1,
                                                    u32ChunkLen,
                                                    xferInfo.m_hFile );
    if( u32BytesRead != u32ChunkLen )
    {
        int32_t rc = VxGetLastError();
        xferSession->setErrorCode( rc );
        xferErr = eXferErrorFileReadError;

        VFileClose( xferInfo.m_hFile );
        xferInfo.m_hFile  = NULL;
        LogMsg( LOG_ERROR, "AssetBaseXferMgr: ERROR: %d reading send file at offset %" PRId64 " when file len %" PRId64 "  file name %s",
                    rc,
                    xferInfo.m_u64FileOffs,
                    xferInfo.m_u64FileLen,
                    xferInfo.getLclFileNameAndPath().c_str() );
    }
    else
    {
        xferInfo.m_u64FileOffs += u32ChunkLen;
        pktChunkReq->setChunkLen( (uint16_t)u32ChunkLen );
        pktChunkReq->setLclSessionId( xferInfo.getLclSessionId() );
        pktChunkReq->setRmtSessionId( xferInfo.getRmtSessionId() );
    }

    if( eXferErrorNone == xferErr )
    {
        if( false == pluginIsLocked )
        {
            assetMutex.unlock();
        }

        if( false == m_XferInterface.txPacket( xferSession->getSendToId(), xferSession->getSkt(), pktChunkReq ) )
        {
            xferErr = eXferErrorDisconnected;
        }

        if( false == pluginIsLocked )
        {
            assetMutex.lock();
        }
    }
    if( eXferErrorNone != xferErr )
    {
        sendToGuiAssetAction( eAssetActionTxError, assetId, xferErr );
    }
    else
    {
        // the session may have ended so get again
        AssetBaseTxSession* xferSession2 = findTxSessionSessionId( true, lclSessionId );
        if( xferSession2 )
        {
            VxFileXferInfo& xferInfo2 = xferSession2->getXferInfo();
            if( xferInfo2.calcProgress() )
            {
                sendToGuiAssetAction( eAssetActionTxProgress, xferSession2->getAssetBaseInfo().getAssetUniqueId(), xferInfo2.getProgress() );
            }
        }
    }

    if( false == pluginIsLocked )
    {
        assetMutex.unlock();
    }

    delete pktChunkReq;
    return xferErr;
}

//============================================================================
EXferError AssetBaseXferMgr::rxAssetBaseChunk( bool pluginIsLocked, AssetBaseRxSession* xferSession, PktBaseChunkReq* poPkt )
{
    if( NULL == xferSession )
    {
        return eXferErrorBadParam;
    }

    VxMutex& xferMutex = m_XferInterface.getAssetXferMutex();
    if( !pluginIsLocked )
    {
        xferMutex.lock();
    }

    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    VxGUID assetId = xferSession->getAssetBaseInfo().getAssetUniqueId();
    VxGUID lclSessionId = xferInfo.getLclSessionId();

    EXferError xferErr = (EXferError)poPkt->getError();
    if( eXferErrorNone != xferErr )
    {
        // canceled by sender
        if( !pluginIsLocked )
        {
            xferMutex.unlock();
        }

        return xferErr;
    }

    // we are receiving a file
    if( xferInfo.m_hFile )
    {
        //write the chunk of data out to the file
        uint32_t u32BytesWritten = (uint32_t)VFileWrite(	poPkt->m_au8AssetChunk,
                                                            1,
                                                            poPkt->getChunkLen(),
                                                            xferInfo.m_hFile );
        if( u32BytesWritten != poPkt->getChunkLen() )
        {
            int32_t rc = VxGetLastError();
            xferSession->setErrorCode( rc );
            xferErr = eXferErrorFileWriteError;

            LogMsg( LOG_INFO, "VxPktHandler::RxAssetBaseChunk: ERROR %d: writing to file %s",
                            rc,
                            xferInfo.getLclFileNameAndPath().c_str() );
        }
        else
        {
            // successfully write
            xferInfo.m_u64FileOffs += poPkt->getChunkLen();

            PktBaseChunkReply* pktChunkReply = createPktBaseChunkReply();
            pktChunkReply->setIsStream( poPkt->getIsStream() );
            pktChunkReply->setDataLen( poPkt->getDataLen() );
            pktChunkReply->setLclSessionId( xferInfo.getLclSessionId() );
            pktChunkReply->setRmtSessionId( xferInfo.getRmtSessionId() );
            std::shared_ptr<VxSktBase>& sktBase = xferSession->getSkt();

            if( !pluginIsLocked )
            {
                xferMutex.unlock();
            }

            if( false == m_XferInterface.txPacket( xferSession->getSendToId(), sktBase, pktChunkReply, m_XferInterface.getAssetOverridePluginType() ) )
            {
                xferErr = eXferErrorDisconnected;
            }

            if( !pluginIsLocked )
            {
                xferMutex.lock();
            }

            delete pktChunkReply;
        }
    }

    if( eXferErrorNone == xferErr )
    {
        // the session may have allready completed and been erased so get the session again
        AssetBaseRxSession* xferSession2 = findRxSessionSessionId( true, lclSessionId );
        if( xferSession2 )
        {
            VxFileXferInfo& xferInfo2 = xferSession2->getXferInfo();
            if( xferInfo2.calcProgress() )
            {
                sendToGuiAssetAction( eAssetActionRxProgress, xferSession->getAssetBaseInfo().getAssetUniqueId(), xferInfo2.getProgress() );
            }
        }
    }
    else
    {
        sendToGuiAssetAction( eAssetActionRxError, assetId, xferErr );
    }

    if( !pluginIsLocked )
    {
        xferMutex.unlock();
    }

    return xferErr;
}

//============================================================================
void AssetBaseXferMgr::finishAssetBaseReceive( AssetBaseRxSession* xferSession, PktBaseSendCompleteReq* poPkt, bool pluginIsLocked )
{
    // done receiving file
    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    if( xferInfo.m_hFile )
    {
        VFileClose( xferInfo.m_hFile );
        xferInfo.m_hFile = NULL;
    }
    else
    {
        LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s NULL file handle", __func__ );
    }

    //// let other act on the received file
    std::string strAssetBaseName = xferInfo.getLclFileName();

    PktBaseSendCompleteReply* pktCompleteReply = createPktBaseSendCompleteReply();
    pktCompleteReply->setIsStream( poPkt->getIsStream() );
    pktCompleteReply->setLclSessionId( xferInfo.getLclSessionId() );
    pktCompleteReply->setRmtSessionId( xferInfo.getRmtSessionId() );
    pktCompleteReply->setAssetUniqueId( xferSession->getAssetBaseInfo().getAssetUniqueId() );
    m_XferInterface.txPacket( xferSession->getSendToId(), xferSession->getSkt(), pktCompleteReply, m_XferInterface.getAssetOverridePluginType() );
    LogMsg( LOG_INFO, "VxPktHandler: Done Receiving file %s", strAssetBaseName.c_str() );

    xferSession->setErrorCode( poPkt->getError() );
    onAssetBaseReceived( xferSession, xferSession->getAssetBaseInfo(), (EXferError)poPkt->getError(), pluginIsLocked );
    delete pktCompleteReply;
}

//============================================================================
void AssetBaseXferMgr::onAssetBaseReceived( AssetBaseRxSession* xferSession, AssetBaseInfo& assetInfo, EXferError error, bool pluginIsLocked )
{
    //m_PluginMgr.getToGui().toGuiFileDownloadComplete( xferSession->getLclSessionId(), error );
    VxFileXferInfo& xferInfo = xferSession->getXferInfo();
    if( eXferErrorNone == error )
    {
        std::string incompleteAsset = xferInfo.getDownloadIncompleteFileName();
        std::string completedAssetBase = xferInfo.getDownloadCompleteFileName();
        if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "AssetBaseXferMgr::onAssetBaseReceived: moving completed file from: %s to: %s", incompleteAsset.c_str(), completedAssetBase.c_str() );

        int32_t rc = 0;
        if( 0 == ( rc = VxFileUtil::moveAFile( incompleteAsset.c_str(), completedAssetBase.c_str() ) ) )
        {
            assetInfo.setAssetNameAndPath( completedAssetBase.c_str() );
            //BRJ ?? assetInfo.setHistoryId( xferSession->getIdent()->getMyOnlineId() );

            if( eXferErrorNone == error )
            {
                assetInfo.setAssetSendState( eAssetSendStateRxSuccess );
            }
            else
            {
                assetInfo.setAssetSendState(  eAssetSendStateRxFail );
            }

            AssetBaseInfo* createdAsset = nullptr;
            if( !m_AssetBaseMgr.addAsset( assetInfo, createdAsset ) )
            {
                LogMsg( LOG_ERROR, "AssetBaseXferMgr::onAssetBaseReceived: failed add asset" );
            }

            if( createdAsset && createdAsset->isValidFile() )
            {
                if( eXferErrorNone == error )
                {
                    VxGUID srcOnlineId = xferSession->getSendToId();
                    rebroadcastReceivedChatRoomAsset( *createdAsset, srcOnlineId, pluginIsLocked );

                    if( eAssetTypeThumbnail != assetInfo.getAssetType() )
                    {
                        FileInfo fileInfo( *createdAsset, xferInfo.getFileHashId() );
                        m_Engine.fromGuiSetFileIsInLibrary( fileInfo, true );
                    }

                    sendToGuiAssetAction( eAssetActionRxSuccess, xferSession->getAssetBaseInfo().getAssetUniqueId(), 0 );
                }
                else
                {
                    sendToGuiAssetAction( eAssetActionRxError, xferSession->getAssetBaseInfo().getAssetUniqueId(), error );
                }
            }
        }
        else
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::onAssetBaseReceived ERROR %d moving %s to %s", rc, incompleteAsset.c_str(), completedAssetBase.c_str() );
        }
    }

    endAssetBaseXferSession( xferSession, pluginIsLocked );
}

//============================================================================
void AssetBaseXferMgr::onAssetBaseSent( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, AssetBaseInfo& assetInfo, EXferError error, bool pluginIsLocked )
{
    //m_PluginMgr.getToGui().toGuiAssetBaseUploadComplete( xferSession->getRmtSessionId(), error );
    if( eXferErrorNone != error )
    {
        updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxFail, (int)error );
        sendToGuiAssetAction( eAssetActionTxError, assetInfo.getAssetUniqueId(), error );
    }
    else
    {
        updateAssetMgrSendState( sendToId, assetInfo.getAssetUniqueId(), eAssetSendStateTxSuccess, (int)error );
        sendToGuiAssetAction( eAssetActionTxSuccess, assetInfo.getAssetUniqueId(), 0 );
    }

    if( sktBase && sktBase->isConnected() && false == VxIsAppShuttingDown() )
    {
        if( !checkQueForMoreAssetsToSend( pluginIsLocked, sendToId, sktBase ) )
        {
            announceXferReadyToSend( sendToId, sktBase );
        }
    }
}

//============================================================================
bool AssetBaseXferMgr::checkQueForMoreAssetsToSend( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    // check que and start next xfer

    bool queuedAssetBeingSent{ false };
    m_AssetBaseSendQueMutex.lock();
    for( auto iter = m_AssetBaseSendQue.begin(); iter != m_AssetBaseSendQue.end(); ++iter )
    {
        if( sendToId == (*iter).getSendToId() )
        {
            // found asset to send
            AssetBaseInfo& assetInfo = (*iter);
            int32_t rc = createAssetTxSessionAndSend( pluginIsLocked, assetInfo, assetInfo.getSendToId(), sktBase );
            if( 0 == rc )
            {
                queuedAssetBeingSent = true;
                iter = m_AssetBaseSendQue.erase(iter);
            }
            else
            {
                LogMsg( LOG_ERROR, "AssetBaseXferMgr::checkQueForMoreAssetsToSend error %d sendTo %s", rc, m_Engine.describeUser( assetInfo.getSendToId() ).c_str() );
            }

            break;
        }
    }

    m_AssetBaseSendQueMutex.unlock();
    return queuedAssetBeingSent;
}

//============================================================================
void AssetBaseXferMgr::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::replaceConnection AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
    AutoXferLock pluginMutexLock( m_XferInterface.getAssetXferMutex() );
#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::replaceConnection AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
    for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
    {
        AssetBaseTxSession* xferSession = (*iter);
        if( xferSession->getSkt() == poOldSkt )
        {
            xferSession->setSkt( poNewSkt );
        }
    }

    for( auto oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
    {
        AssetBaseRxSession* xferSession = oRxIter->second;
        if( xferSession->getSkt() == poOldSkt )
        {
            xferSession->setSkt( poNewSkt );
        }
    }

#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogMsg( LOG_INFO, "AssetBaseXferMgr::replaceConnection AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void AssetBaseXferMgr::onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
    checkQueForMoreAssetsToSend( false, netIdent->getMyOnlineId(), sktBase);
}

//============================================================================
bool AssetBaseXferMgr::isAssetRequested( VxGUID& assetId, VxGUID& sktConnectId )
{
    static uint64_t timeoutMs = ( 1000 * 5 * 60 ); // 5 minutes

    uint64_t timeNowMs = GetTimeStampMs();

    bool inQue{ false };

    m_AssetRequestedListMutex.lock();
    inQue = m_AssetRequestedList.doesGuidExist( assetId, sktConnectId, timeoutMs );
    m_AssetRequestedList.removeExpired( timeNowMs, timeoutMs );
    m_AssetRequestedListMutex.unlock();

    return inQue;
}

//============================================================================
void AssetBaseXferMgr::assetXferComplete( VxGUID& assetId, VxGUID& sktConnectId )
{
    m_AssetRequestedListMutex.lock();
    m_AssetRequestedList.removeGuid( assetId, sktConnectId );
    m_AssetRequestedListMutex.unlock();
}

//============================================================================
void AssetBaseXferMgr::sendToGuiAssetAction( EAssetAction assetAction, VxGUID& assetId, int pos0to100000 )
{
    IToGui::getIToGui().toGuiAssetAction( assetAction, assetId, pos0to100000 );
}

//============================================================================
void AssetBaseXferMgr::wantAssetXferCallbacks( AssetXferCallback* client, bool enable )
{
    lockClientList();

    bool found{ false };
    for( auto iter = m_AssetXferClients.begin(); iter != m_AssetXferClients.end(); ++iter )
    {
        if( *iter == client )
        {
            found = true;
            if( !enable )
            {
                m_AssetXferClients.erase( iter );
            }
            else
            {
                LogMsg( LOG_ERROR, "MemberActiveMgr::wantMemberActiveCallbacks ignored because already in list" );
            }

            break;
        }
    }

    if( !found && enable )
    {
        m_AssetXferClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void AssetBaseXferMgr::announceXferReadyToSend( VxGUID& sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
    lockClientList();
    for( auto& client : m_AssetXferClients )
    {
        client->callbackAssetXferReadyToSend( sendToId, sktBase );
    }

    unlockClientList();
}

//============================================================================
void AssetBaseXferMgr::announceXferState( VxGUID& sendToId, VxGUID& assetId, enum EAssetSendState sendState, int param )
{
    lockClientList();
    for( auto& client : m_AssetXferClients )
    {
        client->callbackXferState( sendToId, assetId, sendState, param );
    }

    unlockClientList();
}

//============================================================================
void AssetBaseXferMgr::addTxSession( AssetBaseTxSession* xferSession )
{
    for( auto txSession : m_TxSessions )
    {
        if( xferSession == txSession )
        {
            LogMsg( LOG_ERROR, "AssetBaseXferMgr::%s attempted to add same session again", __func__ );
            return;
        }
    }

    m_TxSessions.emplace_back( xferSession );
}
