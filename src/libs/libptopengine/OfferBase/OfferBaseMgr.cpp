//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferBaseMgr.h"

#include "OfferBaseInfo.h"
#include "OfferBaseInfoDb.h"
#include "OfferCallback.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>
#include <Plugins/PluginFileShareServer.h>

#include <GuiInterface/IToGui.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsFileList.h>

#include <CoreLib/Sha1GeneratorMgr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>

#include <time.h>

namespace
{
	const char* OFFER_INFO_DB_NAME = "OfferInfoDb.db3";
	const char* OFFER_STATE_DB_NAME = "OfferStateDb.db3";

	//============================================================================
    static void * OfferBaseMgrStartupThreadFunc( void * pvContext )
	{
		VxThread* poThread = (VxThread*)pvContext;
		poThread->setIsThreadRunning( true );
		OfferBaseMgr * poMgr = (OfferBaseMgr *)poThread->getThreadUserParam();
        if( poMgr )
        {
            poMgr->offerInfoMgrStartup( poThread );
        }

		poThread->threadAboutToExit();
        return nullptr;
	}
}

//============================================================================
OfferBaseMgr::OfferBaseMgr( EOfferMgrType assetMgrType )
: m_OfferMgrType( assetMgrType )
, m_OfferBaseInfoDb( *this )
{
}

//============================================================================
bool OfferBaseMgr::deleteDatabase( void )
{
	return m_OfferBaseInfoDb.deleteDatabase();
}

//============================================================================
void OfferBaseMgr::fromGuiUserLoggedOn( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( !m_Initialized )
	{
		// user specific directory should be set
		std::string dbInfoFileName = VxGetSettingsDirectory();
		dbInfoFileName += OFFER_INFO_DB_NAME;

		lockResources();
		m_OfferBaseInfoDb.dbShutdown();
		m_OfferBaseInfoDb.dbStartup( 1, dbInfoFileName );

		clearOfferInfoList();
		m_OfferBaseInfoDb.getAllOffers( m_OfferBaseInfoList );

		unlockResources();
		m_Initialized = true;		
	}
}

//============================================================================
void OfferBaseMgr::onPluginsInitialized( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( !m_Initialized )
	{
		m_Initialized = true;
		m_OfferMgrStartupThread.startThread( (VX_THREAD_FUNCTION_T)OfferBaseMgrStartupThreadFunc, this, "OfferBaseMgrStartup" );			
	}		
}

//============================================================================
void OfferBaseMgr::offerInfoMgrStartup( VxThread* startupThread )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( startupThread->isAborted() )
		return;
	// user specific directory should be set
	std::string dbName = VxGetSettingsDirectory();
	dbName += OFFER_INFO_DB_NAME; 
	lockResources();
	m_OfferBaseInfoDb.dbShutdown();
	m_OfferBaseInfoDb.dbStartup( 1, dbName );
	unlockResources();
	if( startupThread->isAborted() )
		return;
	updateOfferListFromDb( startupThread );
	m_OfferBaseListInitialized = true;
}

//============================================================================
void OfferBaseMgr::offerInfoMgrShutdown( void )
{
	lockResources();
	clearOfferInfoList();
	clearOfferFileListPackets();
	m_OfferBaseInfoDb.dbShutdown();
	unlockResources();
	m_OfferBaseListInitialized = false;
	m_Initialized = false;
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::findOffer( std::string& fileNameAndPath )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( (*iter)->getOfferName() == fileNameAndPath )
		{
			return (*iter);
		}
	}

	return nullptr;
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::findOffer( VxSha1Hash& fileHashId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( false == fileHashId.isHashValid() )
	{
		LogMsg( LOG_ERROR, "OfferBaseMgr::findOffer: invalid file hash id" );
		return 0;
	}

	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( (*iter)->getOfferHashId() == fileHashId )
		{
			return (*iter);
		}
	}

	return nullptr;
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::findOffer( VxGUID& offerId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( false == offerId.isValid() )
	{
		LogMsg( LOG_ERROR, "OfferBaseMgr::findOffer: invalid VxGUID offer id" );
        return nullptr;
	}

	for( OfferBaseInfo* offerInfo : m_OfferBaseInfoList )
	{
		if( offerInfo->getOfferId() == offerId )
		{
			return offerInfo;
		}
	}

	return nullptr;
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::addOfferFile( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, uint16_t fileType )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    OfferBaseInfo* offerInfo = createOfferInfo( fileName, fileNameAndPath, fileLen, fileType );
    if( offerInfo )
    {
        if( insertNewInfo( offerInfo ) )
        {
            return offerInfo;
        }
    }

    return nullptr;
}

//============================================================================
bool OfferBaseMgr::addOfferFile(	const char*		fileName, 
									const char*		fileNameAndPath,
									VxGUID&			assetId,  
									uint8_t *		hashId, 
									EOfferLocation	locationFlags, 
									const char*		assetTag, 
                                    int64_t			timestamp )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	OfferBaseInfo* offerInfo = createOfferInfo( fileName, fileNameAndPath, assetId, hashId, locationFlags, assetTag, timestamp );
	if( offerInfo )
	{
		return insertNewInfo( offerInfo );
	}
	
	return false;
}

//============================================================================
bool OfferBaseMgr::addOfferFile(	const char*		fileName, 
									const char*		fileNameAndPath,
									VxGUID&			assetId,  
									VxGUID&		    creatorId, 
									VxGUID&		    historyId, 
									uint8_t *		hashId, 
									EOfferLocation	locationFlags, 
									const char*		assetTag, 
                                    int64_t			timestamp )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	OfferBaseInfo* offerInfo = createOfferInfo( fileName, fileNameAndPath, assetId, hashId, locationFlags, assetTag, timestamp );
	if( offerInfo )
	{
		offerInfo->setCreatorId( creatorId );
		offerInfo->setCreatorId( historyId );
		return insertNewInfo( offerInfo );
	}
	
	return false;
}

//============================================================================
bool OfferBaseMgr::addOffer( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	OfferBaseInfo* newOfferBaseInfo = createOfferInfo( offerInfo );
	return insertNewInfo( newOfferBaseInfo );
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::createOfferInfo( std::string fileName, std::string fileNameAndPath, uint64_t fileLen, uint16_t fileType )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    OfferBaseInfo* offerInfo = new OfferBaseInfo( fileName, fileNameAndPath, fileLen, fileType );
    if( offerInfo )
    {
        offerInfo->getOfferId().initializeWithNewVxGUID();
    }

    return offerInfo;
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::createOfferInfo( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	return new OfferBaseInfo( offerInfo );
}

//============================================================================
OfferBaseInfo* OfferBaseMgr::createOfferInfo(	std::string		fileName,
												std::string		fileNameAndPath,
										        VxGUID&			assetId,  
										        uint8_t *	    hashId, 
										        EOfferLocation	locationFlags, 
										        const char*	assetTag, 
                                                int64_t			timestamp )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	uint64_t  fileLen = VxFileUtil::getFileLen( fileName.c_str() );
	uint8_t	fileType = VxFileExtensionToFileTypeFlag( fileName.c_str() );
	if( ( false == isAllowedFileOrDir( fileName ) )
		|| ( 0 == fileLen ) )
	{
        LogMsg( LOG_ERROR, "ERROR %d OfferBaseMgr::createOfferInfo could not get file info %s", VxGetLastError(), fileName.c_str() );
		return NULL;
	}

	OfferBaseInfo* offerInfo = createOfferInfo( fileName, fileNameAndPath, fileLen, fileType );
	if( false == offerInfo->getOfferId().isValid() )
	{
		offerInfo->getOfferId().initializeWithNewVxGUID();
	}

	offerInfo->getOfferHashId().setHashData( hashId );
	offerInfo->setLocationFlags( locationFlags );
	offerInfo->setOfferTag( assetTag );
	offerInfo->setCreationTime( timestamp ? timestamp : GetTimeStampMs() );

	return offerInfo;
}

//============================================================================
bool OfferBaseMgr::insertNewInfo( OfferBaseInfo* offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool result = false;
	OfferBaseInfo* offerInfoExisting = findOffer( offerInfo->getOfferId() );
	if( offerInfoExisting )
	{
		LogMsg( LOG_ERROR, "ERROR OfferBaseMgr::insertNewInfo: duplicate assset %s", offerInfo->getOfferName().c_str() );
		if( offerInfoExisting != offerInfo )
		{
			*offerInfoExisting = *offerInfo;
			offerInfo = offerInfoExisting;
		}
	}

	if( 0 == offerInfo->getCreationTime() )
	{
		offerInfo->setCreationTime( GetTimeStampMs() );
	}

	//if( offerInfo->needsHashGenerated() )
	//{
	//	lockResources();
	//	m_WaitingForHastList.emplace_back( offerInfo );
	//	unlockResources();
	//	generateHashForFile( offerInfo->getOfferName() );
	//	result = true;
	//}
	//else
	{
        updateDatabase( offerInfo );
		if( !offerInfoExisting )
		{
			lockResources();
			m_OfferBaseInfoList.emplace_back( offerInfo );
			unlockResources();
			announceOfferAdded( offerInfo );
		}
        else
        {
            announceOfferUpdated( offerInfo );
        }
	
		result = true;
	}

	return result;
}

//============================================================================
bool OfferBaseMgr::updateOffer( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    OfferBaseInfo* existingOffer = findOffer( offerInfo.getOfferId() );
    if( existingOffer )
    {
        *existingOffer = offerInfo;
        updateDatabase( existingOffer );
        announceOfferUpdated( existingOffer );
        return true;
    }

    return false;
}

//============================================================================
void OfferBaseMgr::announceOfferAdded( OfferBaseInfo* offerInfo, bool resourceLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( offerInfo->isFileOffer() )
	{
		updateFileListPackets();
		updateOfferFileTypes();
	}
	
	if( !resourceLocked )
	{
		lockClientList();
	}

	for( auto& client : m_OfferClients )
	{
		client->callbackOfferAdded( offerInfo );
	}

	if( !resourceLocked )
	{
		unlockClientList();
	}
}

//============================================================================
void OfferBaseMgr::announceOfferUpdated( OfferBaseInfo* offerInfo )
{
    if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    lockClientList();
	for( auto& client : m_OfferClients )
	{
        client->callbackOfferAdded( offerInfo );
    }

    unlockClientList();
}

//============================================================================
void OfferBaseMgr::announceOfferRemoved( OfferBaseInfo* offerInfo, bool resourceLocked )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( !offerInfo )
    {
        LogMsg( LOG_ERROR, "OfferHostMgr::announceOfferRemoved null offerInfo" );
		vx_assert( false );
		return;
    }

	//if( offerInfo->getIsOfferBase() )
	{
		updateFileListPackets();
		updateOfferFileTypes();
	}

	if( !resourceLocked )
	{
		lockClientList();
	}

	for( auto& client : m_OfferClients )
	{
		client->callbackOfferRemoved( offerInfo->getOfferId() );
	}

	if( !resourceLocked )
	{
		unlockClientList();
	}
}

//============================================================================
void OfferBaseMgr::announceOfferXferState( VxGUID& assetOfferId, EOfferSendState assetSendState, int param )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	lockClientList();
	for( auto& client : m_OfferClients )
	{
		client->callbackOfferSendState( assetOfferId, assetSendState, param );
	}

	unlockClientList();
	LogMsg( LOG_INFO, "OfferBaseMgr::announceOfferXferState state %d done", assetSendState );
}

//============================================================================
void OfferBaseMgr::announceOfferAction( VxGUID& assetOfferId, EOfferAction offerAction, int param )
{
    if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    lockClientList();
	for( auto& client : m_OfferClients )
	{
        client->callbackOfferAction( assetOfferId, offerAction, param );
    }

    unlockClientList();
    LogMsg( LOG_INFO, "OfferBaseMgr::announceOfferXferState state %d done", offerAction );
}

//============================================================================
bool OfferBaseMgr::removeOffer( std::string fileName )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool assetRemoved = false;
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( fileName == (*iter)->getOfferName() )
		{
			OfferBaseInfo* offerInfo = *iter;
			m_OfferBaseInfoList.erase( iter );
			m_OfferBaseInfoDb.removeOffer( fileName.c_str() );
			announceOfferRemoved( offerInfo );
			delete offerInfo;
			assetRemoved = true;
			break;
		}
	}

	return assetRemoved;
}

//============================================================================
bool OfferBaseMgr::removeOffer( VxGUID& assetOfferId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool assetRemoved = false;
	std::vector<OfferBaseInfo*>::iterator iter;
	for ( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( assetOfferId == ( *iter )->getOfferId() )
		{
			OfferBaseInfo* offerInfo = *iter;
			m_OfferBaseInfoList.erase( iter );
			m_OfferBaseInfoDb.removeOffer( offerInfo );
			announceOfferRemoved( offerInfo );
			delete offerInfo;
			assetRemoved = true;
			break;
		}
	}

	return assetRemoved;
}

//============================================================================
void OfferBaseMgr::clearOfferInfoList( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		delete (*iter);
	}

	m_OfferBaseInfoList.clear();
}

//============================================================================
void OfferBaseMgr::updateOfferListFromDb( VxThread* startupThread )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	std::vector<AssetBaseInfo*> toDeleteList;

	lockResources();
	clearOfferInfoList();
	m_OfferBaseInfoDb.getAllOffers( m_OfferBaseInfoList );

	// there should not be any without valid hash but if is then generate it
	for( auto iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end();  )
	{
		if( startupThread->isAborted() )
		{
			unlockResources();
			return;
		}

		OfferBaseInfo* offerInfo = (*iter);
		if( !offerInfo->validateAssetExist() )
		{
			// add to list to remove from database and delete
			toDeleteList.emplace_back( offerInfo );
			++iter;
			continue;
		}

		if( !offerInfo->isValid() )
		{
			toDeleteList.emplace_back( offerInfo );
			++iter;
			continue;
		}

		if( offerInfo->isSharedFileAsset() )
		{
			GetPtoPEngine().getPluginFileShareServer().fileShareEnable(offerInfo, true);
		}

		EOfferSendState sendState = offerInfo->getOfferSendState();
		if( eOfferSendStateTxProgress == sendState ) 
		{
			offerInfo->setOfferSendState( eOfferSendStateTxFail );
			m_OfferBaseInfoDb.updateOfferSendState( offerInfo->getOfferId(), eOfferSendStateTxFail );
		}
		else if(  eOfferSendStateRxProgress == sendState  )
		{
			offerInfo->setOfferSendState( eOfferSendStateRxFail );
			m_OfferBaseInfoDb.updateOfferSendState( offerInfo->getOfferId(), eOfferSendStateRxFail );
		}

		if( offerInfo->needsHashGenerated() )
		{
			m_WaitingForHastList.emplace_back( offerInfo );
			iter = m_OfferBaseInfoList.erase( iter );
			requestFileHash( offerInfo );
			break;
		}
		else
		{
			++iter;
		}
	}

	unlockResources();
	updateFileListPackets();
	updateOfferFileTypes();
}

//============================================================================
void OfferBaseMgr::updateOfferFileTypes( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	uint16_t u16FileTypes = 0;
	std::vector<OfferBaseInfo*>::iterator iter;
	lockResources();
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( (*iter)->isFileOffer() )
		{
			u16FileTypes		|= (*iter)->getOfferType();
		}
	}

	unlockResources();
	// ignore extended types
	u16FileTypes = u16FileTypes & 0xff;
	m_u16OfferBaseFileTypes = u16FileTypes;
	bool fileTypesChanged = false;

	GetPtoPEngine().lockAnnouncePktAccess();
	PktAnnounce& pktAnn = GetPtoPEngine().getMyPktAnnounce();
	if( pktAnn.getSharedFileTypes() != u16FileTypes )
	{
		fileTypesChanged = true;
		pktAnn.setSharedFileTypes( (uint8_t)u16FileTypes );
	}

	GetPtoPEngine().unlockAnnouncePktAccess();
	if( fileTypesChanged )
	{
		lockClientList();
		for( auto& client : m_OfferClients )
		{
			client->callbackOfferFileTypesChanged( u16FileTypes );
		}

		unlockClientList();
	}
}

//============================================================================
void OfferBaseMgr::updateFileListPackets( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool hadOfferBaseFiles = m_FileListPackets.size() ? true : false;
	PktFileListReply * pktFileList = 0;
	clearOfferFileListPackets();
	lockFileListPackets();
	lockResources();
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		OfferBaseInfo* offerInfo = (*iter); 
		if( ( false == offerInfo->isFileOffer() ) || ( false == offerInfo->getOfferHashId().isHashValid() ) )
			continue;

		if( 0 == pktFileList )
		{
			pktFileList = new PktFileListReply();
			pktFileList->setListIndex( (uint32_t)m_FileListPackets.size() );
		}

		if( pktFileList->canAddFile( (uint32_t)(offerInfo->getRemoteOfferName().size() + 1) ) )
		{
			pktFileList->addFile(	offerInfo->getOfferHashId(),
									offerInfo->getOfferLength(),
									offerInfo->getOfferType(),
									offerInfo->getRemoteOfferName().c_str() );
		}
		else
		{
			m_FileListPackets.emplace_back( pktFileList );
			pktFileList = new PktFileListReply();
			pktFileList->setListIndex( (uint32_t)m_FileListPackets.size() );
			pktFileList->addFile(	offerInfo->getOfferHashId(),
									offerInfo->getOfferLength(),
									offerInfo->getOfferType(),
									offerInfo->getOfferName().c_str() );
		}
	}

	if( 0 != pktFileList )
	{
		if( pktFileList->getFileCount() )
		{
			pktFileList->setIsListCompleted( true ); // last pkt in list
			m_FileListPackets.emplace_back( pktFileList );
		}
		else
		{
			delete pktFileList;
		}
	}

	unlockResources();
	unlockFileListPackets();
	if( hadOfferBaseFiles || m_FileListPackets.size() )
	{
		lockClientList();
		for( auto& client : m_OfferClients )
		{
			client->callbackOfferPktFileListUpdated();
		}

		unlockClientList();
	}
}

//============================================================================
void OfferBaseMgr::clearOfferFileListPackets( void )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	lockFileListPackets();
	std::vector<PktFileListReply*>::iterator iter;
	for( iter = m_FileListPackets.begin(); iter != m_FileListPackets.end(); ++iter )
	{
		delete (*iter);
	}

	m_FileListPackets.clear();
	unlockFileListPackets();
}

//============================================================================
bool OfferBaseMgr::fromGuiSetFileIsShared( std::string fileName, std::string fileNameAndPath, bool shareFile, uint8_t * fileHashId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	lockResources();
	OfferBaseInfo* offerInfo = findOffer( fileNameAndPath );
	if( offerInfo )
	{
		if( ( false == shareFile ) && offerInfo->isSharedFileOffer() )
		{
			offerInfo->setIsSharedFileOffer( false );
			updateDatabase( offerInfo );
			unlockResources();
			updateOfferFileTypes();
			updateFileListPackets();
			return true;
		}
	}

	unlockResources();

	if( shareFile )
	{
		// file is not currently OfferBase and should be
		VxGUID guid;
		OfferBaseInfo* offerInfo = createOfferInfo( fileName.c_str(), fileNameAndPath.c_str(), guid, fileHashId, eOfferLocShared );
		if( offerInfo )
		{
			insertNewInfo( offerInfo );
		}
	}
	else
	{
		return false;
	}

	return true;
}

//============================================================================
bool OfferBaseMgr::getFileHashId( std::string& fileFullName, VxSha1Hash& retFileHashId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool foundHash = false;
	lockResources();
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( fileFullName == (*iter)->getOfferName() )
		{
			retFileHashId = (*iter)->getOfferHashId();
			foundHash = retFileHashId.isHashValid();
			break;
		}
	}

	unlockResources();
	return foundHash;
}

//============================================================================
bool OfferBaseMgr::getFileFullName( VxSha1Hash& fileHashId, std::string& retFileFullName )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool isOfferBase = false;
	lockResources();
	std::vector<OfferBaseInfo*>::iterator iter;
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		if( fileHashId == (*iter)->getOfferHashId() )
		{
			isOfferBase = true;
			retFileFullName = (*iter)->getOfferName();
			break;
		}
	}

	unlockResources();
	return isOfferBase;
}

//============================================================================
bool OfferBaseMgr::hostedOfferExists( OfferBaseInfo& offerInfo, bool updateResponse )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool offerExists{ false };
	lockResources();
	for( auto& offerInfoitem : m_OfferBaseInfoList )
	{
		if( eOfferMgrHost == offerInfoitem->getOfferMgr() && offerInfoitem->getOfferId() == offerInfo.getOfferId() )
		{
			if( updateResponse )
			{
				offerInfoitem->setOfferResponse( offerInfo.getOfferResponse() );
			}

			offerExists = true;
			break;
		}
	}

	unlockResources();

	return offerExists;
}

//============================================================================
bool OfferBaseMgr::clientOfferExists( OfferBaseInfo& offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	bool offerExists{ false };
	lockResources();
	for( auto offerInfoitem : m_OfferBaseInfoList )
	{
		if( eOfferMgrClient == offerInfoitem->getOfferMgr() && offerInfoitem->getOfferId() == offerInfo.getOfferId() )
		{
			offerExists = true;
			break;
		}
	}

	unlockResources();

	return offerExists;
}

//============================================================================
void OfferBaseMgr::updateDatabase( OfferBaseInfo* offerInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    if( offerInfo->getPluginType() != ePluginTypePersonFileXfer )
    {
        // personal file offers go away on reboot
        m_OfferBaseInfoDb.addOffer( offerInfo );
    }
}

//============================================================================
void OfferBaseMgr::updateOfferDatabaseSendState( VxGUID& assetOfferId, EOfferSendState sendState )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	m_OfferBaseInfoDb.updateOfferSendState( assetOfferId, sendState );
}

//============================================================================
void OfferBaseMgr::queryHistoryOffers( VxGUID& historyId )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );

	OfferBaseInfo* offerInfo;
	lockResources();
	for( auto iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		offerInfo = (*iter);
		if( offerInfo->getHistoryId() == historyId )
		{
            onQueryHistoryOffer( offerInfo );
			// BRJ IToGui::getIToGui().toGuiOfferSessionHistory( offerInfo );
		}
	}

	unlockResources();
}

//============================================================================
void OfferBaseMgr::updateOfferXferState( VxGUID& assetOfferId, EOfferSendState assetSendState, int param )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s %s param %d", __func__, DescribeOfferSendState( assetSendState ), param );
	std::vector<OfferBaseInfo*>::iterator iter;
	OfferBaseInfo* offerInfo;
	bool assetSendStateChanged = false;
	lockResources();
	for( iter = m_OfferBaseInfoList.begin(); iter != m_OfferBaseInfoList.end(); ++iter )
	{
		offerInfo = (*iter);
		if( offerInfo->getOfferId() == assetOfferId )
		{
			EOfferSendState oldSendState = offerInfo->getOfferSendState();
			if( oldSendState != assetSendState )
			{
				offerInfo->setOfferSendState( assetSendState );
				assetSendStateChanged = true;
				//updateDatabase( offerInfo );
				updateOfferDatabaseSendState( assetOfferId, assetSendState );
				switch( assetSendState )
				{
				case eOfferSendStateTxProgress:
					if( eOfferSendStateNone == oldSendState )
					{
						announceOfferAction( assetOfferId, eOfferActionTxBegin, param );
					}

					announceOfferAction( assetOfferId, eOfferActionTxProgress, param );
					break;

				case eOfferSendStateRxProgress:
					if( eOfferSendStateNone == oldSendState )
					{
						announceOfferAction( assetOfferId, eOfferActionRxBegin, param );
					}

					announceOfferAction( assetOfferId, eOfferActionRxProgress, param );
					break;

				case eOfferSendStateRxSuccess:
					if( ( eOfferSendStateNone == oldSendState )
						|| ( eOfferSendStateRxProgress == oldSendState ) )
					{
						announceOfferAction( assetOfferId, eOfferActionRxSuccess, param );
						announceOfferAction( offerInfo->getCreatorId(), eOfferActionRxNotifyNewMsg, 100 );
					}
					else 
					{
						announceOfferAction( assetOfferId, eOfferActionRxSuccess, param );
					}

					break;

				case eOfferSendStateTxSuccess:
					announceOfferAction( assetOfferId, eOfferActionTxSuccess, param );
					break;

				case eOfferSendStateRxFail:
					announceOfferAction( assetOfferId, eOfferActionRxError, param );
					break;

				case eOfferSendStateTxFail:
					announceOfferAction( assetOfferId, eOfferActionTxError, param );
					break;

				case eOfferSendStateTxPermissionErr:
					announceOfferAction( assetOfferId, eOfferActionTxError, param );
					break;

				case eOfferSendStateRxPermissionErr:
					announceOfferAction( assetOfferId, eOfferActionRxError, param );
					break;


				case eOfferSendStateNone:
				default:
					break;
				}
			}

			break;
		}
	}

	unlockResources();
	if( assetSendStateChanged )
	{
        announceOfferXferState( assetOfferId, assetSendState, param );
	}

	LogMsg( LOG_INFO, "OfferBaseMgr::updateOfferXferState state %d done", assetSendState );
}


//============================================================================
bool OfferBaseMgr::isAllowedFileOrDir( std::string strFileName )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
    if( VxIsExecutableFile( strFileName ) 
        || VxIsShortcutFile( strFileName ) )
    {
        return false;
    }

    return true;
}

//============================================================================
void OfferBaseMgr::wantOfferCallbacks( OfferCallback* client, bool enable )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	lockClientList();

	bool found{ false };
	for( auto iter = m_OfferClients.begin(); iter != m_OfferClients.end(); ++iter )
	{
		if( *iter == client )
		{
			found = true;
			if( !enable )
			{
				m_OfferClients.erase( iter );
			}

			break;
		}
	}

	if( enable && !found )
	{
		m_OfferClients.emplace_back( client );
	}

	unlockClientList();
}

//============================================================================
void OfferBaseMgr::requestFileHash( OfferBaseInfo* assetInfo )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	GetSha1GeneratorMgr().generateSha1( assetInfo->getAssetUniqueId(), assetInfo->getAssetName(), assetInfo->getFileNameAndPath(), this);
}

//============================================================================
void OfferBaseMgr::callbackSha1GenerateResult( ESha1GenResult sha1GenResult, VxGUID& assetId, Sha1Info& sha1Info )
{
	if(LogEnabled(eLogOffer))LogModule( eLogOffer, LOG_VERBOSE, "OfferBaseMgr::%s", __func__ );
	if( eSha1GenResultNoError == sha1GenResult )
	{
		lockResources();

		// move from waiting to completed
		bool wasMoved{ false };
		for( auto iter = m_WaitingForHastList.begin(); iter != m_WaitingForHastList.end(); ++iter )
		{
			OfferBaseInfo* inListAssetBaseInfo = *iter;
			if( assetId == inListAssetBaseInfo->getAssetUniqueId() && sha1Info.getFileNameAndPath() == inListAssetBaseInfo->getAssetNameAndPath() )
			{
				OfferBaseInfo* toMoveAssetInfo = inListAssetBaseInfo;
				m_WaitingForHastList.erase( iter );
				toMoveAssetInfo->setAssetHashId( sha1Info.getSha1Hash() );
				m_OfferBaseInfoList.emplace_back( toMoveAssetInfo );
				updateDatabase( toMoveAssetInfo );
				announceOfferAdded( toMoveAssetInfo, true );
				wasMoved = true;
				break;
			}
		}

		bool wasFound{ false };
		if( !wasMoved )
		{
			for( auto* assetInfo : m_OfferBaseInfoList )
			{
				if( assetId == assetInfo->getAssetUniqueId() && sha1Info.getFileNameAndPath() == assetInfo->getAssetNameAndPath() )
				{
					assetInfo->setAssetHashId( sha1Info.getSha1Hash() );
					updateDatabase( assetInfo );
					wasFound = true;
					break;
				}
			}
		}

		unlockResources();
		if( wasMoved || wasFound )
		{
			GetPtoPEngine().getPluginFileShareServer().fromGuiFileHashGenerated( sha1Info.getFileNameAndPath(), sha1Info.getFileLen(), sha1Info.getSha1Hash() );
		}
	}
	else
	{
		LogMsg( LOG_VERBOSE, "OfferBaseMgr::%s failed %s", __func__, DescribeSha1GenResult( sha1GenResult ) );
	}
}
