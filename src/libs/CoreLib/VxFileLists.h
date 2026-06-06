#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================


#include <CoreLib/VxFileInfo.h>

#include <vector>

class VxFileList
{
public:
	// if list limit then files added after list full will add to front and remove from back
	void						setListLimit( int listItems ) { m_ListLimit = listItems; }

	bool 						empty( void ) { return m_FileList.empty(); }
	size_t						size( void ) { return m_FileList.size(); }

	std::string					getFileAtIndex( int listIdx );

	std::vector<std::string>&	getFileList( void ) { return m_FileList; };

	bool						addFileToFront( std::string fileName, bool checkIfFileExists = true );
	bool						removeFileFromFront( void );

	bool						addFileToBack( std::string fileName, bool checkIfFileExists = true );
	bool						removeFileFromBack( void );

	void						removeFile( std::string fileName );

    void                        dumpToLog( uint32_t dbgLevel );

protected:
	bool						doesFileExistInDiskStorage( std::string& fileName );

	size_t						m_ListLimit{0};
	std::vector<std::string>	m_FileList;
};

class VxFileInfoList
{
public:
    // if list limit then files added after list full will add to front and remove from back
    void						setListLimit( int listItems ) { m_ListLimit = listItems; }

    bool 						empty( void ) { return m_FileInfoList.empty(); }
    size_t						size( void ) { return m_FileInfoList.size(); }

    std::string                 getFileNameFromNameAndPath( std::string fileNameAndPath );
    std::string					getFileNameAtIndex( int listIdx );
    std::string					getFileNameAndPathAtIndex( int listIdx );

    std::vector<VxFileInfoBase>& getFileInfoList( void ) { return m_FileInfoList; };

    void                        moveToTopOfList( VxFileInfoBase& fileInfo );
    void                        moveToTopOfList( std::string fileNameAndPath );

protected:
    bool						doesFileExistInDiskStorage( std::string& fileNameAndPath );

    size_t						m_ListLimit{0};
    std::vector<VxFileInfoBase>	m_FileInfoList;
};

class VxFileFinder
{
public:
	bool m_bAbort;

	void Abort( void ){ m_bAbort = true; }

	//find files matching the extension (file type) list. returns non zero if error
	int FindFilesByExtension(	std::string					csPath,								//start path to search in
								std::vector<std::string> &	acsExtensionList,					//Extensions of files to find
								std::vector<VxFileInfo> &	aoFileList,							//return FileInfo in array
								bool						bRecurse = false,					//recurse subdirectories if true
								bool						bUseFilterListToExclude = false		//if true dont return files matching filter else return files that do
								);
	//find files matching the extension (file type) list
	int FindFilesByName(		std::string					csPath,								//start path to search in
								std::vector<std::string> &	acsWildNameList,					//Wildcard Name match strings
								std::vector<VxFileInfo> &	aoFileList,							//return FileInfo in array
								bool						bRecurse = false,					//recurse subdirectories if true
								bool						bUseFilterListToExclude = false		//if true dont return files matching filter else return files that do
								);
protected:
	bool HasSameExtension( std::string csCurrentNode, std::vector<std::string> &acsExtensionList );
	bool HasMatchingName( std::string csCurrentNode, std::vector<std::string> &acsWildNameList );
};

//! search for files
int VxFindFilesByExtension(	std::string					csPath,							//start path to search in
							std::vector<std::string>&	acsExtensionList,				//Extensions ( file extentions )
							std::vector<VxFileInfo>&	aoFileList,						//return FileInfo in array
							bool						bRecurse = false,				//recurse subdirectories if true
							bool						bUseFilterListToExclude = false	//if true dont return files matching filter else return files that do
							);

int VxFindFilesByName(	std::string					csPath,							//start path to search in
						std::vector<std::string>&	acsWildNameList,				//filters
						std::vector<VxFileInfo>&	aoFileList,						//return FileInfo in array
						bool						bRecurse = false,				//recurse subdirectories if true
						bool						bUseFilterListToExclude = false	//if true dont return files matching filter else return files that do
						);




