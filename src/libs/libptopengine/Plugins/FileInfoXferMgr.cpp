//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileInfoXferMgr.h"

#include "FileTxSession.h"
#include "FileRxSession.h"

#include "FileInfoBaseMgr.h"

#include "PluginBase.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>
#include <OfferBase/OfferBaseInfo.h>
#include <OfferBase/OfferMgr.h>

#include <PktLib/VxSearchDefs.h>
#include <PktLib/PktsFileShare.h>
#include <PktLib/PktsFileList.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/PktsStreamCtrl.h>

#include <CoreLib/AppErr.h>
#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#include <NetLib/VxSktBase.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <array>

namespace
{
	const int MAX_OUTSTANDING_STREAM_PKTS = 25;
};

//============================================================================
FileInfoXferMgr::FileInfoXferMgr( P2PEngine& engine, PluginBase& plugin, FileInfoBaseMgr& fileInfoMgr )
: m_Engine( engine )
, m_Plugin( plugin )
, m_PluginMgr( engine.getPluginMgr() )
, m_FileInfoMgr( fileInfoMgr )
, m_eFileRxOption( eFileXOptionReplaceIfExists )
{
	if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s %s ", __func__, DescribePluginType( plugin.getPluginType() ) );
}

//============================================================================
FileInfoXferMgr::~FileInfoXferMgr()
{
	clearRxSessionsList();
	clearTxSessionsList();
}

//============================================================================
EPluginType FileInfoXferMgr::getPluginType( void ) 
{ 
	return m_Plugin.getPluginType(); 
}

//============================================================================
void FileInfoXferMgr::clearRxSessionsList( void )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	for( auto iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		FileRxSession* xferSession = iter->second;
		delete xferSession;
	}

	m_RxSessions.clear();
}

//============================================================================
void FileInfoXferMgr::clearTxSessionsList( void )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	FileTxIter iter;
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* xferSession = (*iter);
		delete xferSession;
	}

	m_TxSessions.clear();
}

//============================================================================
void FileInfoXferMgr::onAfterUserLogOnThreaded( void )
{
}

//============================================================================
// returns -1 if unknown else percent downloaded
int FileInfoXferMgr::fromGuiGetFileDownloadState( uint8_t * fileHashId )
{
	int result = -1;

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	for( auto iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		FileRxSession* xferSession = iter->second;
		if( xferSession->getXferInfo().getFileHashId().isEqualTo( fileHashId ) )
		{
			result = xferSession->getXferInfo().getProgress();
			break;
		}
	}

	return result;
}

//============================================================================
bool FileInfoXferMgr::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	if( false == m_bIsInSession )
	{
		m_bIsInSession = true;
	}

	return true;
}

//============================================================================
void FileInfoXferMgr::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	if( true == m_bIsInSession )
	{
		m_bIsInSession = false;
	}
}

//============================================================================
bool FileInfoXferMgr::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_bIsInSession;
}

//============================================================================
void FileInfoXferMgr::fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings )
{
	fileShareSettings = m_FileShareSettings;
}

//============================================================================
void FileInfoXferMgr::fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings )
{
	bool wasInSession = fromGuiIsPluginInSession();
	if( wasInSession )
	{
		fromGuiStopPluginSession();
	}

	m_FileShareSettings = fileShareSettings;
	m_FileShareSettings.saveSettings( m_Engine.getEngineSettings() );
	if( wasInSession )
	{
		fromGuiStartPluginSession();
	}
}

//============================================================================
void FileInfoXferMgr::fileAboutToBeDeleted( std::string& fileName )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	if( fileName.empty() )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::fileAboutToBeDeleted empty file name" );
		return;
	}

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	FileTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* xferSession = ( *iter );
		if( xferSession->getXferInfo().getLclFileName() == fileName )
		{
			sendFileXferCancel( xferSession );
			xferSession->cancelUpload( xferSession->getXferInfo().getLclSessionId() );
			delete xferSession;
			m_TxSessions.erase( iter );
			return;
		}
	}
}

//============================================================================
void FileInfoXferMgr::fromGuiCancelDownload( VxGUID& lclSessionId )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	auto iter = m_RxSessions.find( lclSessionId );
	if( iter != m_RxSessions.end() )
	{
		FileRxSession* xferSession = iter->second;
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			sendFileXferCancel( xferSession );
			xferSession->cancelDownload( lclSessionId );
			delete xferSession;
			m_RxSessions.erase( iter );
		}
	}
}

//============================================================================
void FileInfoXferMgr::fromGuiCancelUpload( VxGUID& lclSessionId )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* xferSession = ( *iter );
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			sendFileXferCancel( xferSession );
			xferSession->cancelUpload( lclSessionId );
			delete xferSession;
			m_TxSessions.erase( iter );
			return;
		}
	}
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool FileInfoXferMgr::fromGuiMakePluginOffer( VxGUID& onlineId,	OfferBaseInfo& offerInfo )
{
	VxGUID& lclSessionId = offerInfo.getOfferId();
	if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s lcl & rmt session id %s", __func__, lclSessionId.toHexString().c_str() );

	lclSessionId.assureIsValidGUID();
	std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
	if( sktBase && sktBase->isConnected() )
	{
		m_Engine.getOfferMgr().addOffer( offerInfo );
		PktPluginOfferReq pktReq;
		pktReq.setLclSessionId( lclSessionId );
		pktReq.setRmtSessionId( lclSessionId );
		pktReq.setPluginType( m_Plugin.getPluginType() );
		offerInfo.addToBlob( pktReq.getBlobEntry() );
		pktReq.calcPktLen();
		if( true == m_PluginMgr.pluginApiTxPacket(	m_Plugin.getPluginType(), 
			onlineId, 
			sktBase, 
			&pktReq ) )
		{
			// create xfer session now. must exist before reply to assure we actually created the offer replied to
			FileInfo fileInfo( offerInfo );

			FileTxSession* xferSession = findOrCreateTxSession( lclSessionId, onlineId, sktBase );

			xferSession->m_FilesToXferList.emplace_back( FileToXfer( fileInfo, lclSessionId, lclSessionId ) );

			return true;
		}
	}

	return false;
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool FileInfoXferMgr::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	VxGUID& lclSessionId = offerInfo.getOfferId();
	lclSessionId.assureIsValidGUID();
	std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
	if( sktBase && sktBase->isConnected() )
	{
		PktPluginOfferReply pktReq;
		pktReq.setLclSessionId( lclSessionId );
		pktReq.setRmtSessionId( lclSessionId );
		pktReq.setPluginType( m_Plugin.getPluginType() );
		offerInfo.addToBlob( pktReq.getBlobEntry() );
		pktReq.calcPktLen();
		if( true == m_PluginMgr.pluginApiTxPacket(	m_Plugin.getPluginType(), 
			onlineId, 
			sktBase, 
			&pktReq ) )
		{
			if( offerInfo.getOfferResponse() == eOfferResponseAccept )
			{ 
				FileInfo fileInfo( offerInfo );

				FileRxSession* xferSession = findOrCreateRxSession( lclSessionId, onlineId, sktBase );

				xferSession->m_FilesToXferList.emplace_back( FileToXfer( fileInfo, lclSessionId, lclSessionId ) );
			}

			return true;
		}
	}

	return false;
}

//============================================================================
EXferError FileInfoXferMgr::fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	if( eXferActionDownload == xferAction )
	{
		if( isFileDownloading( fileInfo.getFileHashId() ) )
		{
			return eXferErrorAlreadyDownloading;
		}

		//if( isFileInDownloadFolder( pAction )
		//	|| m_FileLibraryMgr.isFileInLibrary( fileHashId ) )
		//{
		//	return eXferErrorAlreadyDownloaded;
		//}	

		std::shared_ptr<VxSktBase> sktBase = m_Engine.getConnectIdListMgr().findBestUserOnlineConnection( onlineId );
		if( sktBase && sktBase->isConnected() )
		{
			VxGUID& sessionId = fileInfo.getXferSessionId();
			FileRxSession* xferSession = findOrCreateRxSession( sessionId, onlineId, sktBase );
			
			xferSession->m_FilesToXferList.emplace_back( FileToXfer( fileInfo, sessionId, sessionId ) );
			return beginFileGet( xferSession );
		}
		else
		{
			return eXferErrorDisconnected;
		}
	}

	return eXferErrorBadParam;
}

//============================================================================
void FileInfoXferMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	bool erasedSession = true;
	FileTxIter iter;
	while( erasedSession )
	{
		erasedSession = false;
		for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
		{
			FileTxSession* xferSession = ( *iter );
			if( xferSession->getSkt() == sktBase )
			{
				delete xferSession;
				m_TxSessions.erase( iter );
				erasedSession = true;
				break;
			}
		}
	}

	erasedSession = true;
	FileRxIter oRxIter; 
	while( erasedSession )
	{
		erasedSession = false;
		for( oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
		{
			FileRxSession* xferSession = oRxIter->second;
			if( xferSession->getSkt() == sktBase )
			{
				delete xferSession;
				m_RxSessions.erase( oRxIter );
				erasedSession = true;
				break;
			}
		}
	}
}

//============================================================================
void FileInfoXferMgr::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktPluginOfferReq* pktReq = (PktPluginOfferReq *)pktHdr;
	OfferBaseInfo offerInfo;
	offerInfo.extractFromBlob( pktReq->getBlobEntry() );
	if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s offer id %s", __func__, offerInfo.getOfferId().toHexString().c_str() );
	m_Engine.getOfferMgr().addOffer( offerInfo );
}

//============================================================================
//! packet with remote users reply to offer
void FileInfoXferMgr::onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktPluginOfferReply* pktReply = (PktPluginOfferReply*)pktHdr;
	OfferBaseInfo offerInfo;
	offerInfo.extractFromBlob( pktReply->getBlobEntry() );

	if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s lcl session %s rmt session %s offer id %s", __func__,
		pktReply->getRmtSessionId().toHexString().c_str(), pktReply->getLclSessionId().toHexString().c_str(), offerInfo.getOfferId().toHexString().c_str() );


	if( m_Engine.getOfferMgr().hostedOfferExists( offerInfo, true ) )
	{
		m_FileInfoMgr.toGuiRxedOfferReply( pktReply->getSrcOnlineId(), m_Plugin.getPluginType(), offerInfo, offerInfo.getOfferId(), pktReply->getOfferResponse() );

		PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

		FileTxSession* xferSession = findTxSessionSessionId( pktReply->getRmtSessionId() );
		if( xferSession )
		{
			xferSession->offerAccepted( offerInfo.getOfferId() );
			VxFileXferInfo& xferInfo = xferSession->getXferInfo();
			std::string fileName = xferInfo.getLclFileName();
			if( !fileName.empty() )
			{	
				if( eOfferResponseAccept == offerInfo.getOfferResponse() )
				{
					//xferSession->m_FilesToXferList.emplace_back( FileToXfer( fileInfo, xferSession->getLclSessionId(), xferSession->getRmtSessionId() ) );
					EXferError xferErr = beginFileSend( xferSession );
					if( eXferErrorNone != xferErr )
					{
						m_FileInfoMgr.updateToGuiFileXferState( xferSession->getLclSessionId(), eXferDirectionTx, eXferStateUploadError, xferErr );
					}
				}
				else
				{
					LogMsg( LOG_ERROR, "FileInfoXferMgr::%s %s offer rejected %s", __func__, DescribePluginType( getPluginType() ), fileName.c_str() );
					endFileXferSession( xferSession );
				}
			}
			else
			{
				LogMsg( LOG_ERROR, "FileInfoXferMgr::%s %s empty file name", __func__, DescribePluginType( getPluginType() ) );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "FileInfoXferMgr::%s session %s not found %s", __func__, pktReply->getRmtSessionId().toHexString().c_str(), DescribePluginType( getPluginType() ) );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s hosted offer does not exist %s", __func__, DescribePluginType( getPluginType() ) );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PktFileGetReq* pktReq = (PktFileGetReq *)pktHdr;
	PktFileGetReply pktReply;
	pktReply.setIsStream( pktReq->getIsStream() );

	VxGUID assetId = pktReq->getAssetId();

	std::string strLclFileNameAndPath;
	std::string rmtFileName;
	pktReq->getFileName( rmtFileName );

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s start %s rxing cnt %d file %s asset %s", __func__, 
			DescribePluginType( getPluginType() ), 
			m_RxSessions.size(), 
			rmtFileName.c_str(),
			assetId.toHexString().c_str() );

	if( false == m_FileInfoMgr.getFileFullName( assetId, pktReq->getFileHashId(), strLclFileNameAndPath ) )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s file no longer shared %s", __func__, rmtFileName.c_str() );
		pktReply.setError( eXferErrorFileNotFound );
	}
	else
	{
		pktReply.setError( canTxFile( pktReq->getSrcOnlineId(), assetId, pktReq->getFileHashId() ) );
	}

	if( eXferErrorNone == pktReply.getError() )
	{
		FileTxSession* xferSession = createTxSession( pktReq->getSrcOnlineId(), sktBase);
		xferSession->setIsStream( pktReq->getIsStream() );

		VxFileXferInfo& xferInfo = xferSession->getXferInfo();
		xferInfo.setIsStream( pktReq->getIsStream() );
		xferInfo.setRmtSessionId( pktReq->getLclSessionId() );
		xferInfo.setAssetId( assetId );
		xferInfo.setFileHashId( pktReq->getFileHashId() );
		xferInfo.setFileOffset( pktReq->getStartOffset() );
		xferInfo.setLclFileName( rmtFileName.c_str() );
		xferInfo.setLclFileNameAndPath( strLclFileNameAndPath.c_str() );
		xferInfo.setRmtFileName(  rmtFileName.c_str() );
		
		addTxSession( xferSession );

		pktReply.setLclSessionId( xferInfo.getLclSessionId() );
		pktReply.setRmtSessionId( xferInfo.getRmtSessionId() );

		pktReply.setFileName( xferInfo.getRmtFileName() );
		pktReply.setAssetId( assetId );
		pktReply.setFileHashId( xferInfo.getFileHashId() );
		pktReply.setStartOffset( pktReq->getStartOffset() );
		pktReply.setIsStream( pktReq->getIsStream() );

		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "%s %s pktReply txing file %s lcl session %s rmt session %s", __func__, 
			DescribePluginType( getPluginType() ), 
			rmtFileName.c_str(),
			xferInfo.getLclSessionId().toHexString().c_str(), xferInfo.getRmtSessionId().toHexString().c_str() );

		EXferError xferErr  = ( m_Plugin.txPacket( pktHdr->getSrcOnlineId(), sktBase, &pktReply)) ? eXferErrorNone : eXferErrorDisconnected;
		if( eXferErrorNone == xferErr )
		{
			xferErr = beginFileSend( xferSession );
		}

		if( eXferErrorNone != xferErr )
		{
			m_FileInfoMgr.updateToGuiFileXferState( pktReq->getLclSessionId(), eXferDirectionTx, eXferStateUploadError, xferErr );
			endFileXferSession( xferSession );
		}
		else if( !xferInfo.isStream() )
		{
			m_FileInfoMgr.updateToGuiFileXferState( pktReq->getLclSessionId(), eXferDirectionTx, eXferStateBeginUpload, eXferErrorNone );
			m_FileInfoMgr.updateToGuiFileXferState( pktReq->getLclSessionId(), eXferDirectionTx, eXferStateInUploadXfer, eXferErrorNone );
		}
	}
	else
	{
		m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply);
	}

	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s plugin %s rxing %zu", __func__, DescribePluginType( getPluginType() ), m_RxSessions.size() );
}

//============================================================================
void FileInfoXferMgr::onPktFileGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s %s rxing %zu", __func__, DescribePluginType( getPluginType() ), m_RxSessions.size() );
	announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFileSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileSendReq* poPkt = (PktFileSendReq *)pktHdr;
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s %s rxing %zu", __func__, DescribePluginType( getPluginType() ), m_RxSessions.size() );
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	PktFileSendReply pktReply;
	pktReply.setIsStream( poPkt->getIsStream() );
	FileRxSession* xferSession = findRxSessionSessionId( poPkt->getRmtSessionId() );
	if( xferSession )
	{
		xferSession->setRmtSessionId( poPkt->getLclSessionId() );
		if( !poPkt->getIsStream() )
		{
			EXferError xferErr = beginFileReceive( xferSession, poPkt );
			if( eXferErrorNone != xferErr )
			{
				m_FileInfoMgr.updateToGuiFileXferState( xferSession->getLclSessionId(), eXferDirectionRx, eXferStateDownloadError, xferErr );
				endFileXferSession( xferSession );
			}
			else
			{
				m_FileInfoMgr.updateToGuiFileXferState(  xferSession->getLclSessionId(), eXferDirectionRx, eXferStateBeginDownload, eXferErrorNone );
				m_FileInfoMgr.updateToGuiFileXferState(  xferSession->getLclSessionId(), eXferDirectionRx, eXferStateInDownloadXfer, eXferErrorNone );
			}

			pktReply.setError( xferErr );
		}

		m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply);
	}
	else
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s: Could not find session", __func__ );
		pktReply.setError( eXferErrorBadParam );
		m_Plugin.txPacket( poPkt->getSrcOnlineId(), sktBase, &pktReply );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PktFileSendReply* poPkt = ( PktFileSendReply* )pktHdr;
	announcePkt( sktBase, poPkt );

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s %s txing %zu", __func__, DescribePluginType( getPluginType() ), m_TxSessions.size() );

	FileTxSession* xferSession = findTxSessionSessionId( poPkt->getRmtSessionId() );
	if( xferSession )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s found tx session", __func__ );
	}
	else
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s failed to find tx session %s", __func__, poPkt->getRmtSessionId().toHexString().c_str() );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileChunkReq* pktReq = (PktFileChunkReq *)pktHdr;
	if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s %" PRIu64, __func__, pktReq->getChunkOffset() );
	announcePkt( sktBase, pktReq );

	if( pktReq->getIsStream() )
	{
		PktFileChunkReply pktReply;
		pktReply.setIsStream( pktReq->getIsStream() );
		pktReply.setDataLen( pktReq->getDataLen() );
		pktReply.setChunkOffset( pktReq->getChunkOffset() );
		pktReply.setLclSessionId( pktReq->getRmtSessionId() );
		pktReply.setRmtSessionId( pktReq->getLclSessionId() );
		pktReply.setAssetId( pktReq->getAssetId() );

		m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply );
		return;
	}

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	FileRxSession* xferSession = findRxSessionSessionId( pktReq->getRmtSessionId() );
	if( xferSession )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::onPktFileChunkReq %s rx cnt %d", 
			DescribePluginType( getPluginType() ), m_RxSessions.size() );
		EXferError xferErr = rxFileChunk( xferSession, pktReq );
		if( eXferErrorNone != xferErr )
		{
			m_FileInfoMgr.updateToGuiFileXferState( xferSession->getLclSessionId(), eXferDirectionRx, eXferStateDownloadError, xferErr );
			endFileXferSession( xferSession );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "%s failed to find rx session", __func__, pktReq->getRmtSessionId().toHexString().c_str()  );
		PktFileChunkReply pktReply;
		pktReply.setIsStream( pktReq->getIsStream() );
		pktReply.setDataLen(0);
		pktReply.setError( eXferErrorBadParam );
		m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileChunkReply* poPkt = (PktFileChunkReply*)pktHdr;
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s %" PRIu64, __func__, poPkt->getChunkOffset() );

	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
    
	FileTxSession* xferSession = findTxSessionSessionId( poPkt->getRmtSessionId() );
	if( xferSession )
	{
		EXferError xferErr = txNextFileChunk( xferSession );
		if( eXferErrorNone != xferErr )
		{
			m_FileInfoMgr.updateToGuiFileXferState( xferSession->getLclSessionId(), eXferDirectionTx, eXferStateUploadError, xferErr );
			endFileXferSession( xferSession );
		}
	}
	else
	{
		if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "%s failed to find tx session %s", __func__, poPkt->getRmtSessionId().toHexString().c_str() );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    announcePkt( sktBase, pktHdr );
	PktFileGetCompleteReq* poPkt = (PktFileGetCompleteReq *)pktHdr;
	if( !poPkt->getIsStream() )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s session %s", __func__, poPkt->getRmtSessionId().toHexString().c_str() );
		PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );	
		FileRxSession* xferSession = findRxSessionSessionId( poPkt->getRmtSessionId() );
		if( xferSession )
		{
			finishFileReceive( xferSession, poPkt );
		}
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFileSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PktFileSendCompleteReq* pktReq = (PktFileSendCompleteReq *)pktHdr;
	if( pktReq->getIsStream() )
	{
		announcePkt( sktBase, pktHdr );
		return;
	}
    
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	
	FileRxSession* xferSession = findRxSessionSessionId( pktReq->getRmtSessionId() );
	//TODO check checksum
	if( xferSession )
	{
		finishFileReceive( xferSession, pktReq );
	}
}

//============================================================================
void FileInfoXferMgr::onPktFileSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PktFileXferCancel* pktXferCancel = (PktFileXferCancel*)pktHdr;
	VxGUID lclSessionId = pktXferCancel->getRmtSessionId();
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );

	for( auto iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		FileRxSession* xferSession = ( *iter ).second;
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			sendFileXferCancel( sktBase, xferSession  );
			xferSession->cancelDownload( lclSessionId );
			delete xferSession;
			m_RxSessions.erase( iter );
			return;
		}
	}

	for( auto iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* xferSession = ( *iter );
		if( xferSession->getLclSessionId() == lclSessionId )
		{
			sendFileXferCancel( sktBase, xferSession );
			xferSession->cancelUpload( lclSessionId );
			delete xferSession;
			m_TxSessions.erase( iter );
			return;
		}
	}

	announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFindFileReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
}

//============================================================================
void FileInfoXferMgr::onPktFindFileReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
}

//============================================================================
void FileInfoXferMgr::onPktFileListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	/*
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	PktFileListReq* poPkt = (PktFileListReq *)pktHdr;
	uint32_t reqListIdx = poPkt->getListIndex();
	m_FileInfoMgr.lockFileListPackets();
	std::vector<PktFileListReply*>& pktList = m_FileInfoMgr.getFileListPackets();
	if( reqListIdx >= pktList.size() )
	{
		PktFileListReply pktReply;
		if( 0 == pktList.size() )
		{
			pktReply.setError( ERR_NO_SHARED_FILES );
		}
		else
		{
			pktReply.setError( ERR_FILE_LIST_IDX_OUT_OF_RANGE );
		}

		m_Plugin.txPacket( netIdent, sktBase, &pktReply );	
	}
	else
	{
		m_Plugin.txPacket( netIdent, sktBase, pktList[reqListIdx] );	
	}

	m_FileInfoMgr.unlockFileListPackets();
	*/
}

//============================================================================
void FileInfoXferMgr::onPktFileListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	/*
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	FileRxSession* xferSession = findOrCreateRxSession( netIdent, sktBase );
	PktFileListReply * poPkt = (PktFileListReply *)pktHdr;
	int32_t rc = poPkt->getError();
	if( 0 != rc )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::onPktFileListReply error %d\n", rc );
		m_FileInfoMgr.toGuiFileListReply( netIdent, m_Plugin.getPluginType(), 0, 0, "", VxGUID::nullVxGUID(), 0 );
		return;
	}

	if( !poPkt->getIsListCompleted() )
	{
		// request next in list
		PktFileListReq pktReq;
		pktReq.setListIndex( poPkt->getListIndex() + 1 );
		m_Plugin.txPacket( netIdent, sktBase, &pktReq );
	}

	std::vector<VxFileInfo> fileList;
	poPkt->getFileList( fileList );
	std::vector<VxFileInfo>::iterator iter;
	for( iter = fileList.begin(); iter != fileList.end(); ++iter )
	{
		VxFileInfo& fileInfo = (*iter);
		m_FileInfoMgr.toGuiFileListReply( netIdent, 
									m_Plugin.getPluginType(), 
									fileInfo.getFileType(), 
									fileInfo.getFileLength(),
									fileInfo.getFileName().c_str(),
									VxGUID::nullVxGUID(),
									fileInfo.getFileHashId().getHashData() );
	}

	if( poPkt->getIsListCompleted() 
		|| ( 0 == poPkt->getFileCount() ) )
	{
		m_FileInfoMgr.toGuiFileListReply( netIdent, m_Plugin.getPluginType(), 0, 0, "", VxGUID::nullVxGUID(), 0 );
		endFileXferSession( xferSession );
	}
	*/
}

//============================================================================
void FileInfoXferMgr::onPktFileInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFileInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktFileInfoErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::onPktStreamCtrlReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    
	PktStreamCtrlReq* pktReq = (PktStreamCtrlReq*)pktHdr;

	PktStreamCtrlReply pktReply;
	pktReply.setStreamCtrl( pktReq->getStreamCtrl() );
	pktReply.setLclSessionId( pktReq->getRmtSessionId() );
	pktReply.setRmtSessionId( pktReq->getLclSessionId() );
	pktReply.setAssetId( pktReq->getAssetId() );

	EXferError xferErr{ eXferErrorFileNotFound };
	if( pktReq->getStreamCtrl() == eStreamSeek )
	{
		FileTxSession* xferSession = findTxSessionSessionId( pktReq->getRmtSessionId() );
		if( xferSession )
		{
			VxFileXferInfo& xferInfo = xferSession->getXferInfo();
			if( xferInfo.m_hFile )
			{
				int64_t seekPos = pktReq->getStartOffset();
				if( seekPos >= 0 && seekPos < xferInfo.getFileLength() )
				{
					if( 0 == VFileSeek( xferInfo.m_hFile, seekPos, SEEK_SET ) )
					{
						xferInfo.setFileOffset( seekPos );
						xferErr = eXferErrorNone;
					}
					else
					{
						LogMsg( LOG_ERROR, "FileInfoXferMgr::%s fseek error %d pos %" PRId64 " file %s", __func__,
							VxGetLastError(), seekPos, xferInfo.getLclFileNameAndPath().c_str() );
					}
				}
				else
				{
					LogMsg( LOG_ERROR, "FileInfoXferMgr::%s invalid seek pos %" PRId64 " file %s", __func__,
							seekPos, xferInfo.getLclFileNameAndPath().c_str() );
				}
			}
			else
			{
				LogMsg( LOG_ERROR, "FileInfoXferMgr::%s file has been closed", __func__ );
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "FileInfoXferMgr::%s failed to find tx session", __func__ );
		}
	}

	m_Plugin.txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply );
}
	
//============================================================================
void FileInfoXferMgr::onPktStreamCtrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
    announcePkt( sktBase, pktHdr );
}

//============================================================================
void FileInfoXferMgr::endFileXferSession( FileRxSession* poSessionIn )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s end rx session lcl %s rmt %s", __func__,
			   poSessionIn->getLclSessionId().toHexString().c_str(), poSessionIn->getRmtSessionId().toHexString().c_str() );
	VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
    if( xferInfo.useFileIo() )
    {
        if( xferInfo.m_hFile )
        {
            VFileClose( xferInfo.m_hFile );
            xferInfo.m_hFile = NULL;
        }

        if( getMoveCompletedFilesToDownloadFolder() )
        {
            // file was moved to completed folder.. remove the temporary download file
            std::string fileName = xferInfo.getDownloadIncompleteFileName();
            if( fileName.length() )
            {
                VxFileUtil::deleteFile( fileName.c_str() );
            }
        }
    }

	FileRxIter oRxIter = m_RxSessions.begin();

	while( oRxIter != m_RxSessions.end() )
	{
		FileRxSession* xferSession = oRxIter->second;
		if( poSessionIn == xferSession )
		{
			oRxIter = m_RxSessions.erase( oRxIter );
			delete xferSession;
		}
		else
		{
			++oRxIter;
		}
	}
}

//============================================================================
void FileInfoXferMgr::endFileXferSession( FileTxSession* poSessionIn )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s end tx session lcl %s rmt %s", __func__,
			poSessionIn->getLclSessionId().toHexString().c_str(), poSessionIn->getRmtSessionId().toHexString().c_str() );

	VxFileXferInfo& xferInfo = poSessionIn->getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
	}

	FileTxIter iter = m_TxSessions.begin();
	while( iter != m_TxSessions.end() )
	{
		FileTxSession* xferSession = (*iter);
		if( xferSession == poSessionIn )
		{
			iter = m_TxSessions.erase( iter );
			delete xferSession;
			break;
		}
		else
		{
			++iter;
		}
	}
}

//============================================================================
FileRxSession* FileInfoXferMgr::findRxSessionSendToId( VxGUID& sendToId )
{
	FileRxIter iter;
	for( iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		FileRxSession* xferSession = iter->second;
		if( xferSession->getSendToId() == sendToId )
		{
			return  xferSession;
		}
	}

	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s session not found %s", __func__, sendToId.toHexString().c_str() );
	return nullptr;
}

//============================================================================
FileRxSession* FileInfoXferMgr::findRxSessionSessionId( VxGUID& lclSessionId )
{
	FileRxIter iter = m_RxSessions.find( lclSessionId );
	if( iter != m_RxSessions.end() )
	{
		return iter->second;
	}

	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s session not found %s", __func__, lclSessionId.toHexString().c_str() );
	return nullptr;
}

//============================================================================
FileRxSession*	FileInfoXferMgr::findOrCreateRxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	FileRxSession* xferSession = findRxSessionSendToId( sendToId );
	if( !xferSession )
	{
		xferSession = new FileRxSession( sktBase, sendToId );
		m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
		if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s rx session %s sendToId %s", __func__, 
			xferSession->getLclSessionId().toHexString().c_str(), sendToId.toHexString().c_str() );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	return xferSession;
}

//============================================================================
FileRxSession* FileInfoXferMgr::findOrCreateRxSession( VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	FileRxSession* xferSession = findRxSessionSessionId( lclSessionId );
	if( ( xferSession ) && ( lclSessionId.isValid() ) )
	{
		xferSession = new FileRxSession( lclSessionId, sktBase, sendToId );
		m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
	}

	if( !xferSession && !lclSessionId.isValid() )
	{
		xferSession = findRxSessionSendToId( sendToId );
	}

	if( !xferSession )
	{
		xferSession = new FileRxSession( lclSessionId, sktBase, sendToId );
		m_RxSessions.insert( std::make_pair( xferSession->getLclSessionId(), xferSession ) );
		LogMsg( LOG_VERBOSE, "FileInfoXferMgr::%s %s rx cnt %d session id %s sendToId %s", __func__, 
			DescribePluginType( getPluginType() ), m_RxSessions.size(), lclSessionId.toHexString().c_str(), sendToId.toHexString().c_str() );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	return xferSession;
}

//============================================================================
FileTxSession* FileInfoXferMgr::findTxSessionSendToId( VxGUID& sendToId )
{
	FileTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* txSession = ( *iter );
		if( txSession->getSendToId() == sendToId )
		{
			return txSession;
		}
	}

	return NULL;
}

//============================================================================
FileTxSession* FileInfoXferMgr::findTxSessionSessionId( VxGUID& lclSessionId )
{
	FileTxIter iter;
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* txSession = ( *iter );
		if( txSession->getLclSessionId() == lclSessionId )
		{
			return txSession;
		}
	}

	return NULL;
}

//============================================================================
FileTxSession*	FileInfoXferMgr::createTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	return new FileTxSession( sktBase, sendToId );
}

//============================================================================
FileTxSession*	FileInfoXferMgr::findOrCreateTxSession( VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	FileTxSession* xferSession = findTxSessionSendToId( sendToId );
	if( NULL == xferSession )
	{
		xferSession = createTxSession( sendToId, sktBase );
		addTxSession( xferSession );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	return xferSession;
}

//============================================================================
FileTxSession* FileInfoXferMgr::findOrCreateTxSession( VxGUID& lclSessionId, VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase )
{
	FileTxSession* xferSession = 0;
	if( lclSessionId.isValid() )
	{
		xferSession = findTxSessionSessionId( lclSessionId );
	}

	if( 0 == xferSession )
	{
		xferSession = findTxSessionSendToId( sendToId );
	}

	if( NULL == xferSession )
	{
		xferSession = new FileTxSession( lclSessionId, sktBase, sendToId );
		addTxSession( xferSession );
	}
	else
	{
		xferSession->setSkt( sktBase );
	}

	return xferSession;
}

//============================================================================
EXferError FileInfoXferMgr::beginFileSend( FileTxSession* xferSession )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	PktFileSendReq oPktReq;
	oPktReq.setIsStream( xferSession->isStream() );

	EXferError xferErr = eXferErrorNone;
	xferSession->setErrorCode( 0 );
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( xferInfo.m_hFile )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::beginFileSend: ERROR: File transfer still in progress" );
		xferErr = eXferErrorAlreadyUploading;
	}

	xferInfo.setXferDirection( eXferDirectionTx );
	if( eXferErrorNone == xferErr )
	{
		xferInfo.m_u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str() );
		if( 0 == xferInfo.m_u64FileLen )
		{
			// no file found to send
			LogMsg( LOG_VERBOSE, "FileInfoXferMgr::beginFileSend: File %s not found to send", xferInfo.getLclFileNameAndPath().c_str() );
			xferErr = eXferErrorFileNotFound;
		}
		else if( false == xferInfo.getFileHashId().isHashValid() )
		{
			// see if we can get hash from shared files
			if( !m_FileInfoMgr.getFileHashId( xferInfo.getLclFileNameAndPath(), xferInfo.getFileHashId() ) )
			{
				// TODO.. que for hash
			}
		}
	}

	if( eXferErrorNone == xferErr )
	{
		xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "rb" ); 
		if( NULL == xferInfo.m_hFile )
		{
			// open file failed
			xferInfo.m_hFile = NULL;
			LogMsg( LOG_VERBOSE, "FileInfoXferMgr::beginFileSend: Could not open File %s", xferInfo.getLclFileNameAndPath().c_str() );
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
				LogMsg( LOG_VERBOSE, "FileInfoXferMgr::beginFileSend: File %s could not be resumed because too short", 
					xferInfo.getLclFileNameAndPath().c_str() );
				xferErr = eXferErrorFileSeekError;
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
					LogMsg( LOG_VERBOSE, "FileInfoXferMgr::beginFileSend: could not seek to position %d in file %s",
						xferInfo.m_u64FileOffs,
						xferInfo.getLclFileNameAndPath().c_str() );
					xferErr = eXferErrorFileSeekError;
				}
			}
		}
	}

	oPktReq.setError( xferErr );
	//get file extension
	std::string	strExt;
	VxFileUtil::getFileExtension( xferInfo.getLclFileName(), strExt );

	uint8_t u8FileType = VxFileUtil::fileExtensionToFileTypeFlag( strExt.c_str() );

	oPktReq.setFileType( u8FileType );
	oPktReq.setFileLen( xferInfo.m_u64FileLen );
	oPktReq.setLclSessionId( xferInfo.getLclSessionId() );
	oPktReq.setRmtSessionId( xferInfo.getRmtSessionId() );
	oPktReq.setAssetId( xferInfo.getAssetId() );
	oPktReq.setFileName( xferInfo.getRmtFileName().c_str() );
	oPktReq.setFileOffset( xferInfo.getFileOffset() );
	oPktReq.setFileHashId( xferInfo.getFileHashId() );

	if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &oPktReq ) )
	{
		if( eXferErrorNone == xferErr )
		{
			xferErr = eXferErrorDisconnected;
		}
	}

	LogMsg( LOG_VERBOSE, "FileInfoXferMgr::beginFileSend: start sending file %s to %s", 
						xferInfo.getLclFileName().c_str(),
           m_Engine.describeUser( xferSession->getSendToId() ).c_str() );

	if( eXferErrorNone == xferErr )
	{
		if( oPktReq.getIsStream() && xferInfo.m_hFile && eXferErrorNone == xferErr && xferInfo.getFileOffset() == 0 )
		{
			PktStreamCtrlReply pktReply;
			pktReply.setStreamCtrl( eStreamReadTail );
			pktReply.setLclSessionId( xferInfo.getLclSessionId() );
			pktReply.setRmtSessionId( xferInfo.getRmtSessionId() );
			pktReply.setAssetId( xferInfo.getAssetId() );
			pktReply.setFileHashId( xferInfo.getFileHashId() );

			int64_t origOffs = xferInfo.getFileOffset();
			int64_t readOffs = xferInfo.getFileLength() - PKT_TYPE_FILE_MAX_DATA_LEN;
			bool readData{ false };
			if( 0 == VFileSeek64( xferInfo.m_hFile, readOffs ) )
			{
				pktReply.setStartOffset( readOffs );
				pktReply.setEndOffset( xferInfo.getFileLength() );

				if( PKT_TYPE_FILE_MAX_DATA_LEN == VFileRead( pktReply.getDataBuf(), 1, PKT_TYPE_FILE_MAX_DATA_LEN, xferInfo.m_hFile ) )
				{
					pktReply.setDataLen( PKT_TYPE_FILE_MAX_DATA_LEN );
					readData = true;
				}

				if( 0 != VFileSeek64( xferInfo.m_hFile, origOffs ) )
				{
					LogMsg( LOG_ERROR, "FileInfoXferMgr::%s could not seek back to origin" );

				}
			}

			if( readData )
			{
				if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply ) )
				{
					if( eXferErrorNone == xferErr )
					{
						xferErr = eXferErrorDisconnected;
					}
				}
			}
		}

		if( eXferErrorNone == xferErr )
		{
			FileInfo fileInfo( xferInfo, xferSession->getSendToId() );
			m_FileInfoMgr.toGuiFileUploadStart( xferSession->getSendToId(), xferInfo.getLclSessionId(), fileInfo );

			// file is open and setup so send first chunk of data
			if( xferSession->isStream() )
			{
				VxGUID streamSessionId = xferSession->getLclSessionId();
				for( int i = 0; i < MAX_OUTSTANDING_STREAM_PKTS; i++ )
				{
					FileTxSession* streamSession = findTxSessionSessionId( streamSessionId );
					if( !streamSession )
					{
						// The stream can complete and delete its tx session in txNextFileChunk for short files.
						break;
					}

					EXferError xferError = txNextFileChunk( streamSession );
					if( xferError != eXferErrorNone )
					{
						LogMsg( LOG_ERROR, "FileInfoXferMgr::%s error %s txNextFileChunk multiple", __func__, DescribeXferError( xferError ) );
						return xferError;
					}
				}

				return eXferErrorNone;
			}
			else
			{
				return txNextFileChunk( xferSession );
			}
		}
		else
		{
			endFileXferSession( xferSession );
			return xferErr;
		}
	}
	else
	{
		endFileXferSession( xferSession );
		return xferErr;
	}
}

//============================================================================
EXferError FileInfoXferMgr::beginFileReceive( FileRxSession* rxSession, PktFileSendReq* poPkt )
{
	if( NULL == rxSession )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s bad param", __func__ );
		return eXferErrorBadParam;
	}

	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	EXferError xferErr = eXferErrorNone;
	VxFileXferInfo& xferInfo = rxSession->getXferInfo();
	xferInfo.setRmtSessionId( poPkt->getLclSessionId() );
	xferInfo.setAssetId( poPkt->getAssetId() );
	xferInfo.setFileHashId( poPkt->getFileHashId() );
	xferInfo.setRmtFileName( poPkt->getFileName() );
	xferInfo.setLclFileName( poPkt->getFileName() );
	xferInfo.setFileLength( poPkt->getFileLen() );
	xferInfo.setFileOffset( poPkt->getFileOffset() );
	xferInfo.setIsStream( rxSession->isStream() );

	if( poPkt->getError() )
	{
		xferErr = (EXferError)poPkt->getError();
		m_FileInfoMgr.updateToGuiFileXferState(  poPkt->getRmtSessionId(), eXferDirectionRx, eXferStateDownloadError, xferErr, xferErr );
		endFileXferSession( rxSession );
		return xferErr;
	}

	xferErr = setupFileDownload( xferInfo, rxSession->getSendToId() );

	if( eXferErrorNone == xferErr )
	{
		LogMsg( LOG_VERBOSE, "FileInfoXferMgr::(File Send) start recieving file %s", xferInfo.getLclFileNameAndPath().c_str() );

        // uint8_t u8FileType = VxFileUtil::fileExtensionToFileTypeFlag( xferInfo.getRmtFileName().c_str() );
		FileInfo fileInfo( xferInfo, rxSession->getSendToId() );
		m_FileInfoMgr.toGuiFileDownloadStart( rxSession->getSendToId(), xferInfo.getLclSessionId(), fileInfo );
	}

	// don't send reply.. will get file chunk next anyway
	return xferErr;
}

//============================================================================
EXferError FileInfoXferMgr::txNextFileChunk( FileTxSession* xferSession )
{
	if( NULL == xferSession )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s bad param", __func__ );
		return eXferErrorBadParam;
	}

	EXferError xferErr = eXferErrorNone;
	// fill the packet with data from the file
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
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
		// move file from to be sent to sent
		//xferSession->m_astrFilesSent.emplace_back( xferSession->m_FilesToXferList[0] );
		//xferSession->m_FilesToXferList.erase( xferSession->m_FilesToXferList.begin() );
		//xferSession->m_FilesToXferList.erase( xferSession->m_FilesToXferList.begin() );

		PktFileSendCompleteReq oPkt;
		oPkt.setIsStream( xferInfo.isStream() );
		oPkt.setLclSessionId( xferSession->getLclSessionId() );
		oPkt.setRmtSessionId( xferSession->getRmtSessionId() );
		oPkt.setAssetId( xferSession->getAssetId() );
		oPkt.setFileHashId( xferSession->getFileHashId() );
		m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &oPkt );

		LogMsg( LOG_ERROR, "FileInfoXferMgr:: Done Sending file %s", xferInfo.getLclFileNameAndPath().c_str() );
		m_FileInfoMgr.toGuiFileUploadComplete( xferInfo.getLclSessionId(), xferInfo.getLclFileNameAndPath(), eXferErrorNone );
		onFileSent( xferSession, xferInfo.getLclFileNameAndPath(), eXferErrorNone );
		return eXferErrorNone;
	}

	xferErr = sendNextFileChunk( xferInfo, xferSession->getSendToId(), xferSession->getSkt() );
	return xferErr;
}

//============================================================================
EXferError FileInfoXferMgr::rxFileChunk( FileRxSession* xferSession, PktFileChunkReq* poPkt )
{
	if( nullptr == xferSession )
	{
		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s bad param", __func__ );
		return eXferErrorBadParam;
	}

	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );
	EXferError xferErr = eXferErrorNone;
	vx_assert( xferSession );
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	// we are receiving a file
	if( xferInfo.isOpened() )
	{
		uint32_t u32BytesWritten = poPkt->getChunkLen();
		if( xferInfo.useFileIo() )
		{
			//write the chunk of data out to the file
			u32BytesWritten = (uint32_t)VFileWrite(	poPkt->m_au8FileChunk,
				1,
				poPkt->getChunkLen(),
				xferInfo.m_hFile );
		}

		if( u32BytesWritten != poPkt->getChunkLen() ) 
		{
			int32_t rc = VxGetLastError();
			xferSession->setErrorCode( rc );
			xferErr = eXferErrorFileWriteError;

			LogMsg( LOG_VERBOSE, "VxPktHandler::RxFileChunk: ERROR %d: writing to file %s",
							rc,
							xferInfo.getLclFileNameAndPath().c_str() );
		}
		else
		{
			//successfully write
			xferInfo.m_u64FileOffs += poPkt->getChunkLen();

			PktFileChunkReply pktReply;
			pktReply.setIsStream( poPkt->getIsStream() );
			pktReply.setDataLen( poPkt->getDataLen() );
			pktReply.setLclSessionId( xferInfo.getLclSessionId() );
			xferInfo.setRmtSessionId( poPkt->getLclSessionId() );
			pktReply.setRmtSessionId( poPkt->getLclSessionId() );
			pktReply.setAssetId( poPkt->getAssetId() );

			if( false == m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply ) )
			{
				xferErr = eXferErrorDisconnected;
			}
		}
	}

	if( eXferErrorNone == xferErr )
	{
		if( xferInfo.calcProgress() )
		{
			m_FileInfoMgr.updateToGuiFileXferState( xferInfo.getLclSessionId(), eXferDirectionRx, eXferStateInDownloadXfer, eXferErrorNone, xferInfo.getProgress() );
		}
	}
	else
	{
		m_FileInfoMgr.updateToGuiFileXferState( xferInfo.getLclSessionId(), eXferDirectionRx, eXferStateDownloadError, xferErr );
	}

	return xferErr;
}

//============================================================================
void FileInfoXferMgr::finishFileReceive( FileRxSession* xferSession, PktFileSendCompleteReq* pktReq )
{
	if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s", __func__ );

	if( !xferSession || !pktReq )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s null parameter", __func__ );
		return;
	}

	// done receiving file
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( xferInfo.m_hFile )
	{
		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
	}
	else
	{
		LogMsg( LOG_ERROR,  "FileInfoXferMgr::%s: NULL file handle", __func__ );
	}

	// let other act on the received file
	std::string fileName = xferInfo.getLclFileNameAndPath();
	if( !fileName.empty() )
	{
		//=== acknowlege ===//
		PktFileSendCompleteReply pktReply;
		pktReply.setIsStream( pktReq->getIsStream() );
		pktReply.setLclSessionId( xferInfo.getLclSessionId() );
		pktReply.setRmtSessionId( xferInfo.getRmtSessionId() );
		pktReply.setAssetId( xferSession->getAssetId() );
		pktReply.setFileHashId( xferSession->getFileHashId() );
		
		FileInfo fileInfo( xferInfo, xferSession->getSendToId() );
		xferSession->m_FilesXferedList.emplace_back( FileToXfer( fileInfo, xferInfo.getLclSessionId(), xferInfo.getRmtSessionId() ) );

		LogMsg( LOG_VERBOSE, "VxPktHandler: Done Receiving file %s", fileName.c_str() );

		m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply );
		xferSession->setErrorCode( pktReq->getError() );
		onFileReceived( xferSession, fileName, (EXferError)pktReq->getError() );
	}
	else
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s empty file name", __func__ );
	}
}

//============================================================================
void FileInfoXferMgr::onFileReceived( FileRxSession* xferSession, std::string& fileName, EXferError xferError )
{
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( !xferSession->isStream() )
	{
		if( eXferErrorNone == xferError )
		{
			std::string incompleteFile = xferInfo.getLclFileNameAndPath();
			if( getMoveCompletedFilesToDownloadFolder() )
			{
				std::string completedFile = xferInfo.getDownloadCompleteFileName();
				if( VxFileUtil::fileExists( completedFile.c_str() ) )
				{
					completedFile = VxFileUtil::makeUniqueFileName( completedFile.c_str() );
				}

				int32_t rc = 0;
				if( 0 == (rc = VxFileUtil::moveAFile( incompleteFile.c_str(), completedFile.c_str() )) )
				{
					m_FileInfoMgr.onFileDownloadComplete( xferSession->getSendToId(), xferSession->getSkt(), xferInfo.getLclSessionId(), completedFile, xferInfo.getAssetId(), xferInfo.getFileHashId() );
				}
				else
				{
					LogMsg( LOG_ERROR, "FileInfoXferMgr::%s ERROR %d moving %s to %s", __func__, rc, incompleteFile.c_str(), completedFile.c_str() );
					m_FileInfoMgr.toGuiFileDownloadComplete( xferSession->getLclSessionId(), incompleteFile, eXferErrorFileMoveError );
				}
			}
			else if( VxFileUtil::fileExists( incompleteFile.c_str() ) )
			{
				m_FileInfoMgr.onFileDownloadComplete( xferSession->getSendToId(), xferSession->getSkt(), xferInfo.getLclSessionId(), incompleteFile, xferInfo.getAssetId(), xferInfo.getFileHashId() );
				m_FileInfoMgr.toGuiFileDownloadComplete( xferSession->getLclSessionId(), incompleteFile, eXferErrorNone );
			}
			else
			{
				m_FileInfoMgr.toGuiFileDownloadComplete( xferSession->getLclSessionId(), "", eXferErrorFileMoveError );
			}
		}
		else
		{
			m_FileInfoMgr.toGuiFileDownloadComplete( xferSession->getLclSessionId(), "", xferError );
		}
	}
	else
	{
		m_FileInfoMgr.toGuiFileDownloadComplete( xferSession->getLclSessionId(), xferInfo.getRmtFileName().c_str(), xferError);
	}

	endFileXferSession( xferSession );
}

//============================================================================
void FileInfoXferMgr::onFileSent( FileTxSession* xferSession, std::string& fileName, EXferError error )
{
	m_FileInfoMgr.toGuiFileUploadComplete( xferSession->getLclSessionId(), fileName, error );
	endFileXferSession( xferSession );

	checkQueForMoreFilesToSend();
}

//============================================================================
void FileInfoXferMgr::checkQueForMoreFilesToSend( void )
{
	//TODO check que and start next xfer
}

//============================================================================
void FileInfoXferMgr::finishFileReceive( FileRxSession* xferSession, PktFileGetCompleteReq* poPkt )
{
	// done receiving file
	xferSession->setErrorCode( poPkt->getError() );
	VxFileXferInfo& xferInfo = xferSession->getXferInfo();
	if( !xferInfo.isStream() )
	{
		if( xferInfo.m_hFile )
		{
			VFileClose( xferInfo.m_hFile );
			xferInfo.m_hFile = NULL;
		}
		else
		{
			LogMsg( LOG_ERROR,  "FileInfoXferMgr::finishFileReceive: NULL file handle" );
		}
	}

	//=== acknowlege ===//
	PktFileSendCompleteReply pktReply;
	pktReply.setIsStream( poPkt->getIsStream() );
	pktReply.setLclSessionId( xferInfo.getLclSessionId() );
	pktReply.setRmtSessionId( xferInfo.getRmtSessionId() );
	pktReply.setAssetId( xferInfo.getAssetId() );
	pktReply.setFileHashId( xferInfo.getFileHashId() );
	pktReply.setError( xferSession->getErrorCode() );
	m_Plugin.txPacket( xferSession->getSendToId(), xferSession->getSkt(), &pktReply );

	// let other act on the received file
	std::string fileName = xferInfo.getLclFileNameAndPath();
	if( !fileName.empty() )
	{
		if( 0 == xferSession->getErrorCode() )
		{
			FileInfo fileInfo( xferInfo, xferSession->getSendToId() );
			xferSession->m_FilesXferedList.emplace_back( FileToXfer( fileInfo, xferInfo.getLclSessionId(), xferInfo.getRmtSessionId() ) );
		}

        LogMsg( LOG_VERBOSE,  "FileInfoXferMgr::finishFileReceive: Done Receiving from %s file %s",
               m_Engine.describeUser( xferSession->getSendToId() ).c_str(), fileName.c_str() );
		onFileReceived( xferSession, fileName, eXferErrorNone );
	}
	else
	{
		LogMsg( LOG_VERBOSE, "FileInfoXferMgr::finishFileReceive: empty file name" );
	}
}

//============================================================================
int32_t FileInfoXferMgr::sendFileShareError(		std::shared_ptr<VxSktBase>&		sktBase,		// socket
												int				iPktType,	// type of packet
												unsigned short	u16Cmd,		// packet command
												long			rc,			// error code
												const char*		pMsg, ...)	// error message
{
	// send an error message
	int32_t rcSendErr = 0;
	//build message on stack so no out of memory issue
	std::array<char, 2048> szBuffer;
	va_list argList;
	va_start(argList, pMsg);
	vsnprintf( szBuffer.data(), 2048, pMsg, argList);
	va_end(argList);
	LogMsg( LOG_VERBOSE, szBuffer.data() );
	
	return rcSendErr;
}

//============================================================================
bool FileInfoXferMgr::isFileDownloading( VxSha1Hash& fileHashId )
{
	FileRxIter iter;
	for( iter = m_RxSessions.begin(); iter != m_RxSessions.end(); ++iter )
	{
		FileRxSession* xferSession = iter->second;
		if( xferSession->getFileHashId() == fileHashId )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool FileInfoXferMgr::isFileInDownloadFolder( const char* pPartialFileName )
{
	std::string	strFullFileName = VxGetDownloadsDirectory() + pPartialFileName;
	return VxFileUtil::fileExists(strFullFileName.c_str()) ? 1 : 0;
}

//============================================================================
void FileInfoXferMgr::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	FileTxIter iter;
	PluginBase::AutoPluginLock pluginMutexLock( &m_Plugin );
	for( iter = m_TxSessions.begin(); iter != m_TxSessions.end(); ++iter )
	{
		FileTxSession* xferSession = (*iter);
		if( xferSession->getSkt() == poOldSkt )
		{
			xferSession->setSkt( poNewSkt );
		}
	}

	FileRxIter oRxIter;
	for( oRxIter = m_RxSessions.begin(); oRxIter != m_RxSessions.end(); ++oRxIter )
	{
		FileRxSession* xferSession = oRxIter->second;
		if( xferSession->getSkt() == poOldSkt )
		{
			xferSession->setSkt( poNewSkt );
		}
	}
}

//============================================================================
EXferError FileInfoXferMgr::beginFileGet( FileRxSession* xferSession )
{
	if( ( false == xferSession->isXferingFile() ) &&
		xferSession->m_FilesToXferList.size() )
	{
		PktFileGetReq pktReq;
		pktReq.setIsStream( xferSession->m_FilesToXferList[0].isStream() );
		pktReq.setFileName( xferSession->m_FilesToXferList[0].getFileName() );
		pktReq.setLclSessionId( xferSession->m_FilesToXferList[0].getLclSessionId() );
		pktReq.setRmtSessionId( xferSession->m_FilesToXferList[0].getRmtSessionId() );
		pktReq.setAssetId( xferSession->m_FilesToXferList[0].getAssetId() );
		pktReq.setFileHashId( xferSession->m_FilesToXferList[0].getFileHashId() );

		if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_VERBOSE, "%s get from %s %s lcl session %s rmt session %s", __func__,
                m_Engine.describeUser( xferSession->getSendToId() ).c_str(), 
				xferSession->m_FilesToXferList[0].getFileName().c_str(),
				pktReq.getLclSessionId().toHexString().c_str(),
				pktReq.getRmtSessionId().toHexString().c_str());

		EXferError xferErr = m_PluginMgr.pluginApiTxPacket(	m_Plugin.getPluginType(),
												xferSession->getSendToId(), 
												xferSession->getSkt(), 
												&pktReq ) ? eXferErrorNone : eXferErrorDisconnected;
		if( xferSession->isStream() )
		{
			return xferErr;
		}

		if( eXferErrorNone == xferErr )
		{
			m_Plugin.toGuiFileDownloadStart( xferSession->getSendToId(), pktReq.getLclSessionId(), xferSession->m_FilesToXferList[ 0 ] );
		}
		else
		{
			if(LogEnabled(eLogFileXfer)) LogModule( eLogFileXfer, LOG_WARN, "%s disconnected", __func__ );
		}

		return xferErr;
	}
	else if( xferSession->isXferingFile() )
	{
		return eXferErrorBusy;
	}
	else if( xferSession->m_FilesToXferList.empty() )
	{
		return eXferErrorBadParam;
	}

	return eXferErrorDisconnected;
}

//============================================================================
EXferError FileInfoXferMgr::canTxFile( VxGUID sendToId, VxGUID& assetId, VxSha1Hash& fileHashId  )
{
	EXferError xferErr = eXferErrorFileNotFound;
	//#define FILE_XFER_ERR_BUSY						0x0010
	//#define FILE_XFER_ERR_FILE_NOT_FOUND			0x0020
	//#define FILE_XFER_ERR_PERMISSION				0x0040
	if( m_FileInfoMgr.isFileShared( assetId, fileHashId ) )
	{
		//TODO check for busy and permission
		xferErr = eXferErrorNone;
	}

	return xferErr;
}

//============================================================================
bool FileInfoXferMgr::isViewFileListMatch( FileTxSession* xferSession, FileInfo& fileInfo )
{
	size_t viewDirLen = xferSession->m_strViewDirectory.length();
    std::string filePath = fileInfo.getFilePath();
    if( filePath.empty() )
    {
        return false;
    }

	bool bRootDir = viewDirLen ? false : true;

    if( filePath.length() >= viewDirLen )
	{
		if( VXFILE_TYPE_DIRECTORY == fileInfo.getFileType() )
		{
			if( bRootDir )
			{
				return true;
			}
            else if( 0 == strncmp( xferSession->m_strViewDirectory.c_str(), filePath.c_str(), viewDirLen ) )
			{
                if( filePath.length() == viewDirLen )
				{
					// is this directory
					return false;
				}

				return true;
			}

			return false;
		}
		else
		{
            if( filePath.length() > viewDirLen && 0 == strncmp( xferSession->m_strViewDirectory.c_str(), filePath.c_str(), viewDirLen ) )
			{
                const char* pLeftOver = &(filePath.c_str()[viewDirLen]);
				if( strchr(pLeftOver, '/') )
				{
					// is in one of the sub dirs
					return false;
				}

				return true;
			}
			// not the same dir
			return false;
		}
	}

	return false;
}

//============================================================================
bool FileInfoXferMgr::startDownload( FileInfo& fileInfo, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	bool result{ false };
	if( false == lclSessionId.isValid() )
	{
		lclSessionId.initializeWithNewVxGUID();
	}

	FileRxSession* fileRxSession = findOrCreateRxSession( lclSessionId, onlineId, sktBase );
	if( fileRxSession )
	{
		fileRxSession->setIsStream( fileInfo.isStream() );
		fileRxSession->setAssetId( fileInfo.getAssetId() );
		fileRxSession->setFileHashId( fileInfo.getFileHashId() );
		FileToXfer fileToXfer( fileInfo, lclSessionId, lclSessionId );

		fileRxSession->m_FilesToXferList.emplace_back( fileToXfer );
		result = beginFileGet( fileRxSession ) == eXferErrorNone;
	}

	return result;
}


//============================================================================
EXferError FileInfoXferMgr::setupFileDownload( VxFileXferInfo& xferInfo, VxGUID& sendToId )
{
	EXferError xferErr = eXferErrorNone;
	uint64_t u64FileLen = 0;
	if( false == xferInfo.getLclSessionId().isValid() )
	{
		xferInfo.getLclSessionId().initializeWithNewVxGUID();
	}

	xferInfo.setXferDirection( eXferDirectionRx );
	if( xferInfo.m_hFile )
	{
		LogMsg( LOG_ERROR, "FileXferBaseMgr::setupFileDownload: ERROR:(File Receive) receive transfer still in progress" );
		xferErr = eXferErrorAlreadyDownloading;
	}

	std::string rmtFileName = xferInfo.getRmtFileName();
	if( eXferErrorNone == xferErr )
	{
		if( 0 == rmtFileName.length() )
		{
			LogMsg( LOG_ERROR, "FileInfoXferMgr::beginFileReceive: ERROR: No file Name" );
			xferErr = eXferErrorBadParam;
		}
	}

	if( eXferErrorNone == xferErr )
	{
		if( VxFileUtil::isFullPath( rmtFileName.c_str() ) )
		{
			LogMsg( LOG_ERROR, "FileInfoXferMgr::beginFileReceive: ERROR: FULL file Name %s", rmtFileName.c_str() );
			xferErr = eXferErrorBadParam;
		}
	}

	if( eXferErrorNone == xferErr )
	{
		makeIncompleteFileName( rmtFileName, xferInfo.getLclFileNameAndPath(), sendToId );
		if( xferInfo.useFileIo() )
		{
			// make sure the path exists
			std::string filePath;
			std::string justFileName;
			VxFileUtil::seperatePathAndFile( xferInfo.getLclFileNameAndPath(), filePath, justFileName );
			VxFileUtil::makeDirectory( filePath );
			// the file name may have been incremented so might have changed
			vx_assert( !justFileName.empty() );
			vx_assert( !filePath.empty() )
			xferInfo.setLclFileName( justFileName.c_str() );
		}
	}

	if( eXferErrorNone == xferErr )
	{
		if( !xferInfo.useFileIo() )
		{
			xferInfo.setIsOpened( true );
		}
		else
		{
			u64FileLen = VxFileUtil::getFileLen( xferInfo.getLclFileNameAndPath().c_str(), false );
			if( 0 != xferInfo.m_u64FileOffs )
			{
				if( u64FileLen < xferInfo.m_u64FileOffs )
				{
					xferErr = eXferErrorBadParam;
					LogMsg( LOG_INFO, "FileXferBaseMgr: ERROR:(File Rx) %d File %s could not be resumed because too short",
							xferErr,
							xferInfo.getLclFileNameAndPath().c_str() );
				}
				else
				{
					xferInfo.m_hFile = VFileOpen( xferInfo.getLclFileNameAndPath().c_str(), "a+" ); // pointer to name of the file
					if( NULL == xferInfo.m_hFile )
					{
						// failed to open file
						xferInfo.m_hFile = NULL;
						int32_t rc = VxGetLastError();
						xferErr = eXferErrorFileOpenAppendError;

						LogMsg( LOG_INFO, "FileXferBaseMgr: ERROR:(File Rx) %d File %s could not be created",
								rc,
								xferInfo.getLclFileNameAndPath().c_str() );
					}
					else
					{
						// we have valid file so seek to end so we can resume if partial file exists
                        if( 0 != VFileSeek64( xferInfo.m_hFile, xferInfo.m_u64FileOffs ) )
						{
							xferErr = eXferErrorFileOpenAppendError;
							// seek failed
							VFileClose( xferInfo.m_hFile );
							xferInfo.m_hFile = nullptr;
							LogMsg( LOG_INFO, "FileXferBaseMgr: ERROR: (File Rx) could not seek to position %d in file %s",
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
				if( !xferInfo.m_hFile )
				{
					xferErr = eXferErrorFileOpenError;
					// failed to open file
					int32_t rc = VxGetLastError();
					LogMsg( LOG_INFO, "FileInfoXferMgr: ERROR: %d File %s could not be created",
							rc,
							xferInfo.getLclFileNameAndPath().c_str() );
				}
			}
		}
	}

	return xferErr;
}

//============================================================================
bool FileInfoXferMgr::makeIncompleteFileName( std::string& strRemoteFileName, std::string& strRetIncompleteFileName, VxGUID& sendToId )
{
	std::string justFileName;
	VxFileUtil::getFileName( strRemoteFileName.c_str(), justFileName );
	//strRetIncompleteFileName = VxGetIncompleteDirectory() + justFileName;
	strRetIncompleteFileName = m_FileInfoMgr.getIncompleteFileXferDirectory( sendToId ) + justFileName;
	while( VxFileUtil::fileExists( strRetIncompleteFileName.c_str(), false ) )
	{
		if( false == VxFileUtil::incrementFileName( strRetIncompleteFileName ) )
		{
			break;
		}
	}

	return strRetIncompleteFileName.size() ? true : false;
}

//============================================================================
EXferError FileInfoXferMgr::sendNextFileChunk( VxFileXferInfo& xferInfo, VxGUID sendToId, std::shared_ptr<VxSktBase>& skt )
{
	EXferError xferErr = eXferErrorNone;
	PktFileChunkReq pktReq;
	pktReq.setIsStream( xferInfo.isStream() );
	// see how much we can read
	uint32_t u32ChunkLen = ( uint32_t )( xferInfo.m_u64FileLen - xferInfo.m_u64FileOffs );
	if( PKT_TYPE_FILE_MAX_DATA_LEN < u32ChunkLen )
	{
		u32ChunkLen = PKT_TYPE_FILE_MAX_DATA_LEN;
	}

	if( 0 == u32ChunkLen )
	{
		LogMsg( LOG_ERROR, "FileInfoXferMgr::%s ERROR 0 len u32ChunkLen", __func__ );
		// what to do?
		return eXferErrorNone;
	}

	// read data into packet
	uint32_t u32BytesRead = ( uint32_t )VFileRead( pktReq.m_au8FileChunk,
		1,
		u32ChunkLen,
		xferInfo.m_hFile );
	if( u32BytesRead != u32ChunkLen )
	{
		int32_t rc = VxGetLastError();
		//xferSession->setErrorCode( rc );
		xferErr = eXferErrorFileReadError;

		VFileClose( xferInfo.m_hFile );
		xferInfo.m_hFile = NULL;
		LogMsg( LOG_ERROR, "FileXferBaseMgr: ERROR: %d reading send file at offset %" PRIu64 " when file len %" PRIu64 "  file name %s",
			rc,
			xferInfo.m_u64FileOffs,
			xferInfo.m_u64FileLen,
			( const char* )xferInfo.getLclFileNameAndPath().c_str() );
	}
	else
	{
		pktReq.setChunkOffset( xferInfo.m_u64FileOffs );
		xferInfo.m_u64FileOffs += u32ChunkLen;
		pktReq.setChunkLen( ( uint16_t )u32ChunkLen );
		pktReq.setLclSessionId( xferInfo.getLclSessionId() );
		pktReq.setRmtSessionId( xferInfo.getRmtSessionId() );
		pktReq.setAssetId( xferInfo.getAssetId() );
		if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s %" PRIu64 " of %" PRIu64, __func__, xferInfo.m_u64FileOffs, xferInfo.m_u64FileLen );
	}

	if( eXferErrorNone == xferErr )
	{
		if( false == m_Plugin.txPacket( sendToId, skt, &pktReq ) )
		{
			xferErr = eXferErrorDisconnected;
		}
	}

	if( eXferErrorNone != xferErr )
	{
		m_FileInfoMgr.updateToGuiFileXferState( xferInfo.getLclSessionId(), eXferDirectionTx, eXferStateUploadError, xferErr );
	}
	else
	{
		if( xferInfo.calcProgress() )
		{
			m_FileInfoMgr.updateToGuiFileXferState( xferInfo.getLclSessionId(), eXferDirectionTx, eXferStateInUploadXfer, eXferErrorNone, xferInfo.getProgress() );
		}
	}

	return xferErr;
}

//============================================================================
void FileInfoXferMgr::wantFileXferCallback( FileXferCallback* client, bool wantCallback )
{
    if( !client )
    {
        LogMsg( LOG_ERROR, "%s null client", __func__ );
		vx_assert( false );
        return;
    }

    lockClientList();
    bool foundClient{ false };
    for( auto iter = m_FileXferCallbackClients.begin(); iter != m_FileXferCallbackClients.end(); ++iter )
    {
        if( *iter == client )
        {
            foundClient = true;
            if( !wantCallback )
            {
                m_FileXferCallbackClients.erase( iter );
            }

            break;
        }
    }

    if( !foundClient && wantCallback )
    {
        m_FileXferCallbackClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void FileInfoXferMgr::announcePkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	lockClientList();
    for( auto& client : m_FileXferCallbackClients )
    {
		client->onFileXferPktRxed( sktBase, pktHdr );
    }

    unlockClientList();
}

//============================================================================
void FileInfoXferMgr::sendFileXferCancel( FileShareXferSession* xferSession )
{
	if( xferSession->getSkt().get() && xferSession->getSkt()->isConnected() && xferSession->getSendToId().isValid() )
	{
		sendFileXferCancel( xferSession->getSkt(), xferSession );
	}
	else
	{
		if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s no connection", __func__ );
	}
}

//============================================================================
void FileInfoXferMgr::sendFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, FileShareXferSession* xferSession )
{
	if( sktBase.get() && sktBase->isConnected() && xferSession->getSendToId().isValid() )
	{
		PktFileXferCancel pktXferCancel;
		VxFileXferInfo& xferInfo = xferSession->getXferInfo();
		pktXferCancel.setLclSessionId( xferInfo.getLclSessionId() );
		pktXferCancel.setRmtSessionId( xferInfo.getRmtSessionId() );
		pktXferCancel.setAssetId( xferInfo.getAssetId() );
		pktXferCancel.setIsStream( xferSession->isStream() );
		m_Plugin.txPacket( xferSession->getSendToId(), sktBase, &pktXferCancel );
	}
	else
	{
		if( LogEnabled( eLogFileXfer ) ) LogModule( eLogFileXfer, LOG_VERBOSE, "FileInfoXferMgr::%s no connection", __func__ );
	}
}

//============================================================================
void FileInfoXferMgr::addTxSession( FileTxSession* xferSession )
{
	for( auto txSession : m_TxSessions )
	{
		if( xferSession == txSession )
		{
			LogMsg( LOG_ERROR, "FileInfoXferMgr::%s attempted to add same session again", __func__ );
			return;
		}
	}

	m_TxSessions.emplace_back( xferSession );
}