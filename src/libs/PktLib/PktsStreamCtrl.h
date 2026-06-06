#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxCommon.h"
#include "VxPktHdr.h"

#include <CoreLib/IsBigEndianCpu.h>
#include <CoreLib/VxSha1Hash.h>

#define PKT_TYPE_FILE_MAX_DATA_LEN		14320	// maximum length of chunk of file data
#define MAX_FILE_LIST_LEN				4096	// maximum length of list of files

enum EStreamCtrl
{
	eStreamCtrlNone,
	eStreamReadTail,
	eStreamSeek,

	eMaxStreamCtrl
};

#pragma pack(push)
#pragma pack(1)

class PktStreamCtrlReq : public VxPktHdr
{
public:
    PktStreamCtrlReq();

	void						setAssetId( VxGUID& fileAssetId )				{ m_FileAssetId = fileAssetId; }
	VxGUID&						getAssetId( void )								{ return m_FileAssetId; }
	void						setFileHashId( VxSha1Hash& fileHashId )			{ m_FileHashId = fileHashId; }
	VxSha1Hash&					getFileHashId( void )							{ return m_FileHashId; }

	void						setLclSessionId( VxGUID& lclId )			    { m_LclSessionId = lclId; }
	VxGUID&						getLclSessionId( void )						    { return m_LclSessionId; }
	void						setRmtSessionId( VxGUID& rmtId )			    { m_RmtSessionId = rmtId; }
	VxGUID&						getRmtSessionId( void )						    { return m_RmtSessionId; }

	void						setStartOffset( int64_t offset )				{ m_s64StartOffs = htonU64( offset ); }
	int64_t						getStartOffset( void )							{ return ntohU64( m_s64StartOffs ); }
	void						setEndOffset( int64_t offset )					{ m_s64EndOffs = htonU64( offset ); }
	int64_t						getEndOffset( void )							{ return ntohU64( m_s64EndOffs ); }

	void						setStreamCtrl( enum EStreamCtrl streamCtrl )		{ m_StreamCtrl = (uint8_t)streamCtrl; }
	enum EStreamCtrl			getStreamCtrl( void )							    { return (EStreamCtrl)m_StreamCtrl; }

private:
	VxGUID						m_LclSessionId;
	VxGUID						m_RmtSessionId;
	VxGUID						m_FileAssetId;
	VxSha1Hash					m_FileHashId;
	int64_t						m_s64StartOffs{ 0 };
	int64_t						m_s64EndOffs{ 0 };	//if 0 then get all
	uint8_t						m_StreamCtrl{ 0 };
	uint8_t						m_u8Res1{ 0 };
	uint16_t					m_u16Res1{ 0 };
};

class PktStreamCtrlReply : public VxPktHdr
{
public:
	PktStreamCtrlReply();

	void						setAssetId( VxGUID& fileAssetId )					{ m_FileAssetId = fileAssetId; }
	VxGUID&						getAssetId( void )									{ return m_FileAssetId; }
	void						setFileHashId( VxSha1Hash& fileHashId )			    { m_FileHashId = fileHashId; }
	VxSha1Hash&					getFileHashId( void )							    { return m_FileHashId; }
    uint16_t					getemptyLen( void );

	void						setLclSessionId( VxGUID& lclId )			        { m_LclSessionId = lclId; }
	VxGUID&						getLclSessionId( void )						        { return m_LclSessionId; }
	void						setRmtSessionId( VxGUID& rmtId )			        { m_RmtSessionId = rmtId; }
	VxGUID&						getRmtSessionId( void )						        { return m_RmtSessionId; }

	void						setStartOffset( int64_t offset )					{ m_s64StartOffs = htonU64( offset ); }
	int64_t						getStartOffset( void )							    { return ntohU64( m_s64StartOffs ); }
	void						setEndOffset( int64_t offset )						{ m_s64EndOffs = htonU64( offset ); }
	int64_t						getEndOffset( void )							    { return ntohU64( m_s64EndOffs ); }

	void						setError( uint32_t error )							{ m_u32Error = htonl( error ); }
	uint32_t					getError( void )							        { return ntohl( m_u32Error ); }

	void						setStreamCtrl( enum EStreamCtrl streamCtrl )		{ m_StreamCtrl = (uint8_t)streamCtrl; }
	enum EStreamCtrl			getStreamCtrl( void )							    { return (EStreamCtrl)m_StreamCtrl; }

	void						setDataLen( uint16_t dataLen );
	uint16_t					getDataLen( void )							        { return ntohs( m_u16DataLen ); }
	char*						getDataBuf( void )									{ return m_ChunkData; }

private:
	VxGUID						m_LclSessionId;
	VxGUID						m_RmtSessionId;
	VxGUID						m_FileAssetId;
	VxSha1Hash					m_FileHashId;
	int64_t						m_s64StartOffs{ 0 };
	int64_t						m_s64EndOffs{ 0 };	//if 0 then get all
	uint32_t					m_u32Error{ 0 };
	uint16_t					m_u16DataLen{ 0 };
	uint8_t						m_StreamCtrl{ 0 };
	uint8_t						m_u8Res1{ 0 };
	uint32_t					m_u32Res2{ 0 };
	uint64_t					m_u64Res3{ 0 };
	char						m_ChunkData[ PKT_TYPE_FILE_MAX_DATA_LEN ];
};

#pragma pack(pop) 

const char* DescribeStreamCtrlError( uint16_t streamError );

