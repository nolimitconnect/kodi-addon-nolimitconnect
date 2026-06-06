//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginFileShareClient.h"
#include "PluginMgr.h"

#include <Plugins/FileInfo.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <NetLib/VxSktBase.h>

//============================================================================
PluginFileShareClient::PluginFileShareClient( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseFilesClient( engine, pluginMgr, myIdent, pluginType, "FileShareClient.db3" )
{
	setPluginType( ePluginTypeFileShareClient );
}

//============================================================================
void PluginFileShareClient::onAfterUserLogOnThreaded( void )
{
	m_RootFileFolder = VxGetDownloadsDirectory();
	getFileInfoMgr().setRootFolder( m_RootFileFolder );

	getFileInfoMgr().onAfterUserLogOnThreaded();
}

//============================================================================
void PluginFileShareClient::onLoadedFilesReady( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes )
{
	if( LogEnabled( eLogStartup ) )LogMsg( LOG_VERBOSE, "PluginFileShareClient::onLoadedFilesReady" );
	checkIsWebPageClientReady();
}

//============================================================================
void PluginFileShareClient::onFilesChanged( int64_t lastFileUpdateTime, int64_t totalBytes, uint16_t fileTypes )
{
	checkIsWebPageClientReady();
}

//============================================================================
void PluginFileShareClient::onFileDownloadStart( bool started, VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& lclSessionId, std::string fileName, VxGUID& assetId )
{
	EPluginMsgType pluginMsgType = started ? ePluginMsgDownloading : ePluginMsgDownloadFailed;
	m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, pluginMsgType, fileName.c_str() );
}

//============================================================================
bool PluginFileShareClient::onFileDownloadComplete( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& lclSessionId, std::string& fileNameAndPath, VxGUID& assetId, VxSha1Hash& sha11Hash )
{
	std::string filePath;
	std::string fileName;
	VxFileUtil::seperatePathAndFile(fileNameAndPath,	// path and file name
                                        filePath,		// return path to file
                                        fileName );	// return file name
	bool result = onlineId.isValid() && sktBase && lclSessionId.isValid() && !fileNameAndPath.empty() && assetId.isValid() && sha11Hash.isHashValid();
	if( result )
	{
		result = false;
		// move from in progress to completed
		lockInProgressFileList();
		for( auto iter = m_InProgressFileInfoList.begin(); iter != m_InProgressFileInfoList.end(); ++iter )
		{
			FileInfo& fileInfo = *iter;
			if( fileInfo.getAssetId() == assetId && fileInfo.getFileHashId() == sha11Hash )
			{
				lockCompletedFileList();
				fileInfo.setFileName( fileName );
				fileInfo.setFileNameAndPath( fileNameAndPath );

				m_CompletedFileInfoList.emplace_back( fileInfo );
				m_InProgressFileInfoList.erase( iter );
				result = true;
				unlockCompletedFileList();
				break;
			}
		}

		unlockInProgressFileList();

		if( !result && !m_SearchFileInfoList.empty() )
		{
			FileInfo foundFileInfo;
			lockSearchFileList();
			// may have been started from a search result
			for( auto iter = m_SearchFileInfoList.begin(); iter != m_SearchFileInfoList.end(); ++iter )
			{
				FileInfo& curFileInfo = *iter;
				if( curFileInfo.getAssetId() == assetId && curFileInfo.getFileHashId() == sha11Hash )
				{
					lockCompletedFileList();
					curFileInfo.setFileName( fileName );
					curFileInfo.setFileNameAndPath( fileNameAndPath );
					foundFileInfo = curFileInfo;
					m_SearchFileInfoList.erase( iter );
					result = true;
					unlockCompletedFileList();
					break;
				}
			}

			unlockSearchFileList();
			if( result )
			{
				// all done
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgDownloadComplete, foundFileInfo.getFileNameAndPath().c_str() );
			}
			else
			{
				// failed to find the web index file in downloaded files
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgDownloadFailed );
			}

			// do not start another
			result = false;
		}
		
		if( result )
		{
			result = false;
			if( m_SearchFileInfoList.empty() && m_InProgressFileInfoList.empty() )
			{
				// find the index file and send to gui
				FileInfo indexFileInfo;
				lockCompletedFileList();
				for( auto iter = m_CompletedFileInfoList.begin(); iter != m_CompletedFileInfoList.end(); ++iter )
				{
					FileInfo& fileInfo = *iter;
					if( fileInfo.getFileName() == getWebIndexFileName() )
					{
						indexFileInfo = fileInfo;
						result = true;
						break;
					}
				}

				unlockCompletedFileList();
				if( result )
				{
					// all done
					m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgDownloadComplete, indexFileInfo.getFileNameAndPath().c_str() );
				}
				else
				{
					// failed to find the web index file in downloaded files
					m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgDownloadFailed );
				}
			}
			else
			{
				result = startDownload( lclSessionId, sktBase, onlineId );
			}
		}
		else
		{
			m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgDownloadFailed );
		}
	}
	else
	{
		m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgInvalidParam );
	}

	return result;
}

//============================================================================
void PluginFileShareClient::checkIsWebPageClientReady( void )
{
	setIsWebPageClientReady( getFileInfoMgr().getIsInitialized() );
}

//============================================================================
void PluginFileShareClient::setIsWebPageClientReady( bool isReady )
{
	if( m_WebPageClientReady != isReady )
	{
		m_WebPageClientReady = isReady;
		onWebPageClientReady( isReady );
	}
}

//============================================================================
void PluginFileShareClient::onWebPageClientReady( bool isReady )
{

}

//============================================================================
std::string	PluginFileShareClient::getIncompleteFileXferDirectory( VxGUID& onlineId )
{
	std::string incompleteDir{ "" };
	if( onlineId.isValid() )
	{
		incompleteDir = m_RootFileFolder + onlineId.toHexString().c_str() + "/";
		VxFileUtil::makeDirectory( incompleteDir.c_str() );
		if( VxFileUtil::directoryExists( incompleteDir.c_str() ) )
		{
			int64_t diskFreeSpace = VxFileUtil::getDiskFreeSpace( incompleteDir.c_str() );

			if( diskFreeSpace && diskFreeSpace < VxFileUtil::SIZE_1GB )
			{
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgLowDiskSpace, "" );
			}
		}
		else
		{
			m_Engine.getToGui().toGuiPluginMsg( getPluginType(), onlineId, ePluginMsgPermissionError, incompleteDir.c_str() );
		}
	}

	return incompleteDir;
}

//============================================================================
bool PluginFileShareClient::fromGuiDownloadWebPage( EWebPageType webPageType, VxGUID& onlineId )
{
	bool result{ false };
	if( (eWebPageTypeAboutMe == webPageType || eWebPageTypeStoryboard == webPageType ) && onlineId.isValid() )
	{
		m_HisOnlineId = onlineId;
		m_DownloadFileFolder = getIncompleteFileXferDirectory( onlineId );
		if( VxFileUtil::directoryExists( m_DownloadFileFolder.c_str() ) )
		{
			// must clear any previous files or download will make duplicates filename_1 filename_2 etc
			VxFileUtil::deleteFilesInFolder( m_DownloadFileFolder, true );
			m_WebPageIndexFile = m_DownloadFileFolder + getWebIndexFileName();
			int64_t diskFreeSpace = VxFileUtil::getDiskFreeSpace( m_DownloadFileFolder.c_str() );

			if( diskFreeSpace && diskFreeSpace < VxFileUtil::SIZE_1GB )
			{
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgLowDiskSpace, "" );
			}
			else
			{
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgConnecting, "" );
				result = connectForWebPageDownload( onlineId );
			}
		}
		else
		{
			m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgPermissionError, m_DownloadFileFolder.c_str() );
		}
	}
	else
	{
		LogMsg( LOG_VERBOSE, "PluginFileShareClient::fromGuiDownloadWebPage invalid EWebPageType" );
	}

	return result;
}

//============================================================================
bool PluginFileShareClient::fromGuiCancelWebPage( EWebPageType webPageType, VxGUID& onlineId )
{
	bool result{ false };
	if( eWebPageTypeAboutMe == webPageType )
	{
		cancelDownload();
		m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgCanceled, "" );

	}
	else
	{
		LogMsg( LOG_VERBOSE, "PluginFileShareClient::fromGuiCancelWebPage invalid EWebPageType" );
	}

	return result;
}

//============================================================================
bool PluginFileShareClient::onConnectForFileListDownload( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	lockSearchFileList();
	m_SearchFileInfoList.clear();
	unlockSearchFileList();
	return PluginBaseFilesClient::onConnectForFileListDownload( sktBase, onlineId );
}

//============================================================================
bool PluginFileShareClient::fileInfoSearchResult( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID srcOnlineId, FileInfo& fileInfo )
{
	bool result = fileInfo.isValid( true );
	if( result )
	{
		fileInfo.setFileNameAndPath( m_DownloadFileFolder + fileInfo.getFileName() );
		lockSearchFileList();
		m_SearchFileInfoList.emplace_back( fileInfo );
		unlockSearchFileList();
		if( fileInfo.m_ThumbId.isValid() )
		{
			// query media thumnail if we do not have it
			m_Engine.getThumbMgr().queryMediaThumbIfNeeded( sktBase, srcOnlineId, getPluginType(), fileInfo.m_ThumbId );
		}
		
		sendFileSearchResultToGui( searchSessionId, srcOnlineId, fileInfo );
	}

	return result;
}

//============================================================================
void PluginFileShareClient::fileInfoSearchCompleted( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, ECommErr commErr )
{
	if( commErr == eCommErrNone )
	{
		LogMsg( LOG_VERBOSE, "PluginFileShareClient::fileInfoSearchCompleted with no errors" );

		m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgRetrieveInfoComplete, StdStringFormat( " %d", m_SearchFileInfoList.size() ).c_str() );
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginFileShareClient::fileInfoSearchCompleted with error %s from %s", DescribeCommError( commErr ), sktBase->describeSktConnection().c_str() );
		m_Engine.getToGui().toGuiPluginCommError( getPluginType(), m_HisOnlineId, ePluginMsgRetrieveInfoFailed, commErr );
	}
}

//============================================================================
void PluginFileShareClient::cancelDownload( void )
{
	lockSearchFileList();
	for( auto& fileInfo : m_SearchFileInfoList )
	{
		m_FileInfoMgr.cancelAndDelete( fileInfo.getAssetId() );
	}

	m_SearchFileInfoList.clear();
	unlockSearchFileList();

	lockInProgressFileList();
	for( auto& fileInfo : m_InProgressFileInfoList )
	{
		m_FileInfoMgr.cancelAndDelete( fileInfo.getAssetId() );
	}

	m_InProgressFileInfoList.clear();
	unlockInProgressFileList();

	lockCompletedFileList();
	for( auto& fileInfo : m_InProgressFileInfoList )
	{
		m_FileInfoMgr.cancelAndDelete( fileInfo.getAssetId() );
	}

	m_InProgressFileInfoList.clear();
	unlockCompletedFileList();

	// clear out files on cancel
	VxFileUtil::deleteFilesInFolder( m_DownloadFileFolder, true );
}

//============================================================================
bool PluginFileShareClient::startDownload( VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	bool result{ false };
	lockSearchFileList();
	for( auto iter = m_SearchFileInfoList.begin(); iter != m_SearchFileInfoList.end(); ++iter )
	{
		FileInfo& fileInfo = *iter;
		lockInProgressFileList();
		VxGUID xferSessionId = fileInfo.initializeNewXferSessionId();

		m_InProgressFileInfoList.push_back( fileInfo );
		if( m_FileInfoMgr.startDownload( *iter, xferSessionId, sktBase, onlineId ) )
		{
			result = true;
			m_SearchFileInfoList.erase( iter );
		}
		else
		{
			m_InProgressFileInfoList.pop_back();
		}

		unlockInProgressFileList();
		break;
	}

	unlockSearchFileList();
	return result;
}

//============================================================================
bool PluginFileShareClient::fromGuiDownloadFileList( VxGUID& onlineId, VxGUID& sessionId, uint8_t fileTypes )
{
	bool result{ false };
	m_HisOnlineId = onlineId;
	m_SearchSessionId = sessionId;
	m_LclSessionId = sessionId;
	if( m_HisOnlineId == m_Engine.getMyOnlineId() )
	{
		std::vector<AssetBaseInfo> sharedFiles;
		m_Engine.getAssetMgr().getSharedFiles( sharedFiles );
		int resultCnt{ 0 };
		for( auto& assetInfo : sharedFiles )
		{
			FileInfo fileInfo = assetInfo.getFileInfo();
			if( fileInfo.getFileType() & fileTypes )
			{
				sendFileSearchResultToGui( m_SearchSessionId, m_HisOnlineId, fileInfo );
				resultCnt++;
			}
		}
		
		m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgRetrieveInfoComplete, StdStringFormat( " %d", resultCnt ).c_str() );
		return true;
	}
	else
	{
		m_DownloadFileFolder = getIncompleteFileXferDirectory( onlineId );
		if( VxFileUtil::directoryExists( m_DownloadFileFolder.c_str() ) )
		{
			int64_t diskFreeSpace = VxFileUtil::getDiskFreeSpace( m_DownloadFileFolder.c_str() );

			if( diskFreeSpace && diskFreeSpace < VxFileUtil::SIZE_1GB )
			{
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgLowDiskSpace, "" );
			}
			else
			{
				m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgConnecting, "" );
				if( !fileTypes )
				{
					fileTypes = VXFILE_TYPE_ALLNOTEXE;
				}

				setSearchFileTypes( fileTypes );
				result = connectForFileListDownload( onlineId );
			}
		}
		else
		{
			m_Engine.getToGui().toGuiPluginMsg( getPluginType(), m_HisOnlineId, ePluginMsgPermissionError, m_DownloadFileFolder.c_str() );
		}
	}

	return result;
}

//============================================================================
bool PluginFileShareClient::startStream( std::shared_ptr<VxSktBase>& sktBase, AssetBaseInfo& assetInfo, VxGUID lclSessionId )
{
	FileInfo fileInfo = assetInfo.getFileInfo();
	fileInfo.setIsStream( true );
	fileInfo.setXferSessionId( lclSessionId );

	VxGUID destUserId = assetInfo.getDestUserId();

	return m_FileInfoMgr.startDownload( fileInfo, lclSessionId, sktBase, destUserId );
}

//============================================================================
bool PluginFileShareClient::fromGuiDownloadFileListCancel( VxGUID& onlineId, VxGUID& sessionId )
{

	return false;
}

//============================================================================
void PluginFileShareClient::onPktStreamCtrlReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	m_FileInfoMgr.getFileInfoXferMgr().onPktStreamCtrlReply( sktBase, pktHdr, netIdent );
}
