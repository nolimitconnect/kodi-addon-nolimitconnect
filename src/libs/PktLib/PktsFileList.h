#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPktHdr.h"

#include <CoreLib/VxFileInfo.h>

#include <vector>

#pragma pack(push)
#pragma pack(1)

#define MAX_SHARED_FILE_LIST_LEN			12288
#define ERR_NO_SHARED_FILES					0x0001
#define ERR_FILE_LIST_IDX_OUT_OF_RANGE		0x0002

class PktFileListReq : public VxPktHdr
{
public:
	PktFileListReq();

	void						setListIndex( uint32_t index );
	uint32_t					getListIndex( void );

private:
	//=== vars ===//
	uint32_t					m_u16Res;	
	uint32_t					m_u32Index;			
};

class PktFileListReply : public VxPktHdr
{
public:
	PktFileListReply();

	void						calcPktLen( uint16_t dataLen );

	void						setIsListCompleted( bool isCompleted )			{ m_bListComplete = isCompleted ? 1 : 0; }
	bool						getIsListCompleted( void )						{ return m_bListComplete ? true : false; }

	void						setError( uint32_t u32Error );
	uint32_t					getError( void );

	void						setListDataLen( uint16_t dataLen );
	uint16_t					getListDataLen( void );
	void						setFileCount( uint16_t fileCount );
	uint16_t					getFileCount( void );
	void						setListIndex( uint32_t index );
	uint32_t					getListIndex( void );

	void						getFileList( std::vector<VxFileInfo>& retList );
	bool						canAddFile( int fileNameLenIncludingZero );
	void						addFile( VxSha1Hash& fileHashId, uint64_t fileLen, uint8_t fileTypeFlags, const char* fileName );

private:
	//=== vars ===//
	uint8_t						m_bListComplete;		// if 1 then list has been completed
	uint8_t						m_u8Res1;
	uint16_t					m_u16Res;				
	uint16_t					m_u16DataLen;			// length of data
	uint16_t					m_u16FileCnt;			// number of files in this pkt
	uint32_t					m_u32Error;	
	uint32_t					m_u32ListIndex;
	char						m_as8FileList[ MAX_SHARED_FILE_LIST_LEN ];
};

#pragma pack(pop)


