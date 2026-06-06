//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsHostSearch.h"

#include <CoreLib/VxDebug.h>

#include <string.h>


//============================================================================
PktHostSearchReq::PktHostSearchReq()
    : VxPktHdr()
    , m_BlobEntry()
{
    setPktType( PKT_TYPE_HOST_SEARCH_REQ );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostSearchReq ) ) ); 
    // LogMsg( LOG_DEBUG, "PktHostSearchReq size %d %d", sizeof( PktHostSearchReq ), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
void PktHostSearchReq::calcPktLen()
{
    uint16_t pktLen = ( uint16_t)sizeof( PktHostSearchReq ) - sizeof(PktBlobEntry);
    uint16_t blobLen = getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen + blobLen ) ); 

    // LogMsg( LOG_DEBUG, "PktHostSearchReq calcPktLen blob %d len %d %d", blobLen, getPktLength(), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
PktHostSearchReply::PktHostSearchReply()
    : VxPktHdr()
    , m_BlobEntry()
{
    setPktType( PKT_TYPE_HOST_SEARCH_REPLY );
    setPktLength( sizeof(PktHostSearchReply) );
    LogMsg( LOG_DEBUG, "PktHostSearchReply size %d %d", sizeof( PktHostSearchReply ), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktHostSearchReply::calcPktLen()
{
    uint16_t pktLen = ( uint16_t)sizeof( PktHostSearchReply ) - sizeof(PktBlobEntry);
    uint16_t blobLen = getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen + blobLen ) ); 

    LogMsg( LOG_DEBUG, "PktHostSearchReply calcPktLen blob %d len %d %d", blobLen, getPktLength(), (getPktLength() & 0x0f) ); 
    vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
void PktHostSearchReply::setTotalMatches( uint16_t matchCnt )
{
    m_TotalMatches = htons( matchCnt );
}

//============================================================================
uint16_t PktHostSearchReply::getTotalMatches( void ) const
{
    return ntohs( m_TotalMatches );
}

//============================================================================
// add guid to blob and increment total match count
bool PktHostSearchReply::addMatchOnlineId( VxGUID& onlineId )
{
    bool result = m_BlobEntry.setValue( onlineId );
    if( result )
    {
        setTotalMatches( getTotalMatches() + 1 );
    }

    return result;
}

//============================================================================
// add guid and plugin type to blob and increment total match count
bool PktHostSearchReply::addPluginId( const PluginId& pluginId )
{
    bool result = m_BlobEntry.setValue( pluginId );
    if( result )
    {
        setTotalMatches( getTotalMatches() + 1 );
    }

    return result;
}

//============================================================================
uint16_t PktHostSearchReply::getRemainingBlobStorageLen( void ) const
{
    return m_BlobEntry.getRemainingStorageLen();
}