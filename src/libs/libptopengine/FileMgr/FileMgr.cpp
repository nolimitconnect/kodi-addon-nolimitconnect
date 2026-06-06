//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileIsTypeFunctions.h>

#include <filesystem>


//============================================================================
void FileMgr::fromGuiScanFolderForMedia( P2PEngine& engine, VxGUID& appInstId, std::string dirToScan, uint8_t fileTypeFilter )
{
	m_AppInstId = appInstId;
	m_WasCanceled = false;

	int fileCnt{ 0 };

	for( const auto& entry : std::filesystem::recursive_directory_iterator( dirToScan ) ) 
	{
		if( m_WasCanceled )
		{
			break;
		}

		if( entry.is_regular_file() )
		{
			std::string ext = entry.path().extension().string();

			uint8_t fileType = VxFileExtensionToFileTypeFlag( ext.c_str() );
			if( fileType & fileTypeFilter )
			{
				std::string fileNameAndPath = entry.path().string();
				AssetBaseInfo* assetInfo = engine.getAssetMgr().findAsset( fileNameAndPath );
				if( assetInfo )
				{
					if( !assetInfo->isInLibrary() )
					{
						FileInfo fileInfo = assetInfo->getFileInfo();
						fileCnt++;
						engine.getToGui().toGuiFolderScan( m_AppInstId, fileInfo );
						LogMsg( LOG_VERBOSE, "FileMgr::%s existing asset file %s", __func__, fileNameAndPath.c_str() );
						waitForReceiveAck();
					}
				}
				else
				{
					uint64_t fileLen = VxFileUtil::fileExists( fileNameAndPath.c_str() );
					if( fileLen )
					{
						std::string	filePath;
						std::string	justFileName;
						//! separate Path and file name into separate strings
						if( 0 == VxFileUtil::seperatePathAndFile( fileNameAndPath, filePath, justFileName ) )
						{
							AssetBaseInfo* newAsset = engine.getAssetMgr().addAssetFile( (EAssetType)fileType, justFileName.c_str(), fileNameAndPath.c_str(), fileLen );
							if( newAsset )
							{
								fileCnt++;
								FileInfo fileInfo = newAsset->getFileInfo();
								engine.getToGui().toGuiFolderScan( m_AppInstId, fileInfo );
								waitForReceiveAck();
							}
						} 
						else
						{
							LogMsg( LOG_ERROR, "FileMgr::%s %s failed to separate name and path", __func__, fileNameAndPath.c_str() );
						}
					}
					else
					{
						LogMsg( LOG_ERROR, "FileMgr::%s %s does not exist", __func__, fileNameAndPath.c_str() );
					}
				}
			}
		}

		if( m_WasCanceled )
		{
			break;
		}
	}

	engine.getToGui().toGuiFolderScanCompleted( m_AppInstId, m_WasCanceled );
}

//============================================================================
void FileMgr::fromGuiScanFolderCancel( VxGUID& appInstId )
{
	if( appInstId == m_AppInstId )
	{
		m_WasCanceled = true;
		m_RxSemaphore.signal();
	}
}

//============================================================================
void FileMgr::fromGuiScanItemReceived( VxGUID& appInstId )
{
	if( appInstId == m_AppInstId )
	{
		m_RxSemaphore.signal();
	}
}

//============================================================================
void FileMgr::waitForReceiveAck( void )
{
	m_RxSemaphore.wait();
}