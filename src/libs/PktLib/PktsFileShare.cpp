//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktsFileShare.h"

#include <CoreLib/VxChop.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>

#include <string.h>

//============================================================================
PktFileGetReq::PktFileGetReq()
{
   setPktType( PKT_TYPE_FILE_GET_REQ );
}

//============================================================================
void PktFileGetReq::setFileName( std::string &csName )
{
	if( csName.size() )
	{
		strcpy( (char *)m_FileName, csName.c_str() );
	}
	else
	{
		m_FileName[ 0 ] = 0; 
	}

	uint16_t u16PktLen = ( uint16_t )((sizeof( PktFileGetReq ) - sizeof( m_FileName )) + strlen( m_FileName ) + 1);
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( u16PktLen ) );
}

//============================================================================
void PktFileGetReq::getFileName( std::string &csName )
{
	csName = m_FileName;
}

//============================================================================
PktFileGetReply::PktFileGetReply()
{
	setPktType( PKT_TYPE_FILE_GET_REPLY );
}

//============================================================================
void PktFileGetReply::getFileName( std::string &csName )
{
	csName = m_FileName;
}

//============================================================================
void PktFileGetReply::setFileName( std::string &csName )
{
    if( csName.size() )
    {
		strcpy( (char *)m_FileName, csName.c_str() );
    }
    else
    {
        m_FileName[ 0 ] = 0;
    }

	uint16_t u16PktLen = ( uint16_t)(( sizeof( PktFileGetReply ) - sizeof( m_FileName )) + strlen( m_FileName ) + 1);
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( u16PktLen ) );
}

//============================================================================
uint16_t PktFileGetReply::getemptyLen( void )
{
    return (uint16_t)(sizeof( PktFileGetReq )-VX_MAX_PATH);
}

//============================================================================
// PktFileSendReq
//============================================================================
PktFileSendReq::PktFileSendReq()
{ 
	setPktType( PKT_TYPE_FILE_SEND_REQ ); 
	setFileName( "" );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktFileSendReq::setFileName( const char* pFileName )
{
	strcpy( (char *)m_FileName, pFileName );
	uint16_t u16PktLen = (uint16_t)((sizeof( PktFileSendReq ) - sizeof( m_FileName )) + strlen( m_FileName ) + 1);
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( u16PktLen ) );
}

//============================================================================
PktFileSendReply::PktFileSendReply()
{ 
	//LogMsg( LOG_DEBUG, "PktFileSendReq sizeof %d", sizeof( PktFileSendReq ) );
	//LogMsg( LOG_DEBUG, "PktFileSendReply sizeof %d", sizeof( PktFileSendReply ) );
	//LogMsg( LOG_DEBUG, "PktFileGetReq sizeof %d", sizeof( PktFileGetReq ) );
	//LogMsg( LOG_DEBUG, "PktFileGetReply sizeof %d", sizeof( PktFileGetReply ) );
	//LogMsg( LOG_DEBUG, "PktFileFindReq sizeof %d", sizeof( PktFileFindReq ) );
	//LogMsg( LOG_DEBUG, "PktFileChunkReq sizeof %d", sizeof( PktFileChunkReq ) );
	//LogMsg( LOG_DEBUG, "PktFileChunkReply sizeof %d", sizeof( PktFileChunkReply ) );
	
	setPktType( PKT_TYPE_FILE_SEND_REPLY ); 
	setPktLength( sizeof( PktFileSendReply ) );
	m_FileName[0] = 0;
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktFileSendReply::setFileName( const char* pFileName )
{
	strcpy( (char *)m_FileName, pFileName );
	uint16_t u16PktLen = (uint16_t)((sizeof( PktFileSendReply ) - sizeof( m_FileName )) + strlen( m_FileName ) + 1);
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( u16PktLen ) );
}

//============================================================================
// PktFileFindReq
//============================================================================
PktFileFindReq::PktFileFindReq()
{ 
	setPktLength( getEmptyLen() ); 
	setPktType(  PKT_TYPE_FILE_FIND_REQ ); 
	m_MatchName[ 0 ] = 0;
}

//============================================================================
void PktFileFindReq::SetMatchName( std::string &csName )
{
    if( ( PKT_SHARE_FIND_FILE_MATCHNAME_MAX_LEN - 1) > csName.size() )
	{
		strcpy( m_MatchName, (const char*)csName.c_str() );
		setPktLength( ROUND_TO_16BYTE_BOUNDRY( getEmptyLen()  + csName.size() + 1 ) );
	}
	else
	{
		m_MatchName[ 0 ] = 0; 
		setPktLength( getEmptyLen() );
	}
}

//============================================================================
void PktFileFindReq::GetMatchName( std::string &csName )
{
	if( getEmptyLen() >= getPktLength() )
	{
		vx_assert( false );
		csName = "";
	}
	else
	{
		char as8Buf[ PKT_SHARE_FIND_FILE_MATCHNAME_MAX_LEN ];
		VxUnchopStr( (unsigned char *)m_MatchName, as8Buf );
		csName = as8Buf;
	}
}

//============================================================================
PktFileChunkReq::PktFileChunkReq()
{
	setPktType( PKT_TYPE_FILE_CHUNK_REQ );
	setPktLength( emptyLength() );
}

//============================================================================
uint16_t PktFileChunkReq::emptyLength( void )
{ 
	return (uint16_t)(sizeof( PktFileChunkReq) - PKT_TYPE_FILE_MAX_DATA_LEN); 
}

//============================================================================
void PktFileChunkReq::setChunkLen( uint16_t u16ChunkLen ) 
{ 
	m_u16FileChunkLen = htons( u16ChunkLen ); 
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( emptyLength() + u16ChunkLen ) );
}

//============================================================================
uint16_t PktFileChunkReq::getChunkLen( void ) 
{ 
	return htons( m_u16FileChunkLen ); 
}

//============================================================================
PktFileChunkReply::PktFileChunkReply()
{
	setPktType( PKT_TYPE_FILE_CHUNK_REPLY );
	setPktLength( (uint16_t)sizeof( PktFileChunkReply ) );
}

//============================================================================
PktFileSendCompleteReq::PktFileSendCompleteReq()
{
	setPktType(  PKT_TYPE_FILE_SEND_COMPLETE_REQ );
	setPktLength( (uint16_t)sizeof( PktFileSendCompleteReq ) );
}

//============================================================================
PktFileSendCompleteReply::PktFileSendCompleteReply()
{
	setPktType( PKT_TYPE_FILE_SEND_COMPLETE_REPLY );
	setPktLength( (uint16_t)sizeof( PktFileSendCompleteReply ) );
}

//============================================================================
PktFileGetCompleteReq::PktFileGetCompleteReq()
{
	setPktType( PKT_TYPE_FILE_GET_COMPLETE_REQ );
	setPktLength( (uint16_t)sizeof( PktFileGetCompleteReq ) );
}

//============================================================================
PktFileGetCompleteReply::PktFileGetCompleteReply()
{
	setPktType( PKT_TYPE_FILE_GET_COMPLETE_REPLY );
	setPktLength( (uint16_t)sizeof( PktFileGetCompleteReply ) );
}

//============================================================================
PktFileXferCancel::PktFileXferCancel()
{
	setPktType( PKT_TYPE_FILE_XFER_CANCEL );
	setPktLength( (uint16_t)sizeof( PktFileXferCancel ) );
}

//============================================================================
PktFileShareErr::PktFileShareErr()
{
	setPktType( PKT_TYPE_FILE_SHARE_ERR ); 
	setPktLength( sizeof( PktFileShareErr ) );
}

//============================================================================
const char* PktFileShareErr::describeError( void )
{
	switch( m_u16Err )
	{
	case PKT_REQ_STATUS_OK:
		return "200 Ok";
	case PKT_REQ_STATUS_CREATED:
		return "201 Created";
	case PKT_REQ_STATUS_ACCEPTED:
		return "202 Accepted";
	case PKT_REQ_ERR_NO_CONTENT:
		return "204 No Content";
	case PKT_REQ_ERR_MOVED_PERM:
		return "301 Moved Permanently";
	case PKT_REQ_ERR_MOVED_TEMP:
		return "301 Moved Temporarily";
	case PKT_REQ_ERR_NOT_MODIFIED:
		return "304 Not Modified";
	case PKT_REQ_ERR_BAD_REQUEST:
		return "400 Bad Request";
	case PKT_REQ_ERR_UNAUTHORIZED:
		return "401 Unauthorized";
	case PKT_REQ_ERR_FORBIDDEN:
		return "403 Forbidden";
	case PKT_REQ_ERR_NOT_FOUND:
		return "404 Not Found";
	case PKT_REQ_ERR_INTERNAL_SERVER_ERR:
		return "500 Internal Server Error";
	case PKT_REQ_ERR_NOT_IMPLEMENTED:
		return "501 Not Implemented";
	case PKT_REQ_ERR_BAD_GATEWAY:
		return "502 Bad Gateway";
	case PKT_REQ_ERR_SERVICE_UNAVAIL:
		return "503 Service Unavailable";
		//custom statuses
	case PKT_REQ_ERR_BANDWITH_LIMIT:
		return "600 Refused because of Bandwidth limit";
	case PKT_REQ_ERR_CONNECT_LIMIT:
		return "601 Refused because of Connection limit";
	case PKT_REQ_ERR_SERVICE_DISABLED:
		return "602	Refused because Service was disabled";
	case PKT_REQ_ERR_ALL_THREADS_BUSY:
		return "602	Refused because all threads are busy";
	default:
		return "Unknown Err";
	}
}


