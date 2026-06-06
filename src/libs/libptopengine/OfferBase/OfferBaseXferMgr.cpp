//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "OfferBaseXferMgr.h"
#include "OfferBaseInfo.h"
#include "OfferBaseMgr.h"

#include "../Plugins/PluginBase.h"
#include "../Plugins/PluginMgr.h"
#include "../Plugins/PluginMessenger.h"
#include "OfferBaseTxSession.h"
#include "OfferBaseRxSession.h"

#include <GuiInterface/IToGui.h>

#include <BigListLib/BigListInfo.h>
#include <ConnectIdListMgr/ConnectIdListMgr.h>

#include <OfferBase/OfferBaseInfo.h>
#include <OfferBase/OfferBaseRxSession.h>
#include <OfferBase/OfferBaseTxSession.h>
#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>

#include <PktLib/PktsOfferXfer.h>
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
	//#define		MAX_OFFER_XFER_OUTSTANDING_PKTS 3
	//#define		MAX_OFFER_XFER_TX_SESSIONS		5;
	const char* OFFER_XFER_DB_NAME = "OfferBaseXferDb.db3";

	//============================================================================
    static void * OfferBaseXferMgrThreadFunc( void * pvContext )
	{
		VxThread* poThread = (VxThread*)pvContext;
		poThread->setIsThreadRunning( true );
		OfferBaseXferMgr * poMgr = (OfferBaseXferMgr *)poThread->getThreadUserParam();
        if( poMgr )
        {
            poMgr->assetXferThreadWork( poThread );
        }

		poThread->threadAboutToExit();
        return nullptr;
	}
}


//============================================================================
OfferBaseXferMgr::OfferBaseXferMgr( P2PEngine& engine, OfferBaseMgr& offerMgr, PluginMessenger&	plugin, PluginSessionMgr& pluginSessionMgr, const char* stateDbName, EOfferMgrType offerMgrType )
: m_Initialized( false )
, m_Engine( engine )
, m_OfferMgr( offerMgr )
, m_PluginMgr( engine.getPluginMgr() )
, m_Plugin( plugin )
, m_PluginSessionMgr( pluginSessionMgr )
, m_OfferBaseXferDb( stateDbName )
{
}

//============================================================================
OfferBaseXferMgr::~OfferBaseXferMgr()
{
	clearRxSessionsList();
	clearTxSessionsList();
}

//============================================================================
void OfferBaseXferMgr::fromGuiUserLoggedOn( void )
{
	if( !m_Initialized )
	{
		m_Initialized = true;
		m_WorkerThread.startThread( (VX_THREAD_FUNCTION_T)OfferBaseXferMgrThreadFunc, this, "OfferBaseXferThrd" );			
	}
}

//============================================================================
void OfferBaseXferMgr::assetXferThreadWork( VxThread* workThread )
{
	if( workThread->isAborted() )
		return;
	// user specific directory should be set
	std::string dbName = VxGetSettingsDirectory();
	dbName += OFFER_XFER_DB_NAME; 
	lockOfferBaseQue();
	m_OfferBaseXferDb.dbShutdown();
	m_OfferBaseXferDb.dbStartup( 1, dbName );
	unlockOfferBaseQue();
	if( workThread->isAborted() )
		return;

	std::vector<VxGUID> assetToSendList;
	m_OfferBaseXferDb.getAllOffers( assetToSendList );
	if( 0 == assetToSendList.size() )
	{
		// nothing to do
		return;
	}

	while( ( false == m_OfferMgr.isOfferListInitialized() )
			&& ( false == workThread->isAborted() ) )
	{
		// waiting for assets to be available
		VxSleep( 500 );
	}

	if( workThread->isAborted() )
		return;

	std::vector<VxGUID>::iterator iter; 
	m_OfferMgr.lockResources();
	lockOfferBaseQue();
	for( iter = assetToSendList.begin(); iter != assetToSendList.end(); ++iter )
	{
		OfferBaseInfo* offerInfo = m_OfferMgr.findOffer( *iter );
		if( offerInfo )
		{
			m_OfferBaseSendQue.emplace_back( *offerInfo );
		}
		else
		{
			LogMsg( LOG_ERROR, "assetXferThreadWork removing asset not found in list" );
			m_OfferBaseXferDb.removeOffer( *iter );
		}
	}

	unlockOfferBaseQue();
	m_OfferMgr.unlockResources();
}

//============================================================================
void OfferBaseXferMgr::fromGuiCancelDownload( VxGUID& lclSessionId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	std::map<VxGUID, OfferBaseRxSession*>::iterator iter;
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelDownload AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelDownload AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	iter = m_RxSessions.find( lclSessionId );
	if( iter != m_RxSessions.end() )
	{
		OfferBaseRxSession* xferSession = iter->second;
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			m_RxSessions.erase( iter );
			xferSession->cancelDownload( lclSessionId );
			delete xferSession;
		}
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelDownload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::fromGuiCancelUpload( VxGUID& lclSessionId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelUpload AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelUpload AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	OfferBaseTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * xferSession = ( *iter );
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			m_TxSessions.erase( iter );
			xferSession->cancelUpload( lclSessionId );
			delete xferSession;
#ifdef DEBUG_AUTOPLUGIN_LOCK
			LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelUpload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
			return;
		}
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fromGuiCancelUpload AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}


//============================================================================
void OfferBaseXferMgr::clearRxSessionsList( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	std::map<VxGUID, OfferBaseRxSession*>::iterator iter;

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::clearRxSessionsList AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::clearRxSessionsList AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	for( iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		OfferBaseRxSession* xferSession = iter->second;
		delete xferSession;
	}

	m_RxSessions.clear();
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::clearRxSessionsList AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::clearTxSessionsList( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	OfferBaseTxIter iter;
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReq AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReq AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * xferSession = (*iter);
		delete xferSession;
	}

	m_TxSessions.clear();
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReq AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::fileAboutToBeDeleted( std::string& fileName )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fileAboutToBeDeleted AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fileAboutToBeDeleted AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	OfferBaseTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * xferSession = ( *iter );
		if( xferSession->getXferInfo().getLclFileName() == fileName )
		{
			m_TxSessions.erase( iter );
			xferSession->cancelUpload( xferSession->getXferInfo().getLclSessionId() );
			delete xferSession;
#ifdef DEBUG_AUTOPLUGIN_LOCK
			LogMsg( LOG_INFO, "OfferBaseXferMgr::fileAboutToBeDeleted AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
			return;
		}
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::fileAboutToBeDeleted AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onConnectionLost AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onConnectionLost AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	bool erasedSession = true;
	OfferBaseTxIter iter;
	while( erasedSession )
	{
		erasedSession = false;
		for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
		{
			OfferBaseTxSession * xferSession = ( *iter );
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
	OfferBaseRxIter oRxIter; 
	while( erasedSession )
	{
		erasedSession = false;
		for( oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
		{
			OfferBaseRxSession* xferSession = oRxIter->second;
			if( xferSession->getSkt() == sktBase )
			{				
				m_RxSessions.erase( oRxIter );
				xferSession->cancelDownload( xferSession->getXferInfo().getLclSessionId() );
				delete xferSession;
				erasedSession = true;
				break;
			}
		}
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onConnectionLost AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
bool OfferBaseXferMgr::requireFileXfer( EOfferType assetType )
{
	return eOfferTypePersonFile == assetType;
}

//============================================================================
void OfferBaseXferMgr::onPktOfferSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReq AutoPluginLock start");
	#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReq AutoPluginLock done");
	#endif // DEBUG_AUTOPLUGIN_LOCK

	PktOfferSendReq* poPkt = (PktOfferSendReq *)pktHdr;
	VxGUID& assetOfferId = poPkt->getOfferId();
	EOfferType assetType = (EOfferType)poPkt->getOfferType();
	bool needFileXfer = requireFileXfer( assetType );
	PktOfferSendReply pktReply;
	pktReply.setRequiresFileXfer( needFileXfer );
	pktReply.setError( 0 );
	pktReply.setRmtSessionId( poPkt->getLclSessionId() );
	pktReply.setLclSessionId( poPkt->getRmtSessionId() );
	pktReply.setOfferId( assetOfferId );
	if( false == netIdent->isHisAccessAllowedFromMe( m_Plugin.getPluginType() ) )
	{
		LogMsg( LOG_VERBOSE, "OfferBaseXferMgr::%s: permission denied", __func__ );
		pktReply.setError( eXferErrorPermission );
		m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply);
		return;
	}

	if( false == needFileXfer )
	{
		// all we need is in the send request
		OfferBaseInfo offerInfo;
		poPkt->fillOfferFromPkt( offerInfo );
		// make history id his id
		offerInfo.setHistoryId( netIdent->getMyOnlineId() );
		offerInfo.setOfferSendState( eOfferSendStateRxSuccess );
		m_OfferMgr.addOffer( offerInfo );
		m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply );
		m_OfferMgr.announceOfferAction( offerInfo.getOfferId(), eOfferActionRxSuccess, 100 );
		m_OfferMgr.announceOfferAction( offerInfo.getCreatorId(), eOfferActionRxNotifyNewMsg, 100 );
	}
	else
	{
		OfferBaseRxSession* xferSession = findOrCreateRxSession( true, poPkt->getRmtSessionId(), poPkt->getSrcOnlineId(), sktBase);
		if( xferSession )
		{
			OfferBaseInfo& offerInfo = xferSession->getOfferInfo();
			poPkt->fillOfferFromPkt( offerInfo );
			// make history id his id
			offerInfo.setHistoryId( netIdent->getMyOnlineId() );
			offerInfo.setOfferSendState( eOfferSendStateRxProgress );

			xferSession->setRmtSessionId( poPkt->getLclSessionId() );
			pktReply.setLclSessionId( xferSession->getLclSessionId() );
			EXferError xferErr = beginOfferBaseReceive( xferSession, poPkt, pktReply );
			if( eXferErrorNone != xferErr )
			{
				//IToGui::getIToGui().toGuiUpdateOfferDownload( xferSession->getLclSessionId(), 0, rc );
				endOfferBaseXferSession( xferSession, true );
			}
		}
		else
		{
			LogMsg(LOG_ERROR, "PluginOfferBaseOffer::%s Could not create session", __func__);
			PktOfferSendReply pktReply;
			pktReply.setError( eXferErrorBadParam );
			m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply );
		}
	}
}

//============================================================================
void OfferBaseXferMgr::assetSendComplete( OfferBaseTxSession * xferSession )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	updateOfferMgrSendState( xferSession->getOfferInfo().getOfferId(), eOfferSendStateTxSuccess, 100 );
}

//============================================================================
void OfferBaseXferMgr::onPktOfferSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	PktOfferSendReply * poPkt = (PktOfferSendReply *)pktHdr;
	VxGUID&	assetOfferId =	poPkt->getOfferId();
	OfferBaseInfo* offerInfo = m_OfferMgr.findOffer( assetOfferId );
	if( 0 == offerInfo )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s failed to find asset id", __func__ );
		updateOfferMgrSendState( assetOfferId, eOfferSendStateTxFail, 0 );

		return;
	}

	bool isFileXfer = (bool)poPkt->getRequiresFileXfer();
	uint32_t rxedErrCode = poPkt->getError();
	OfferBaseTxSession * xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );

	if( xferSession )
	{
		xferSession->setRmtSessionId( poPkt->getLclSessionId() );
		if( 0 == rxedErrCode )
		{
			if( isFileXfer )
			{
				// we did txNextOfferBaseChunk in begin file send
				//int32_t rc = txNextOfferBaseChunk( xferSession );
				//if( rc )
				//{
				//	//IToGui::getIToGui().toGuiUpdateOfferUpload( xferSession->getLclSessionId(), 0, rc );
				//	LogMsg( LOG_ERROR, "OfferBaseXferMgr::onPktOfferSendReply beginOfferBaseSend returned error %d", rc );
				//	endOfferBaseXferSession( xferSession, true );
				//}
			}
			else
			{
				assetSendComplete( xferSession );
				endOfferBaseXferSession( xferSession, true, false );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s PktOfferSendReply returned error %d", __func__, poPkt->getError() );
			endOfferBaseXferSession( xferSession, true, false );
			updateOfferMgrSendState( assetOfferId, eOfferSendStateTxFail, rxedErrCode );
		}
	}
	else
	{
		if( isFileXfer )
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s failed to find session", __func__ );
			updateOfferMgrSendState( assetOfferId, eOfferSendStateTxFail, rxedErrCode );
		}
		else
		{
			updateOfferMgrSendState( assetOfferId, rxedErrCode ? eOfferSendStateTxFail : eOfferSendStateTxSuccess, rxedErrCode );
		}
	}
}

//============================================================================
void OfferBaseXferMgr::onPktOfferChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	OfferBaseRxSession* xferSession = 0;
	PktOfferChunkReq* poPkt = (PktOfferChunkReq *)pktHdr;
	{ // scope for lock

		PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

		if( poPkt->getRmtSessionId().isValid() )
		{
			xferSession = findRxSessionSessionId( true, poPkt->getRmtSessionId() );
		}

		if( xferSession )
		{
			EXferError xferErr = rxOfferBaseChunk( true, xferSession, poPkt );
			if( eXferErrorNone != xferErr )
			{

				PktOfferChunkReply pktReply;
				pktReply.setLclSessionId( xferSession->getLclSessionId() );
				pktReply.setRmtSessionId( poPkt->getLclSessionId() );
				pktReply.setDataLen(0);
				pktReply.setError( xferErr );
				m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply);

				m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(),eOfferActionRxError, xferErr );
				endOfferBaseXferSession( xferSession, true );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s failed to find session", __func__ );
			PktOfferChunkReply pktReply;
			pktReply.setLclSessionId( poPkt->getRmtSessionId() );
			pktReply.setRmtSessionId( poPkt->getLclSessionId() );
			pktReply.setDataLen(0);
			pktReply.setError( eXferErrorBadParam );
			m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply );
		}

#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendReply AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}
}

//============================================================================
void OfferBaseXferMgr::onPktOfferChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktOfferChunkReply * poPkt = (PktOfferChunkReply *)pktHdr;
	OfferBaseTxSession * xferSession = 0;
static int cnt = 0;
	cnt++;
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s cnt %d", __func__, cnt );

	{ // scope for lock
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferChunkReply AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferChunkReply AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
		if( poPkt->getRmtSessionId().isValid() )
		{
			xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );
		}

		if( xferSession )
		{
			EXferError xferErr = txNextOfferBaseChunk( xferSession, poPkt->getError(), true );
			if( eXferErrorNone != xferErr )
			{
				//IToGui::getIToGui().toGuiUpdateOfferUpload( xferSession->getLclSessionId(), 0, rc );
				endOfferBaseXferSession( xferSession, true, false );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s failed to find session", __func__ );
		}

		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s done %d", __func__, cnt );
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferChunkReply AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}
}

//============================================================================
void OfferBaseXferMgr::onPktOfferSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReq AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReq AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK

	PktOfferSendCompleteReq* poPkt = (PktOfferSendCompleteReq *)pktHdr;
	OfferBaseRxSession* xferSession = findRxSessionSessionId( true, poPkt->getRmtSessionId() );
	//TODO check checksum
	if( xferSession )
	{
		finishOfferBaseReceive( xferSession, poPkt, true );
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReq AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::onPktOfferSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReply AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReply AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK

	PktOfferSendCompleteReply * poPkt = (PktOfferSendCompleteReply *)pktHdr;
	OfferBaseTxSession * xferSession = findTxSessionSessionId( true, poPkt->getRmtSessionId() );
	if( xferSession )
	{
		VxFileXferInfo xferInfo = xferSession->getXferInfo();
		LogMsg( LOG_INFO, "OfferBaseXferMgr:: Done Sending file %s", xferInfo.getLclFileNameAndPath().c_str() );
		//m_PluginMgr.getToGui().toGuiOfferBaseUploadComplete( xferInfo.getLclSessionId(), 0 );
		onOfferBaseSent( xferSession, xferSession->getOfferInfo(), (EXferError)poPkt->getError(), true );
	}
	else
	{
		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s failed to find session", __func__ );
		updateOfferMgrSendState( poPkt->getOfferId(), eOfferSendStateTxSuccess, 100 );
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::onPktOfferSendCompleteReply AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::onPktOfferBaseXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	// TODO handle error
}

//============================================================================
void OfferBaseXferMgr::endOfferBaseXferSession( OfferBaseRxSession* poSessionIn, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
	}

	std::string fileName = xferInfo.getDownloadIncompleteFileName();
	if( fileName.length() )
	{
		VxFileUtil::deleteFile( fileName.c_str() );
	}

	OfferBaseRxIter oRxIter = m_RxSessions.begin();
	while( oRxIter != m_RxSessions.end() )
	{
		OfferBaseRxSession* xferSession = oRxIter->second;
		if( poSessionIn == xferSession )
		{
			m_RxSessions.erase( oRxIter );
			delete xferSession;
			break;
		}
		else
		{
			++oRxIter;
		}
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}
}

//============================================================================
void OfferBaseXferMgr::endOfferBaseXferSession( OfferBaseTxSession * poSessionIn, bool pluginIsLocked, bool requeOffer )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	//if( requeOffer )
	//{
	//	queOffer( poSessionIn->getOfferInfo() );
	//}

	VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
	}

	OfferBaseTxIter iter = m_TxSessions.begin();
	while( iter != m_TxSessions.end() )
	{
		OfferBaseTxSession * xferSession = (*iter);
		if( xferSession == poSessionIn )
		{
			m_TxSessions.erase( iter );
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
		LogMsg( LOG_INFO, "OfferBaseXferMgr::endOfferBaseXferSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}
}

//============================================================================
OfferBaseRxSession* OfferBaseXferMgr::findRxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseRxIter iter;
	for( iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		OfferBaseRxSession* xferSession = iter->second;
		if( xferSession->getSendToId() == sendToId )
		{
			if( false == pluginIsLocked )
			{
#ifdef DEBUG_AUTOPLUGIN_LOCK
				LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
				pluginMutex.unlock();
			}

			return  xferSession;
		}
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return NULL;
}

//============================================================================
OfferBaseRxSession* OfferBaseXferMgr::findRxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseRxIter iter = m_RxSessions.find( lclSessionId );
	if( iter != m_RxSessions.end() )
	{
		OfferBaseRxSession* rxSession = iter->second;
		if( false == pluginIsLocked )
		{
#ifdef DEBUG_AUTOPLUGIN_LOCK
			LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
			pluginMutex.unlock();
		}

		return rxSession;
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return NULL;
}

//============================================================================
OfferBaseRxSession*	OfferBaseXferMgr::findOrCreateRxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseRxSession* xferSession = findRxSessionSendToId( true, sendToId );
	if( NULL == xferSession )
	{
		xferSession = new OfferBaseRxSession( m_Engine, m_OfferMgr, sktBase, sendToId );
		m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return xferSession;
}

//============================================================================
OfferBaseRxSession*	OfferBaseXferMgr::findOrCreateRxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	if( false == lclSessionId.isValid() )
	{
		lclSessionId.initializeWithNewVxGUID();
	}

	OfferBaseRxSession* xferSession = findRxSessionSessionId( true, lclSessionId );
	if( NULL == xferSession )
	{
		xferSession = new OfferBaseRxSession( m_Engine, m_OfferMgr, lclSessionId, sktBase, sendToId );
		m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateRxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return xferSession;
}

//============================================================================
OfferBaseTxSession * OfferBaseXferMgr::findTxSessionSendToId( bool pluginIsLocked, VxGUID& sendToId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * txSession = ( *iter );
		if( txSession->getSendToId() == sendToId )
		{
			if( false == pluginIsLocked )
			{
#ifdef DEBUG_AUTOPLUGIN_LOCK
				LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
				pluginMutex.unlock();
			}

			return txSession;
		}
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return NULL;
}

//============================================================================
OfferBaseTxSession * OfferBaseXferMgr::findTxSessionSessionId( bool pluginIsLocked, VxGUID& lclSessionId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s pluginMutex.lock start", __func__);
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s pluginMutex.lock done", __func__);
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * txSession = ( *iter );
		if( txSession->getLclSessionId() == lclSessionId )
		{
			if( false == pluginIsLocked )
			{
#ifdef DEBUG_AUTOPLUGIN_LOCK
				LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
				pluginMutex.unlock();
			}

			return txSession;
		}
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return NULL;
}

//============================================================================
OfferBaseTxSession * OfferBaseXferMgr::createTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	OfferBaseTxSession * txSession = new OfferBaseTxSession( m_Engine, m_OfferMgr, sktBase, sendToId );
	return txSession;
}

//============================================================================
OfferBaseTxSession * OfferBaseXferMgr::findOrCreateTxSession( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseTxSession * xferSession = findTxSessionSendToId( true, sendToId );
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
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return xferSession;
}

//============================================================================
OfferBaseTxSession*	OfferBaseXferMgr::findOrCreateTxSession( bool pluginIsLocked, VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.lock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.lock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	OfferBaseTxSession * xferSession = 0;
	if( lclSessionId.isValid() )
	{
		xferSession = findTxSessionSessionId( true, lclSessionId );
	}
	else
	{
		xferSession = findTxSessionSendToId( true, sendToId );
	}

	if( NULL == xferSession )
	{
		xferSession = new OfferBaseTxSession( m_Engine, m_OfferMgr, lclSessionId, sktBase, sendToId );
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
		LogMsg( LOG_INFO, "OfferBaseXferMgr::findOrCreateTxSession pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return xferSession;
}

//============================================================================
void OfferBaseXferMgr::fromGuiSendOfferBase( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	bool xferFailed = true;
	VxGUID sendToId = offerInfo.getSendToId();

	std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( offerInfo.getSendToId() );

	if( sktBase && sktBase->isConnected() )
	{
		EXferError xferError = createOfferTxSessionAndSend( false, offerInfo, offerInfo.getSendToId(), sktBase );
		if( xferError == eXferErrorNone )
		{
			xferFailed = false;
		}
	}
	else
	{
		queOffer( offerInfo );
	}

	if( xferFailed )
	{
		onTxFailed( offerInfo.getOfferId(), false );
	}
}

//============================================================================
void OfferBaseXferMgr::onTxFailed( VxGUID& assetOfferId, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	updateOfferMgrSendState( assetOfferId, eOfferSendStateTxFail, 0 );
}

//============================================================================
void OfferBaseXferMgr::onTxSuccess( VxGUID& assetOfferId, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	updateOfferMgrSendState( assetOfferId, eOfferSendStateTxSuccess, 0 );
}

//============================================================================
void OfferBaseXferMgr::updateOfferMgrSendState( VxGUID& assetOfferId, EOfferSendState sendState, int param )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	m_OfferMgr.updateOfferXferState( assetOfferId, sendState, param );
}

//============================================================================
void OfferBaseXferMgr::queOffer( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	m_OfferBaseSendQueMutex.lock();
	bool foundOfferBase = false;
    std::vector<OfferBaseInfo>::iterator iter;
	for( iter = m_OfferBaseSendQue.begin(); iter != m_OfferBaseSendQue.end(); ++iter )
	{
		if( (*iter).getOfferId() == offerInfo.getOfferId() )
		{
			foundOfferBase = true;
			break;
		}
	}

	if( false == foundOfferBase )
	{
		m_OfferBaseSendQue.emplace_back( offerInfo );
	}

	m_OfferBaseSendQueMutex.unlock();
}

//============================================================================
EXferError OfferBaseXferMgr::createOfferTxSessionAndSend( bool pluginIsLocked, OfferBaseInfo& offerInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	VxMutex& pluginMutex = m_Plugin.getPluginMutex();
	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s pluginMutex.lock start", __func__);
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.lock();
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s pluginMutex.lock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
	}

	EXferError xferErr = eXferErrorNone;
	OfferBaseTxSession * txSession = createTxSession( sendToId, sktBase );
	if( false == txSession->getLclSessionId().isValid() )
	{
		txSession->getLclSessionId().initializeWithNewVxGUID();
	}

	if( false == txSession->getRmtSessionId().isValid() )
	{
		txSession->setRmtSessionId( txSession->getLclSessionId() );
	}

	txSession->setOfferInfo( offerInfo );
	VxFileXferInfo& xferInfo = txSession->getXferInfo();
	xferInfo.setLclSessionId( txSession->getLclSessionId() );
	xferInfo.setRmtSessionId( txSession->getRmtSessionId() );
	xferInfo.setXferDirection( eXferDirectionTx );

	m_TxSessionsMutex.lock();
	addTxSession( txSession );
	m_TxSessionsMutex.unlock();

	updateOfferMgrSendState( offerInfo.getOfferId(), eOfferSendStateTxProgress, 0 );
	if( offerInfo.hasFileName() )
	{
		// need to do first so file handle is set before get asset send reply back
		xferErr = beginOfferBaseSend( txSession );
	}
	else
	{
		// all data was in the request packet .. we just wait for reply
	}

	if( eXferErrorNone != xferErr )
	{
		// failed to open file
		updateOfferMgrSendState( offerInfo.getOfferId(), eOfferSendStateTxFail, xferErr );
		endOfferBaseXferSession( txSession, true, false );
		if( false == pluginIsLocked )
		{
#ifdef DEBUG_AUTOPLUGIN_LOCK
			LogMsg( LOG_INFO, "OfferBaseXferMgr::createOfferTxSessionAndSend pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
			pluginMutex.unlock();
		}

		return xferErr;
	}

	PktOfferSendReq sendReq;
	sendReq.fillPktFromOffer( offerInfo );
	sendReq.setLclSessionId( xferInfo.getLclSessionId() );
	sendReq.setRmtSessionId( xferInfo.getRmtSessionId() );
	if( false == m_PluginMgr.pluginApiTxPacket( m_Plugin.getPluginType(), sendToId, sktBase, &sendReq ) )
	{
		xferErr = eXferErrorDisconnected;
	}	

	if( eXferErrorNone == xferErr )
	{
		// re que for try some other time
		if( requireFileXfer( offerInfo.getOfferType() ) )
		{
			xferErr = txNextOfferBaseChunk( txSession, eXferErrorNone, true );
		}
	}

	if( eXferErrorNone != xferErr )
	{
		// re que for try some other time
		updateOfferMgrSendState( offerInfo.getOfferId(), eOfferSendStateTxFail, xferErr );
		endOfferBaseXferSession( txSession, true, ( ( eXferErrorFileNotFound == xferErr ) || ( eXferErrorDisconnected == xferErr ) ) ? false : true );
	}

	if( false == pluginIsLocked )
	{
#ifdef DEBUG_AUTOPLUGIN_LOCK
		LogMsg( LOG_INFO, "OfferBaseXferMgr::createOfferTxSessionAndSend pluginMutex.unlock");
#endif // DEBUG_AUTOPLUGIN_LOCK
		pluginMutex.unlock();
	}

	return xferErr;
}

//============================================================================
EXferError OfferBaseXferMgr::beginOfferBaseSend( OfferBaseTxSession * xferSession )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	EXferError xferErr = eXferErrorNone;
	xferSession->clearErrorCode();
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( xferInfo.m_hFile )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s: ERROR: OfferBase transfer still in progress", __func__ );
		xferErr = eXferErrorAlreadyUploading;
	}

	if( eXferErrorNone == xferErr )
	{
		xferInfo.setXferDirection( eXferDirectionTx );
		xferInfo.setLclFileName( xferSession->getOfferInfo().getOfferName().c_str() );
		VxFileUtil::getFileName( xferSession->getOfferInfo().getOfferName().c_str(), xferInfo.getRmtFileName() );
		xferInfo.setLclSessionId( xferSession->getLclSessionId() );
		xferInfo.setRmtSessionId( xferSession->getRmtSessionId() );
		xferInfo.setFileHashId( xferSession->getFileHashId() );

		xferInfo.m_u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str() );
		if( 0 == xferInfo.m_u64FileLen )
		{
			// no file found to send
			LogMsg( LOG_INFO, "OfferBaseXferMgr::%s OfferBase %s not found to send", __func__, xferInfo.getLclFileNameAndPath().c_str() );
			xferErr = eXferErrorFileNotFound;
		}
		else if( false == xferInfo.getFileHashId().isHashValid() )
		{
			// see if we can get hash from shared files
			//if( !m_SharedOfferBasesMgr.getOfferHashId( xferInfo.getLclFileNameAndPath(), xferInfo.getFileHashId() ) )
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
			LogMsg( LOG_INFO, "OfferBaseXferMgr::%s Could not open OfferBase %s", __func__, xferInfo.getLclFileNameAndPath().c_str() );
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
				LogMsg( LOG_INFO, "OfferBaseXferMgr::%s: OfferBase %s could not be resumed because too short", __func__, 
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
					LogMsg( LOG_INFO, "OfferBaseXferMgr::%s: could not seek to position %d in file %s", __func__,
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
EXferError OfferBaseXferMgr::beginOfferBaseReceive( OfferBaseRxSession* xferSession, PktOfferSendReq* poPkt, PktOfferSendReply& pktReply )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	if( !xferSession )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s NULL skt info", __func__ );
		return eXferErrorBadParam;
	}

	EXferError xferErr = eXferErrorNone;
	uint64_t u64FileLen;
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( poPkt->getError() )
	{
		//IToGui::getIToGui().toGuiUpdateOfferDownload( poPkt->getRmtSessionId(), 0, poPkt->getError() );
		xferErr = (EXferError)poPkt->getError();
		return xferErr;
	}

	if( eXferErrorNone == xferErr )
	{
		if( xferInfo.m_hFile )
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s: ERROR:(OfferBase Receive) receive transfer still in progress", __func__ );
			xferErr = eXferErrorAlreadyDownloading;
		}
	}

	if( eXferErrorNone == xferErr )
	{
		// get file information
		xferInfo.setFileHashId( poPkt->getOfferHashId() );
		xferInfo.setRmtSessionId( poPkt->getLclSessionId() );
		if( false == xferInfo.getLclSessionId().isValid() )
		{
			xferInfo.getLclSessionId().initializeWithNewVxGUID();
		}

		xferInfo.setRmtFileName( poPkt->getOfferName().c_str() );
		if( 0 == xferInfo.getRmtFileName().length() )
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s: ERROR: No file Name", __func__ );
			xferErr = eXferErrorBadParam;
		}	
	}

	std::string strRmtPath;
	std::string strRmtOfferBaseNameOnly;
    VxFileUtil::seperatePathAndFile(		xferInfo.getRmtFileName(),
                                            strRmtPath,
                                            strRmtOfferBaseNameOnly );


	if( eXferErrorNone == xferErr )
	{
		// make full path
		if( 0 == strRmtOfferBaseNameOnly.length() )
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s: ERROR: NULL file Name %s", __func__,  xferInfo.getRmtFileName().c_str() );
			xferErr = eXferErrorBadParam;
		}
	}

	if( eXferErrorNone == xferErr )
	{
		VxFileUtil::makeFullPath( strRmtOfferBaseNameOnly.c_str(), VxGetIncompleteDirectory().c_str(), xferInfo.getLclFileName() );
		std::string strPath;
		std::string strOfferBaseNameOnly;
		int32_t rc = VxFileUtil::seperatePathAndFile(	xferInfo.getLclFileNameAndPath(),			
													strPath,			
													strOfferBaseNameOnly );	
		VxFileUtil::makeDirectory( strPath );
		xferInfo.m_u64FileLen = poPkt->getOfferLen();
		xferInfo.m_u64FileOffs = poPkt->getOfferOffset();
		u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str() );

		if( 0 != xferInfo.m_u64FileOffs )
		{
			if( u64FileLen < xferInfo.m_u64FileOffs )
			{
				xferErr  = eXferErrorFileSeekError;
				LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR:(OfferBase Send) %d OfferBase %s could not be resumed because too short", 
					 __func__, rc,
					xferInfo.getLclFileNameAndPath().c_str() );
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

					LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR:(OfferBase Send) %d OfferBase %s could not be created", __func__, 
						rc,
						xferInfo.getLclFileNameAndPath().c_str() );
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
						LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR: (OfferBase Send) could not seek to position %d in file %s", __func__,
							xferInfo.m_u64FileOffs,
							xferInfo.getLclFileNameAndPath().c_str() );
					}
				}
			}
		}
		else
		{
			// open file and truncate if exists
			xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "wb+" ); // pointer to name of the file
			if( NULL == xferInfo.m_hFile )
			{
				// failed to open file
				xferInfo.m_hFile = NULL;
				rc = VxGetLastError();
				xferSession->setErrorCode( rc );
				xferErr = eXferErrorFileCreateError;

				LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR: %d OfferBase %s could not be created", __func__, 
					rc,
					xferInfo.getLclFileNameAndPath().c_str() );
			}
		}
	}

	if( eXferErrorNone == xferErr )
	{
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s (OfferBase Send) start recieving file %s", __func__, 
			xferInfo.getLclFileNameAndPath().c_str() );
		poPkt->fillOfferFromPkt( xferSession->getOfferInfo() );
	}

	pktReply.setError( xferErr );
	pktReply.setOfferOffset( xferInfo.m_u64FileOffs );
	if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply ) )
	{
		xferErr = eXferErrorDisconnected;
	}

	return xferErr;
}

//============================================================================
EXferError OfferBaseXferMgr::txNextOfferBaseChunk( OfferBaseTxSession * xferSession, uint32_t remoteErr, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	if( 0 == xferSession )
	{
		return eXferErrorBadParam;
	}

	EXferError xferErr = eXferErrorNone;
	// fill the packet with data from the file
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( 0 != remoteErr )
	{
		// canceled download by remote user
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s Cancel Sending file %s", __func__, xferInfo.getLclFileNameAndPath().c_str() );
		onOfferBaseSent( xferSession, xferSession->getOfferInfo(), eXferErrorCanceled, pluginIsLocked );
		return eXferErrorCanceled;
	}

	vx_assert( xferInfo.m_hFile );
	vx_assert( xferInfo.m_u64FileLen );
	if( xferInfo.m_u64FileOffs >= xferInfo.m_u64FileLen )
	{
		//we are done sending file
		if( xferInfo.m_hFile )
		{
			VFileClose( xferInfo.m_hFile );
			xferInfo.m_hFile  = NULL;
		}

		PktOfferSendCompleteReq oPkt;
		oPkt.setLclSessionId( xferSession->getLclSessionId() );
		oPkt.setRmtSessionId( xferSession->getRmtSessionId() );
		oPkt.setOfferId( xferSession->getOfferInfo().getOfferId() );
		m_Plugin.txPacket(  xferSession->getSendToId(), xferSession->getSkt(), &oPkt );

		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s Done Sending file %s", __func__, xferInfo.getLclFileNameAndPath().c_str() );
		onOfferBaseSent( xferSession, xferSession->getOfferInfo(), eXferErrorNone, pluginIsLocked );
		return eXferErrorNone;
	}

	PktOfferChunkReq oPkt;
	// see how much we can read
	uint32_t u32ChunkLen = (uint32_t)(xferInfo.m_u64FileLen - xferInfo.m_u64FileOffs);
	if( PKT_TYPE_OFFER_MAX_DATA_LEN < u32ChunkLen )
	{
		u32ChunkLen = PKT_TYPE_OFFER_MAX_DATA_LEN;
	}

	if( 0 == u32ChunkLen )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s 0 len u32ChunkLen", __func__ );
		// what to do?
		return eXferErrorNone;
	}

	// read data into packet
	uint32_t u32BytesRead = (uint32_t)VFileRead(	oPkt.m_au8OfferChunk,
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
		LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR: %d reading send file at offset %" PRId64 " when file len %" PRId64 "  file name %s", __func__,
					rc,
					xferInfo.m_u64FileOffs,
					xferInfo.m_u64FileLen,
					xferInfo.getLclFileNameAndPath().c_str() );
	}
	else
	{
		xferInfo.m_u64FileOffs += u32ChunkLen;
        oPkt.setChunkLen( (uint16_t)u32ChunkLen );
		oPkt.setLclSessionId( xferInfo.getLclSessionId() );
		oPkt.setRmtSessionId( xferInfo.getRmtSessionId() );
	}

	if( eXferErrorNone == xferErr )
	{
		if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &oPkt ) )
		{
			xferErr = eXferErrorDisconnected;
		}
	}

	if( eXferErrorNone != xferErr )
	{
		m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionTxError, xferErr );
	}
	else
	{
		if( xferInfo.calcProgress() )
		{
			m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionTxProgress, xferInfo.getProgress() );
		}
	}

	return xferErr;
}

//============================================================================
EXferError OfferBaseXferMgr::rxOfferBaseChunk( bool pluginIsLocked, OfferBaseRxSession* xferSession, PktOfferChunkReq* poPkt )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	if( NULL == xferSession )
	{
		return eXferErrorBadParam;
	}

	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	EXferError xferErr = (EXferError)poPkt->getError();
	if( eXferErrorNone != xferErr )
	{
		// canceled by sender
		return xferErr;
	}

	// we are receiving a file
	if( xferInfo.m_hFile )
	{
		//write the chunk of data out to the file
		uint32_t u32BytesWritten = (uint32_t)VFileWrite(	poPkt->m_au8OfferChunk,
												1,
												poPkt->getChunkLen(),
												xferInfo.m_hFile );
		if( u32BytesWritten != poPkt->getChunkLen() ) 
		{
			int32_t rc = VxGetLastError();
			xferSession->setErrorCode( rc );
			xferErr = eXferErrorFileWriteError;

			LogMsg( LOG_INFO, "OfferBaseXferMgr::%s ERROR %d: writing to file %s", __func__,
							rc,
							xferInfo.getLclFileNameAndPath().c_str() );
		}
		else
		{
			//successfully write
			xferInfo.m_u64FileOffs += poPkt->getChunkLen();

			PktOfferChunkReply oPkt;
			oPkt.setDataLen( poPkt->getDataLen() );
			oPkt.setLclSessionId( xferInfo.getLclSessionId() );
			oPkt.setRmtSessionId( xferInfo.getRmtSessionId() );

			if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &oPkt ) )
			{
				xferErr = eXferErrorDisconnected;
			}
		}
	}

	if( eXferErrorNone == xferErr )
	{
		if( xferInfo.calcProgress() )
		{
			m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionRxProgress, xferInfo.getProgress() );
		}
	}
	else
	{
		m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionRxError, xferErr );
	}

	return xferErr;
}

//============================================================================
void OfferBaseXferMgr::finishOfferBaseReceive( OfferBaseRxSession* xferSession, PktOfferSendCompleteReq* poPkt, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	// done receiving file
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
	}
	else
	{
		LogMsg( LOG_ERROR,  "OfferBaseXferMgr::%s: NULL file handle", __func__ );
	}

	//// let other act on the received file
	std::string strOfferBaseName = xferInfo.getLclFileName();

	PktOfferSendCompleteReply oPkt;
	oPkt.setLclSessionId( xferInfo.getLclSessionId() );
	oPkt.setRmtSessionId( xferInfo.getRmtSessionId() );
	oPkt.setOfferId( xferSession->getOfferInfo().getOfferId() );
	m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &oPkt );
	LogMsg( LOG_INFO,  "OfferBaseXferMgr::%s Done Receiving file %s", __func__, strOfferBaseName.c_str() );

	xferSession->setErrorCode( poPkt->getError() );
	onOfferBaseReceived( xferSession, xferSession->getOfferInfo(), (EXferError)poPkt->getError(), pluginIsLocked );
}

//============================================================================
void OfferBaseXferMgr::onOfferBaseReceived( OfferBaseRxSession* xferSession, OfferBaseInfo& offerInfo, EXferError error, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	//m_PluginMgr.getToGui().toGuiFileDownloadComplete( xferSession->getLclSessionId(), error );
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( eXferErrorNone == error )
	{
		std::string incompleteOffer = xferInfo.getDownloadIncompleteFileName();
		std::string completedOfferBase = xferInfo.getDownloadCompleteFileName();
		int32_t rc = 0;
		if( 0 == ( rc = VxFileUtil::moveAFile( incompleteOffer.c_str(), completedOfferBase.c_str() ) ) )
		{
			offerInfo.setOfferName( completedOfferBase );
			offerInfo.setHistoryId( xferSession->getSendToId() );

			if( eXferErrorNone == error )
			{
				offerInfo.setOfferSendState( eOfferSendStateRxSuccess );
			}
			else
			{
				offerInfo.setOfferSendState(  eOfferSendStateRxFail );
			}

			m_OfferMgr.addOffer( offerInfo );
			FileInfo fileInfo( offerInfo );
			m_Engine.fromGuiSetFileIsInLibrary( fileInfo, true );
			if( eXferErrorNone == error )
			{
				m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionRxSuccess, 0 );
			}
			else
			{
				m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionRxError, error );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s ERROR %d moving %s to %s", __func__, rc, incompleteOffer.c_str(), completedOfferBase.c_str() );
		}
	}

	endOfferBaseXferSession( xferSession, pluginIsLocked );
}

//============================================================================
void OfferBaseXferMgr::onOfferBaseSent( OfferBaseTxSession * xferSession, OfferBaseInfo& offerInfo, EXferError error, bool pluginIsLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	//m_PluginMgr.getToGui().toGuiOfferBaseUploadComplete( xferSession->getRmtSessionId(), error );
	std::shared_ptr<VxSktBase>& sktBase		= xferSession->getSkt();
	VxGUID sendToId = xferSession->getSendToId();
	if( eXferErrorNone != error )
	{
		updateOfferMgrSendState( offerInfo.getOfferId(), eOfferSendStateTxFail, (int)error );
		m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionTxError, error );
	}
	else
	{
		updateOfferMgrSendState( offerInfo.getOfferId(), eOfferSendStateTxSuccess, (int)error );
		m_OfferMgr.announceOfferAction( xferSession->getOfferInfo().getOfferId(), eOfferActionTxSuccess, 0 );
	}

	endOfferBaseXferSession( xferSession, pluginIsLocked, false );
	if( sktBase && sktBase->isConnected() && false == VxIsAppShuttingDown() )
	{
		checkQueForMoreOffersToSend( pluginIsLocked, sendToId, sktBase );
	}
}

//============================================================================
void OfferBaseXferMgr::checkQueForMoreOffersToSend( bool pluginIsLocked, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	// check que and start next xfer

    std::vector<OfferBaseInfo>::iterator iter;

	m_OfferBaseSendQueMutex.lock();
	for( iter = m_OfferBaseSendQue.begin(); iter != m_OfferBaseSendQue.end(); ++iter )
	{
		if( sendToId == (*iter).getSendToId() )
		{
			// found asset to send
			OfferBaseInfo& offerInfo = (*iter);
			int32_t rc = createOfferTxSessionAndSend( pluginIsLocked, offerInfo, sendToId, sktBase );
			if( 0 == rc )
			{
				m_OfferBaseSendQue.erase(iter);
			}

			break;
		}
	}

	m_OfferBaseSendQueMutex.unlock();
}

//============================================================================
void OfferBaseXferMgr::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	OfferBaseTxIter iter;
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::replaceConnection AutoPluginLock start");
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::replaceConnection AutoPluginLock done");
#endif // DEBUG_AUTOPLUGIN_LOCK
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		OfferBaseTxSession * xferSession = (*iter);
		if( xferSession->getSkt() == poOldSkt )
		{
			xferSession->setSkt( poNewSkt );
		}
	}

	OfferBaseRxIter oRxIter;
	for( oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
	{
		OfferBaseRxSession* xferSession = oRxIter->second;
		if( xferSession->getSkt() == poOldSkt )
		{
			xferSession->setSkt( poNewSkt );
		}
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogMsg( LOG_INFO, "OfferBaseXferMgr::replaceConnection AutoPluginLock destroy");
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void OfferBaseXferMgr::onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseXferMgr::%s", __func__ );
	checkQueForMoreOffersToSend( false, netIdent->getMyOnlineId(), sktBase);
}

//============================================================================
void OfferBaseXferMgr::addTxSession( OfferBaseTxSession* xferSession )
{
	for( auto txSession : m_TxSessions )
	{
		if( xferSession == txSession )
		{
			LogMsg( LOG_ERROR, "OfferBaseXferMgr::%s attempted to add same session again", __func__ );
			return;
		}
	}

	m_TxSessions.emplace_back( xferSession );
}