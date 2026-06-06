//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "config_corelib.h"

#include "VxFileLists.h"
#include "VxFileUtil.h"
#include "VxParse.h"
#include "VxFileIsTypeFunctions.h"
#include "VxDebug.h"

#include <string.h>

#ifndef TARGET_OS_WINDOWS
	#include <dirent.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h> 
#else
    #pragma warning( disable : 4244)
#endif

//============================================================================
// VxFileList
//============================================================================
std::string	VxFileList::getFileAtIndex( int listIdx )
{
	if( listIdx >= 0 && listIdx < m_FileList.size() )
	{
		return m_FileList[listIdx];
	}
	else
	{
		return "";
	}
}

//============================================================================
bool VxFileList::addFileToFront( std::string fileName, bool checkIfFileExists )
{
	if( fileName.empty() )
	{
		return false;
	}

	if( checkIfFileExists && !doesFileExistInDiskStorage( fileName ) )
	{
		return false;
	}

	for( auto file : m_FileList )
	{
		if( file == fileName )
		{
			// already exists
			return true;
		}
	}

	m_FileList.insert( m_FileList.begin(), fileName );
	if( m_ListLimit && m_FileList.size() > m_ListLimit )
	{
		return removeFileFromBack();
	}

	return true;
}

//============================================================================
bool VxFileList::removeFileFromFront( void )
{
	if( m_FileList.size() )
	{
		m_FileList.erase( m_FileList.begin() );
		return true;
	}

	return false;
}

//============================================================================
bool VxFileList::addFileToBack( std::string fileName, bool checkIfFileExists )
{
	if( fileName.empty() )
	{
		return false;
	}

	if( checkIfFileExists && !doesFileExistInDiskStorage( fileName ) )
	{
		return false;
	}

	for( auto file : m_FileList )
	{
		if( file == fileName )
		{
			// already exists
			return true;
		}
	}

	m_FileList.emplace_back( fileName );

	if( m_ListLimit && m_FileList.size() > m_ListLimit )
	{
		return removeFileFromBack();
	}

	return true;
}

//============================================================================
bool VxFileList::removeFileFromBack( void )
{
	if( m_FileList.size() )
	{
		m_FileList.pop_back();
		return true;
	}

	return false;
}

//============================================================================
void VxFileList::removeFile( std::string fileName )
{
	for( auto iter = m_FileList.begin(); iter != m_FileList.end(); ++iter )
	{
		if( *iter == fileName )
		{
			m_FileList.erase( iter );
			break;
		}
	}
}

//============================================================================
bool VxFileList::doesFileExistInDiskStorage( std::string& fileName )
{
	return VxFileUtil::fileExists( fileName.c_str(), false ) != 0;
}

//============================================================================
void VxFileList::dumpToLog( uint32_t dbgLevel )
{
    int fileNum = 0;
    for( auto fileName : m_FileList )
    {
        fileNum++;
        LogMsg( dbgLevel, "VxFileList %d - %s", fileNum, fileName.c_str() );
    }
}


//============================================================================
// VxFileInfoList
//============================================================================
std::string	VxFileInfoList::getFileNameAtIndex( int listIdx )
{
    if( listIdx >= 0 && listIdx < m_FileInfoList.size() )
    {
        return m_FileInfoList[listIdx].getFileName();
    }
    else
    {
        return "";
    }
}

//============================================================================
std::string	VxFileInfoList::getFileNameAndPathAtIndex( int listIdx )
{
    if( listIdx >= 0 && listIdx < m_FileInfoList.size() )
    {
        return m_FileInfoList[listIdx].getFileNameAndPath();
    }
    else
    {
        return "";
    }
}

//============================================================================
std::string VxFileInfoList::getFileNameFromNameAndPath( std::string fileNameAndPath )
{
    for( auto iter = m_FileInfoList.begin(); iter != m_FileInfoList.end(); ++iter )
    {
        if( fileNameAndPath == iter->getFileNameAndPath() )
        {
            return iter->getFileName();
        }
    }

    return "";
}

//============================================================================
void VxFileInfoList::moveToTopOfList( std::string fileNameAndPath )
{
    VxFileInfoBase fileInfo;
    bool found{false};
    for( auto iter = m_FileInfoList.begin(); iter != m_FileInfoList.end(); ++iter )
    {
        if( fileNameAndPath == iter->getFileNameAndPath() )
        {
            found = true;
            fileInfo = *iter;
            m_FileInfoList.erase(iter);
            break;
        }
    }

    if(found)
    {
        m_FileInfoList.insert( m_FileInfoList.begin(), fileInfo );
    }
	else if( VxFileUtil::getFileInfo( fileNameAndPath.c_str(), fileInfo ) )
	{
		m_FileInfoList.insert( m_FileInfoList.begin(), fileInfo );
	}
	else
	{
		LogMsg( LOG_ERROR, "%s failed to get fileInfo" );
	}
}

//============================================================================
void VxFileInfoList::moveToTopOfList( VxFileInfoBase& fileInfo )
{
    std::string fileNameAndPath = fileInfo.getFileNameAndPath();
    bool found{false};
    for( auto iter = m_FileInfoList.begin(); iter != m_FileInfoList.end(); ++iter )
    {
        if( fileNameAndPath == iter->getFileNameAndPath() )
        {
            found = true;
            fileInfo = *iter;
            m_FileInfoList.erase(iter);
            break;
        }
    }

    m_FileInfoList.insert(m_FileInfoList.begin(), fileInfo);
}

//============================================================================
// VxFileFinder
//============================================================================
int VxFileFinder::FindFilesByExtension(	std::string				csPath,					//start path to search in
                                        std::vector<std::string> &acsExtensionList,		//Extensions
                                        std::vector<VxFileInfo> &aoFileList,			//return FileInfo in array
                                        bool					bRecurse,				//recurse subdirectories if true
                                        bool					bUseFilterListToExclude	//if true dont return files matching filter else return files that do
											)
{
	m_bAbort = false;
    VxFileInfo oCurFileNode;
    int rc = 0;
#ifdef TARGET_OS_WINDOWS
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW sFindData;
    if( 0 != csPath.size ( ) )
    {
        if ( '\\' == csPath[ csPath.size() - 1 ] )
        {
			std::string newPath = csPath + "*.*";
            oCurFileNode.setFileName( newPath.c_str() );
        }
        else
        {
			std::string newPath = csPath  + '\\' + "*.*";
			oCurFileNode.setFileName( newPath.c_str() );
        }

        hFind = FindFirstFileW( Utf8ToWide( oCurFileNode.getFileName() ).c_str(),
                                &sFindData );
        if( INVALID_HANDLE_VALUE != hFind )
        {
            do
            {
                if ( '\\' == csPath[ csPath.size( ) - 1 ] )
                {
					std::string newFile = csPath + WideToUtf8( sFindData.cFileName );
					oCurFileNode.setFileName( newFile.c_str() );
                }
                else
                {
                    oCurFileNode.setFileName( csPath + '\\' + WideToUtf8( sFindData.cFileName ) );
                }
                if ( ( 0 != (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) )
                {
				    if( bRecurse 
						&& ( false == VxFileUtil::isDotDotDirectory( sFindData.cFileName ) ) ) 
				    {
					    FindFilesByExtension(   oCurFileNode.getFileName(),
												acsExtensionList,
												aoFileList,
												bRecurse,								//recurse subdirectories if true
												bUseFilterListToExclude
												);
				    }
                }
			    else
			    {
					//include or exclude on the give list of extensions
					if( bUseFilterListToExclude )
					{
						if( ! HasSameExtension( oCurFileNode.getFileName(), acsExtensionList ) )
						{
							oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
							oCurFileNode.setFileLength( ((uint64_t)sFindData.nFileSizeHigh << 32) | sFindData.nFileSizeLow );
    						aoFileList.push_back( oCurFileNode );
						}
					}
					else
					{
						if( HasSameExtension( oCurFileNode.getFileName(), acsExtensionList ) )
						{
							oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
							oCurFileNode.setFileLength( ((uint64_t)sFindData.nFileSizeHigh << 32) | sFindData.nFileSizeLow );
							aoFileList.push_back( oCurFileNode );
						}
					}
			    }
            }
            while( ( false == m_bAbort ) &&
                   ( FindNextFileW( hFind,
                                   &sFindData ) ) );
            if( true == m_bAbort )
            {
                rc = ERROR_OPERATION_ABORTED;
            }
            FindClose ( hFind );
        }
    }
#else 
	// linux version of find files
	// find the files in the directory
	DIR *pDir;
	struct dirent *pFileEnt;
    if( VxFileUtil::directoryExists( csPath.c_str() ) )
	{
		//LogMsg( LOG_INFO, "FindFilesByExtension: directory %s exists.. opening dir\n", csPath.c_str() );
		//ok directory exists!
		if(!(NULL == (pDir = opendir( csPath.c_str() ))))
		{
			//pDir is open
			while( 0 != ( pFileEnt = readdir(pDir) ))
			{
				LogMsg( LOG_INFO, "FindFilesByExtension: found file %s\n", pFileEnt->d_name );
				if ( '/' == csPath[ csPath.size( ) - 1 ] )
				{
                    oCurFileNode.setFileName( csPath + pFileEnt->d_name );
				}
				else
				{
                    oCurFileNode.setFileName( csPath + '/' + pFileEnt->d_name );
				}
				struct stat oStat;
                if( 0 != stat( oCurFileNode.getFileName().c_str(), &oStat ) )
				{
					///ERROR how do we handle
                    LogMsg( LOG_INFO, "FindFilesByExtension: Unable to stat file %s\n", oCurFileNode.getFileName().c_str() );
					continue;
				}

				if( pFileEnt->d_type == DT_DIR )
				{
					if( bRecurse  
						&& ( ! VxFileUtil::isDotDotDirectory( pFileEnt->d_name ) ) )
					{
                        FindFilesByExtension(   oCurFileNode.getFileName(),
                                                acsExtensionList,
                                                aoFileList,
                                                bRecurse,					// recurse subdirectories if true
                                                bUseFilterListToExclude
                                                );
					}
				}
				else
				{
					// its a file
					//include or exclude on the give list of extensions
					if( bUseFilterListToExclude )
					{
                        if( ! HasSameExtension( oCurFileNode.getFileName(), acsExtensionList ) )
						{
                            oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
                            oCurFileNode.setFileLength( oStat.st_size );
							aoFileList.push_back( oCurFileNode );
						}
					}
					else
					{
                        if( HasSameExtension( oCurFileNode.getFileName(), acsExtensionList ) )
						{
                            oCurFileNode.setFileType(  VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
                            oCurFileNode.setFileLength( oStat.st_size );
							aoFileList.push_back( oCurFileNode );
						}
					}
				}
			}

			// end of listing
			// done with listing
			closedir(pDir);
			return 0;
		}
		else
		{
			LogMsg( LOG_INFO, "VxListSubDirectories: could not open directory %s \n", csPath.c_str() );
		}
	}
	else
	{
		LogMsg( LOG_INFO, "VxListSubDirectories: directory %s does not exist \n", csPath.c_str() );
	}

	return 0;
#endif // TARGET_OS_WINDOWS
    return rc;
}

//============================================================================
bool VxFileFinder::HasSameExtension(	std::string csCurrentNode,
										std::vector<std::string> &acsExtensionList )
{
    std::string csFilterExt;
	std::string csNodeExt;
	//LogMsg( LOG_INFO,  "TotalLen %d\n", iLen );
    int iExtIdx = StdStringReverseFind( csCurrentNode, '.' );
	//LogMsg( LOG_INFO,  "Index %d\n", iExtIdx );
	if( iExtIdx < 0 )
	{
		// could not find a .
		return false;
	}

	csNodeExt = StdStringRight( csCurrentNode, ((int)csCurrentNode.size ( ) - StdStringReverseFind( csCurrentNode, '.' )) );
	StdStringMakeUpper( csNodeExt );

	size_t iListCnt = acsExtensionList.size();
	for( unsigned int iIdx = 0; iIdx < iListCnt; iIdx++ )
	{
        csFilterExt = acsExtensionList[ iIdx ];
		StdStringMakeUpper( csFilterExt );

        if( csFilterExt == csNodeExt )
        {
            return true;
        }
    }
    return false;
}
//============================================================================
int VxFileFinder::FindFilesByName(	std::string csPath,	//start path to search in
										std::vector<std::string> &acsWildNameList,	//Name match strings with wildcards
										std::vector<VxFileInfo> &aoFileList,//return FileInfo in array
										bool bRecurse,								//recurse subdirectories if true
										bool bUseFilterListToExclude				//if true dont return files matching filter else return files that do
										)
{
	m_bAbort = false;
    VxFileInfo oCurFileNode;
    int rc = 0;

#ifdef TARGET_OS_WINDOWS
	// windows version of find files
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW sFindData;
    if( 0 != csPath.size ( ) )
    {
        if ( '\\' == csPath[ csPath.size() - 1 ] )
        {
            oCurFileNode.setFileName( csPath + "*.*" );
        }
        else
        {
            oCurFileNode.setFileName(  csPath + "\\*.*" );
        }
        hFind = FindFirstFileW( Utf8ToWide( oCurFileNode.getFileName() ).c_str(),
                                &sFindData );
        if( INVALID_HANDLE_VALUE != hFind )
        {
            do
            {
                if( '\\' == csPath[ csPath.size( ) - 1 ] )
                {
                    oCurFileNode.setFileName(  csPath + WideToUtf8( sFindData.cFileName ) );
                }
                else
                {
                    oCurFileNode.setFileName( csPath + '\\' + WideToUtf8( sFindData.cFileName ) );
                }
                if ( 0 != (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                {
				    if ( ( bRecurse ) 
						&& ( false == VxFileUtil::isDotDotDirectory( sFindData.cFileName ) ) )
				    {
					    FindFilesByName(	oCurFileNode.getFileName(),
											acsWildNameList,
											aoFileList,
											bRecurse,								//recurse subdirectories if true
											bUseFilterListToExclude
											);
				    }
                }
			    else
			    {
					//include or exclude on the given list of wild Name Match Strings
					if( bUseFilterListToExclude )
					{
						if( ! HasMatchingName( oCurFileNode.getFileName(), acsWildNameList ) )
						{
							oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
							oCurFileNode.setFileLength( ((uint64_t)sFindData.nFileSizeHigh << 32) | sFindData.nFileSizeLow );
                            VxFileUtil::makeForwardSlashPath( oCurFileNode.getFileName() );
    						aoFileList.push_back( oCurFileNode );
						}
					}
					else
					{
						if( HasMatchingName( oCurFileNode.getFileName(), acsWildNameList ) )
						{
							oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
							oCurFileNode.setFileLength( ((uint64_t)sFindData.nFileSizeHigh << 32) | sFindData.nFileSizeLow );
                            VxFileUtil::makeForwardSlashPath( oCurFileNode.getFileName() );
							aoFileList.push_back( oCurFileNode );
						}
					}
			    }
            }
            while( ( false == m_bAbort ) &&
                    ( FindNextFileW( hFind,
                                   &sFindData ) ) );
            if( true == m_bAbort )
            {
                rc = ERROR_OPERATION_ABORTED;
            }
            FindClose ( hFind );
        }
    }
#else
	// linux version of find files
	// find the files in the directory
	DIR *pDir;
	char as8SrcDir[ VX_MAX_PATH * 2 ];
	strcpy( as8SrcDir, csPath.c_str() );

	struct dirent *pFileEnt;
    if( VxFileUtil::directoryExists( as8SrcDir ) )
	{
		//LogMsg( LOG_ERROR, "VxListFiles: directory %s exists.. opening dir\n", as8SrcDir );
		//ok directory exists!
		if(!(NULL == (pDir = opendir(as8SrcDir))))
		{
			//pDir is open
			while( 0 != (pFileEnt = readdir(pDir)))
			{
				//LogMsg( LOG_INFO, "FindFilesByExtension: found file %s\n", pFileEnt->d_name );
				if ( '/' == csPath[ csPath.size( ) - 1 ] )
				{
                    oCurFileNode.setFileName( csPath + pFileEnt->d_name );
				}
				else
				{
                    oCurFileNode.setFileName( csPath + '/' + pFileEnt->d_name );
				}
				struct stat oStat;
                if( 0 != stat( oCurFileNode.getFileName().c_str(), &oStat ) )
				{
					///ERROR how do we handle
                    LogMsg( LOG_INFO, "FindFilesByExtension: Unable to stat file %s\n", oCurFileNode.getFileName().c_str() );
					continue;
				}

				if( S_IFDIR & oStat.st_mode  )
				{
					if ( ( bRecurse ) &&
                        ( !VxFileUtil::isDotDotDirectory( pFileEnt->d_name ) ) )
					{
                        FindFilesByName(    oCurFileNode.getFileName(),
                                            acsWildNameList,
                                            aoFileList,
                                            bRecurse,								//recurse subdirectories if true
                                            bUseFilterListToExclude
                                            );
					}
				}
				else
				{
					//include or exclude on the given list of wild Name Match Strings
					if( bUseFilterListToExclude )
					{
                        if( ! HasMatchingName( oCurFileNode.getFileName(), acsWildNameList ) )
						{
                            oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
                            oCurFileNode.setFileLength( oStat.st_size );
							aoFileList.push_back( oCurFileNode );
						}
					}
					else
					{
                        if( HasMatchingName( oCurFileNode.getFileName(), acsWildNameList ) )
						{
                            oCurFileNode.setFileType( VxFileUtil::fileExtensionToFileTypeFlag( oCurFileNode.getFileName().c_str() ) );
                            oCurFileNode.setFileLength( oStat.st_size );
							aoFileList.push_back( oCurFileNode );
						}
					}
				}
			}
			// end of listing
			// done with listing
			closedir(pDir);
			return 0;
		}
		else
		{
			LogMsg( LOG_ERROR, "VxListFiles: could not open directory %s \n", as8SrcDir );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "VxListFiles: directory %s does not exist \n", as8SrcDir );
	}
	return 0;

#endif // TARGET_OS_WINDOWS
    return rc;
}

//============================================================================
bool VxFileFinder::HasMatchingName( std::string csCurrentNode,
									std::vector<std::string> &acsWildNameList )
{
	int iCnt = (int)acsWildNameList.size();
	for( int i = 0; i < iCnt; i++ )
	{
		if( VxFileUtil::fileNameWildMatch( csCurrentNode.c_str(), acsWildNameList[ i ].c_str() ) )
		{
			return true;
		}
	}
	return false;

}

//============================================================================
int VxFindFilesByExtension(std::string csPath,				//start path to search in
                std::vector<std::string> &acsExtensionList,//Extensions ( file extentions )
                std::vector<VxFileInfo> &aoFileList,//return FileInfo in array
				bool bRecurse,						//recurse subdirectories if true
				bool bUseFilterListToExclude		//if true dont return files matching filter else return files that do
				)
{
	VxFileFinder oFileFinder;
	return oFileFinder.FindFilesByExtension( csPath,//start path to search in
                acsExtensionList,				//Extensions of files to find
                aoFileList,						//return FileInfo in array
				bRecurse,						//recurse subdirectories if true
				bUseFilterListToExclude			//if true dont return files matching filter else return files that do
				);
}

//============================================================================
int VxFindFilesByName(std::string csPath,				//start path to search in
                std::vector<std::string> &acsWildNameList,//Extensions ( file extentions )
                std::vector<VxFileInfo> &aoFileList,//return FileInfo in array
				bool bRecurse,						//recurse subdirectories if true
				bool bUseFilterListToExclude		//if true dont return files matching filter else return files that do
				)
{
	VxFileFinder oFileFinder;
	return oFileFinder.FindFilesByName( csPath,//start path to search in
                acsWildNameList,				//Extensions of files to find
                aoFileList,						//return FileInfo in array
				bRecurse,						//recurse subdirectories if true
				bUseFilterListToExclude			//if true dont return files matching filter else return files that do
				);
}

