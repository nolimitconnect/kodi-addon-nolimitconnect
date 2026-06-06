//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseFiles.h"

#include "FileInfoBaseMgr.h"
#include "PluginMgr.h"

#include <Plugins/FileInfo.h>
#include <P2PEngine/P2PEngine.h>

#include <GuiInterface/IToGui.h>

#include <PktLib/PktsFileShare.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/VxSearchDefs.h>
#include <PktLib/PktsFileInfo.h>
#include <PktLib/SearchParams.h>
#include <PktLib/VxCommon.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxGlobals.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

//============================================================================
PluginBaseFiles::PluginBaseFiles( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, FileInfoBaseMgr& fileInfoBaseMgr )
: PluginBase( engine, pluginMgr, myIdent, pluginType ) 
, m_PluginSessionMgr( engine, *this, pluginMgr)
, m_FileInfoMgr( fileInfoBaseMgr )
, m_FileInfoXferMgr( m_FileInfoMgr.getFileInfoXferMgr() )
{
	setPluginType( pluginType );
	if(LogEnabled(eLogStartup))LogMsg( LOG_VERBOSE, "PluginBaseFiles::%s %s ", __func__, DescribePluginType( pluginType ) );
}

//============================================================================
void PluginBaseFiles::onAfterUserLogOnThreaded( void )
{
	m_FileInfoMgr.onAfterUserLogOnThreaded();
	m_FileInfoXferMgr.onAfterUserLogOnThreaded();
}

//============================================================================
bool PluginBaseFiles::fromGuiStartPluginSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_FileInfoXferMgr.fromGuiStartPluginSession( onlineId, lclSessionId );
}

//============================================================================
void PluginBaseFiles::fromGuiStopPluginSession( VxGUID& onlineId, VxGUID lclSessionId)
{
	return m_FileInfoXferMgr.fromGuiStopPluginSession( onlineId, lclSessionId );
}

//============================================================================
bool PluginBaseFiles::fromGuiIsPluginInSession( VxGUID& onlineId, VxGUID lclSessionId )
{
	return m_FileInfoXferMgr.fromGuiIsPluginInSession( onlineId, lclSessionId );
}

//============================================================================
void PluginBaseFiles::fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings )
{
	m_FileInfoXferMgr.fromGuiGetFileShareSettings( fileShareSettings );
}

//============================================================================
void PluginBaseFiles::fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings )
{
	m_FileInfoXferMgr.fromGuiSetFileShareSettings( fileShareSettings );
}

//============================================================================
void PluginBaseFiles::fromGuiCancelDownload( VxGUID& fileInstance )
{
	return m_FileInfoXferMgr.fromGuiCancelDownload( fileInstance );
}

//============================================================================
void PluginBaseFiles::fromGuiCancelUpload( VxGUID& fileInstance )
{
	return m_FileInfoXferMgr.fromGuiCancelUpload( fileInstance );
}

//============================================================================
bool PluginBaseFiles::fromGuiBrowseFiles( VxGUID& appInstId, std::string& dir, uint8_t fileFilterMask )
{
	if( 0 == fileFilterMask )
	{
		fileFilterMask = VXFILE_TYPE_ALLNOTEXE | VXFILE_TYPE_DIRECTORY;
	}

	std::vector<VxFileInfoBase> fileList;
	int32_t rc = VxFileUtil::listFilesAndFolders( dir.c_str(), fileList, fileFilterMask );
	if( rc )
	{
		LogMsg( LOG_ERROR, "PluginBaseFiles::fromGuiBrowseFiles error %d", rc );
		return false;
	}

	int fileNum{ 0 };
	for( auto& vxFileInfo : fileList )
	{
		if ( ( false == vxFileInfo.isExecutableFile() )
			&& ( false == vxFileInfo.isShortcutFile() ) )
		{
			fileNum++;
			if ( 0 != ( fileFilterMask & vxFileInfo.getFileType() ) )
			{
				LogMsg( LOG_VERBOSE, "PluginBaseFiles::fromGuiBrowseFiles sending file %d %s", fileNum, vxFileInfo.getFileName().c_str() );

				FileInfo fileInfo( m_Engine.getMyOnlineId(), vxFileInfo.getFileName(), vxFileInfo.getFileNameAndPath(), vxFileInfo.getFileLength(),
					vxFileInfo.getFileType(), VxGUID::nullVxGUID() );

				bool isShared = m_Engine.fromGuiGetIsFileShared( fileInfo );
				bool isInLibrary = m_Engine.fromGuiGetFileIsInLibrary( fileInfo );

				fileInfo.setIsInLibrary( isInLibrary );
				fileInfo.setIsSharedFile( isShared );

				IToGui::getIToGui().toGuiFileList( appInstId, fileInfo );
			}
			else
			{
				LogMsg( LOG_VERBOSE, "PluginBaseFiles::fromGuiBrowseFiles skip file type 0x%x because filter mask 0x%x file %d %s", 
					vxFileInfo.getFileType(), fileFilterMask, fileNum, vxFileInfo.getFileName().c_str() );
			}
		}
		else
		{
			if ( vxFileInfo.isExecutableFile() )
			{
				LogMsg( LOG_WARN, "PluginBaseFiles::fromGuiBrowseFiles skip executeable file %s", vxFileInfo.getFileName().c_str() );
			}
			else
			{
				LogMsg( LOG_WARN, "PluginBaseFiles::fromGuiBrowseFiles skip shortcut file %s", vxFileInfo.getFileName().c_str() );
			}
		}
	}

	IToGui::getIToGui().toGuiFileListCompleted(appInstId);
	return isPluginEnabled();
}

//============================================================================
bool PluginBaseFiles::fromGuiSetFileIsShared( FileInfo& fileInfo, bool isShared )
{
	return m_Engine.getAssetMgr().fromGuiSetFileIsShared( fileInfo, isShared );
}

//============================================================================
bool PluginBaseFiles::fromGuiSetFileIsShared( std::string& fileName, bool isShared )
{
	return m_Engine.getAssetMgr().fromGuiSetFileIsShared( fileName, isShared );
}

//============================================================================
bool PluginBaseFiles::fromGuiGetFileIsShared( FileInfo& fileInfo )
{
	return isFileShared( fileInfo.getFileNameAndPath() );
}

//============================================================================
bool PluginBaseFiles::fromGuiGetIsFileShared( std::string& fileNameAndPath )
{
	return isFileShared( fileNameAndPath );
}

//============================================================================
bool PluginBaseFiles::isFileShared( std::string& fileNameAndPath )
{
	return m_Engine.getAssetMgr().fromGuiGetFileIsShared( fileNameAndPath );
}

//============================================================================
// returns -1 if unknown else percent downloaded
int PluginBaseFiles::fromGuiGetFileDownloadState( uint8_t* fileHashId )
{
	return -1;
	int result = m_FileInfoMgr.fromGuiGetFileDownloadState( fileHashId );
	if( -1 == result )
	{
		result = m_FileInfoXferMgr.fromGuiGetFileDownloadState( fileHashId );
	}

	return result;
}

//============================================================================
bool PluginBaseFiles::fromGuiQueryFileHash( FileInfo& fileInfo )
{
	return m_FileInfoMgr.fromGuiQueryFileHash( fileInfo );
}

//============================================================================
void PluginBaseFiles::fromGuiFileHashGenerated( std::string& fileName, int64_t fileLen, VxSha1Hash& fileHash )
{
	m_FileInfoMgr.fromGuiFileHashGenerated( fileName, fileLen, fileHash );
}

//============================================================================
bool PluginBaseFiles::isServingFiles( void )
{
	return ( m_MyIdent->hasSharedFiles() && isPluginEnabled() );
}

//============================================================================
void PluginBaseFiles::deleteFile( std::string fileNameAndPath, bool shredFile )
{
	m_Engine.getAssetMgr().deleteFile( fileNameAndPath, shredFile );
}

//============================================================================
void PluginBaseFiles::onSharedFilesUpdated( uint16_t u16FileTypes )
{
	if( m_MyIdent->getSharedFileTypes() != u16FileTypes )
	{
		m_Engine.lockAnnouncePktAccess();
		m_MyIdent->setSharedFileTypes( (uint8_t)u16FileTypes );
		m_Engine.unlockAnnouncePktAccess();
		m_Engine.doPktAnnHasChanged( false );
	}
}

//============================================================================
//! user wants to send offer to friend.. return false if cannot connect
bool PluginBaseFiles::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	return m_FileInfoXferMgr.fromGuiMakePluginOffer( onlineId, offerInfo );
}

//============================================================================
bool PluginBaseFiles::fromGuiOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	return m_FileInfoXferMgr.fromGuiOfferReply( onlineId, offerInfo );
}

//============================================================================
EXferError PluginBaseFiles::fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo )
{
	return m_FileInfoXferMgr.fromGuiFileXferControl( onlineId, xferAction, fileInfo );
}

//============================================================================
void PluginBaseFiles::onPktPluginOfferReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktPluginOfferReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileGetReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileGetReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileSendReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileSendReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileChunkReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileChunkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileGetCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileGetCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileSendCompleteReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileSendCompleteReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileXferCancel( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileXferCancel( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFindFileReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFindFileReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFindFileReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFindFileReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileListReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileListReq( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileListReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoXferMgr.onPktFileListReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::onPktFileShareErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//m_FileInfoXferMgr.onPktFileShareErr( sktBase, pktHdr, netIdent );
}

//============================================================================
void PluginBaseFiles::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	m_PluginSessionMgr.replaceConnection( netIdent, poOldSkt, poNewSkt );
}

//============================================================================
void PluginBaseFiles::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onConnectionLost( sktBase );
}

//============================================================================
void PluginBaseFiles::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	m_PluginSessionMgr.onContactWentOffline( netIdent, sktBase );
}

//============================================================================
void PluginBaseFiles::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	m_PluginSessionMgr.onContactOnlineStatusChange( connectId, isOnline );
}

//============================================================================
void PluginBaseFiles::onPktFileInfoInfoReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//m_Engine.getFileInfoListMgr().onPktFileInfoInfoReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseFiles::onPktFileInfoInfoReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//m_Engine.getFileInfoListMgr().onPktFileInfoInfoReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseFiles::onPktFileInfoAnnReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//m_Engine.getFileInfoListMgr().onPktFileInfoAnnReq( sktBase, pktHdr, netIdent, getCommAccessState( netIdent ), this );
}

//============================================================================
void PluginBaseFiles::onPktFileInfoAnnReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	//m_Engine.getFileInfoListMgr().onPktFileInfoAnnReply( sktBase, pktHdr, netIdent, this );
}

//============================================================================
void PluginBaseFiles::onPktFileInfoSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "PluginBaseFiles::onPktFileInfoSearchReq rxed" );

	EPluginAccess pluginAccess = getPluginAccessState( netIdent );
	ECommErr commErr = getCommAccessState( netIdent );

	PktFileInfoSearchReq* pktReq = ( PktFileInfoSearchReq* )pktHdr;
	if( pktReq && pktReq->isValidPktPrefix() )
	{
		uint8_t searchFileTypes = pktReq->getSearchFileTypes();
		PktBlobEntry& blobEntry = pktReq->getBlobEntry();
		blobEntry.resetRead();
		std::string searchText;
		pktReq->getSearchText( searchText );
		if( !searchText.empty() || searchFileTypes || ePluginTypeAboutMePageServer == getPluginType() || ePluginTypeStoryboardServer == getPluginType() )
		{
			PktFileInfoSearchReply pktReply;
			pktReply.setSearchSessionId( pktReq->getSearchSessionId() );
			pktReply.setHostOnlineId( pktReq->getHostOnlineId() );
			pktReply.setSearchFileTypes( searchFileTypes );
			pktReply.setSearchText( searchText );
			
			pktReply.setAccessState( pluginAccess );
			pktReply.setCommError( commErr );
			if( ePluginAccessOk == pluginAccess && eCommErrNone == commErr )
			{
				if( !searchText.empty() && searchText.size() < FileInfo::FILE_INFO_SHORTEST_SEARCH_TEXT_LEN )
				{
					LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseFiles::onPktFileInfoSearchReq search text too short" );
					pktReply.setCommError( eCommErrSearchTextToShort );
					VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReq to short search text" );
				}
				else if( searchText.size() > FileInfo::FILE_INFO_LONGEST_SEARCH_TEXT_LEN )
				{
					LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseFiles::onPktFileInfoSearchReq search text too long" );
					pktReply.setCommError( eCommErrSearchTextToLong );
					VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReq to long search text" );
				}
				else
				{
					ECommErr searchErr = m_FileInfoMgr.searchRequest( pktReply, pktReq->getSpecificAssetId(), searchText, searchFileTypes, sktBase, pktReq->getSrcOnlineId() );
					pktReply.setCommError( searchErr );
				}
			}
			else
			{
				LogModule( eLogHostSearch, LOG_DEBUG, "PluginBaseFiles::onPktFileInfoSearchReq service not enabled" );
				pktReply.setCommError( eCommErrPluginNotEnabled );
			}

			pktReply.calcPktLen();
			if( !txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply ) )
			{
				LogModule( eLogHostSearch, LOG_VERBOSE, "PluginBaseFiles::onPktFileInfoSearchReq failed send search reply" );
			}
		}
		else
		{
			LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseHostService invalid search packet" );
			VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReply invalid search text" );
		}	
	}
	else
	{
		VxReportHack( eHackerLevelSevere, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReq invalid ptk" );
	}
}

//============================================================================
void PluginBaseFiles::onPktFileInfoSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileInfoSearchReply* pktReply = ( PktFileInfoSearchReply* )pktHdr;

	if( pktReply && pktReply->isValidPktPrefix() )
	{
		uint8_t searchFileTypes = pktReply->getSearchFileTypes();
		PktBlobEntry& blobEntry = pktReply->getBlobEntry();
		blobEntry.resetRead();
		std::string searchText;
		if( pktReply->getCommError() == eCommErrNone )
		{
			if( pktReply->getSearchText( searchText ) || searchFileTypes )
			{
				if( updateFromFileInfoSearchBlob( pktReply->getSearchSessionId(), pktReply->getHostOnlineId(), sktBase, pktReply->getSrcOnlineId(), pktReply->getBlobEntry(), pktReply->getFileInfoCountThisPkt() ) )
				{
					if( pktReply->getMoreFileInfosExist() )
					{
						if( !requestMoreFileInfoFromServer( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), pktReply->getNextSearchAssetId(), searchText ) )
						{
							fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrUserOffline);
						}
					}
					else
					{
						fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrNone );
					}
				}
				else
				{
					fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrInvalidParam );
				}
			}
			else
			{
				logCommError( eCommErrInvalidPkt, "PktFileInfoSearchReply", sktBase, netIdent );
				fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrInvalidPkt );
				VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReply invalid search text" );
			}
		}
		else
		{
			logCommError( pktReply->getCommError(), "PktFileInfoSearchReply", sktBase, netIdent );
			fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), pktReply->getCommError() );
		}
	}
	else
	{
		VxGUID nullGuid;
		fileInfoSearchCompleted( nullGuid, sktBase, pktReply->getSrcOnlineId(), eCommErrInvalidPkt );
		VxReportHack( eHackerLevelSevere, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReply invalid ptk" );
	}
}

//============================================================================
void PluginBaseFiles::onPktFileInfoMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileInfoMoreReq* pktReq = ( PktFileInfoMoreReq* )pktHdr;
	if( pktReq && pktReq->isValidPktPrefix() )
	{
		uint8_t searchFileTypes = pktReq->getSearchFileTypes();
		PktBlobEntry& blobEntry = pktReq->getBlobEntry();
		blobEntry.resetRead();
		std::string searchText;
		ECommErr commErr = getCommAccessState( netIdent );
		PktFileInfoMoreReply pktReply;
		pktReply.setCommError( commErr );
		if( pktReq->getSearchText( searchText ) || searchFileTypes )
		{
			pktReply.setSearchFileTypes( searchFileTypes );
			pktReply.setSearchText( searchText );
			EHostType hostType = pktReq->getHostType();
			pktReply.setHostType( hostType );
			VxGUID nextSearchOnlineId = pktReq->getNextSearchAssetId();
			pktReply.setSearchSessionId( pktReq->getSearchSessionId() );

			if( eCommErrNone == commErr )
			{				
				if( !searchText.empty() && searchText.size() < FileInfo::FILE_INFO_SHORTEST_SEARCH_TEXT_LEN )
				{
					LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseFiles::onPktFileInfoMoreReq search text too short" );
					pktReply.setCommError( eCommErrSearchTextToShort );
					VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoMoreReq to short search text" );
				}
				else if( searchText.size() > FileInfo::FILE_INFO_LONGEST_SEARCH_TEXT_LEN )
				{
					LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseFiles::onPktFileInfoMoreReq search text too long" );
					pktReply.setCommError( eCommErrSearchTextToLong );
					VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoMoreReq to long search text" );
				}
				else
				{
					if( nextSearchOnlineId.isValid() )
					{
						ECommErr searchErr = m_FileInfoMgr.searchMoreRequest( pktReply, nextSearchOnlineId, searchText, searchFileTypes, sktBase, pktReq->getSrcOnlineId() );
						pktReply.setCommError( searchErr );
					}
					else
					{
						LogModule( eLogHostSearch, LOG_ERROR, "PluginBaseFiles::onPktFileInfoMoreReq search text too long" );
						pktReply.setCommError( eCommErrInvalidParam );
						VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoMoreReq to long search text" );
					}
				}
			}

			pktReply.calcPktLen();
			txPacket( pktReq->getSrcOnlineId(), sktBase, &pktReply);
		}
		else
		{
			VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoMoreReq invalid search text" );
		}
	}
	else
	{
		VxReportHack( eHackerLevelSevere, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoMoreReq invalid ptk" );
	}
}

//============================================================================
void PluginBaseFiles::onPktFileInfoMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktFileInfoMoreReply* pktReply = ( PktFileInfoMoreReply* )pktHdr;
	if( pktReply && pktReply->isValidPktPrefix() )
	{
		std::string searchStr;
		if( pktReply->getCommError() )
		{
			logCommError( pktReply->getCommError(), "PluginBaseFiles::onPktFileInfoMoreReply", sktBase, netIdent );
			fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), pktReply->getCommError() );
		}
		else if( pktReply->getSearchText( searchStr ) )
		{
			VxGUID hostOnlineId = pktReply->getDestOnlineId();
			updateFromFileInfoSearchBlob( pktReply->getSearchSessionId(), hostOnlineId, sktBase, pktReply->getSrcOnlineId(), pktReply->getBlobEntry(), pktReply->getFileInfoCountThisPkt() );
			if( pktReply->getMoreFileInfosExist() )
			{
				if( !requestMoreFileInfoFromServer( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), pktReply->getNextSearchAssetId(), searchStr ) )
				{
					fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrUserOffline );
				}
			}
			else
			{
				fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrNone );
			}
		}
		else
		{
			fileInfoSearchCompleted( pktReply->getSearchSessionId(), sktBase, pktReply->getSrcOnlineId(), eCommErrInvalidPkt );
			VxReportHack( eHackerLevelSuspicious, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReply invalid search text" );
		}
	}
	else
	{
		VxGUID nullGuid;
		fileInfoSearchCompleted( nullGuid, sktBase, pktReply->getSrcOnlineId(), eCommErrInvalidPkt );
		VxReportHack( eHackerLevelSevere, eHackerReasonInvalidPkt, sktBase, "PluginBaseFiles::onPktFileInfoSearchReply invalid ptk" );
	}
}

//============================================================================
bool PluginBaseFiles::updateFromFileInfoSearchBlob( VxGUID& searchSessionId, VxGUID& hostOnlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID srcOnlineId, PktBlobEntry& blobEntry, int fileInfoCount )
{
	// assumes blobEntry.resetRead(); has been called and any procceeding values like search text has been extracted
	bool result{ true };
	for( int i = 0; i < fileInfoCount; i++ )
	{
		FileInfo fileInfo;
		if( fileInfo.extractFromBlob( blobEntry ) )
		{
			if( !fileInfo.m_OnlineId.isValid() )
			{
				fileInfo.m_OnlineId = srcOnlineId;
			}

			result &= fileInfoSearchResult( searchSessionId, sktBase, srcOnlineId, fileInfo );
			if( !result )
			{
				LogMsg( LOG_ERROR, "FileInfoListMgr::%s fileInfoSearchResult failed ", __func__ );
				break;
			}
		}
		else
		{
			LogMsg( LOG_ERROR, "FileInfoListMgr::%s Could not extract", __func__ );
			result = false;
			break;
		}
	}

	return result;
}

//============================================================================
bool PluginBaseFiles::requestMoreFileInfoFromServer(  VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, VxGUID& nextFileInfoAssetId, std::string& searchText )
{
	PktFileInfoMoreReq pktReq;
	pktReq.setSearchSessionId( searchSessionId );
	pktReq.setNextSearchAssetId( nextFileInfoAssetId );
	pktReq.setSearchText( searchText );
	pktReq.calcPktLen();
	return txPacket( onlineId, sktBase, &pktReq);
}

//============================================================================
ECommErr PluginBaseFiles::searchRequest( PktFileInfoSearchReply& pktReply, VxGUID& specificAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	return m_FileInfoMgr.searchRequest( pktReply, specificAssetId, searchStr, searchFileTypes, sktBase, onlineId );
}

//============================================================================
ECommErr PluginBaseFiles::searchMoreRequest( PktFileInfoMoreReply& pktReply, VxGUID& nextFileAssetId, std::string& searchStr, uint8_t searchFileTypes, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	return m_FileInfoMgr.searchMoreRequest( pktReply, nextFileAssetId, searchStr, searchFileTypes, sktBase, onlineId );
}

//============================================================================
void PluginBaseFiles::sendFileSearchResultToGui( VxGUID& searchSessionId, VxGUID& onlineId, FileInfo& fileInfo )
{
	m_FileInfoMgr.sendFileSearchResultToGui( searchSessionId, onlineId, fileInfo );
}

//============================================================================
void PluginBaseFiles::wantFileXferCallback( FileXferCallback* callback, bool wantCallback )
{
	m_FileInfoXferMgr.wantFileXferCallback( callback, wantCallback );
}

//============================================================================
bool PluginBaseFiles::ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb )
{
	return m_ThumbXferMgr.requestPluginThumb( sktBase, netIdent, thumbId, tmpThumb );
}
