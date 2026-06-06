//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsTestConnection.h"

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
PktTestConnTestReq::PktTestConnTestReq()
{
    setPktType( PKT_TYPE_TEST_CONN_TEST_REQ );
    setPktLength( sizeof( PktTestConnTestReq ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
    m_SessionId.fillRandom(); 
}

//============================================================================
void PktTestConnTestReq::calcPktLen( void )
{
    uint16_t pktLen = ( uint16_t )sizeof( PktTestConnTestReq ) - sizeof( PktBlobEntry );
    pktLen += getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
bool PktTestConnTestReq::setNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetWrite();
    bool result = m_BlobEntry.setValue( netCmd );
    calcPktLen();
    
    return result;
}

//============================================================================
bool PktTestConnTestReq::getNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetRead();
    bool result = m_BlobEntry.getValue( netCmd );

    return result;
}

//============================================================================
PktTestConnTestReply::PktTestConnTestReply()
{
    setPktType( PKT_TYPE_TEST_CONN_TEST_REPLY );
    setPktLength( sizeof( PktTestConnTestReply ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
    m_SessionId.fillRandom(); 
}

//============================================================================
void PktTestConnTestReply::calcPktLen( void )
{
    uint16_t pktLen = ( uint16_t )sizeof( PktTestConnTestReply ) - sizeof( PktBlobEntry );
    pktLen += getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
bool PktTestConnTestReply::setNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetWrite();
    bool result = m_BlobEntry.setValue( netCmd );
    calcPktLen();

    return result;
}

//============================================================================
bool PktTestConnTestReply::getNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetRead();
    bool result = m_BlobEntry.getValue( netCmd );

    return result;
}

//============================================================================
//============================================================================
PktTestConnPingReq::PktTestConnPingReq()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_TEST_CONN_PING_REQ );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktTestConnPingReq ) ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
    m_SessionId.fillRandom(); 
}

//============================================================================
void PktTestConnPingReq::calcPktLen( void )
{
    uint16_t pktLen = ( uint16_t )sizeof( PktTestConnPingReq ) - sizeof( PktBlobEntry );
    pktLen += getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );
    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
bool PktTestConnPingReq::setNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetWrite();
    bool result = m_BlobEntry.setValue( netCmd );
    calcPktLen();

    return result;
}

//============================================================================
bool PktTestConnPingReq::getNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetRead();
    bool result = m_BlobEntry.getValue( netCmd );
    
    return result && !netCmd.empty();
}

//============================================================================
//============================================================================
PktTestConnPingReply::PktTestConnPingReply()
    : VxPktHdr()
{
    setPktType( PKT_TYPE_TEST_CONN_PING_REPLY );
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktTestConnPingReply ) ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
    m_SessionId.fillRandom(); 
}

//============================================================================
void PktTestConnPingReply::calcPktLen()
{
    uint16_t pktLen = ( uint16_t )sizeof( PktTestConnPingReply ) - sizeof( PktBlobEntry );
    uint16_t blobLen = getBlobEntry().getTotalBlobLen();
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen + blobLen ) );

    vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
bool PktTestConnPingReply::setNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetWrite();
    bool result = m_BlobEntry.setValue( netCmd );
    calcPktLen();

    return result;
}

//============================================================================
bool PktTestConnPingReply::getNetCmd( std::string& netCmd )
{
    m_BlobEntry.resetRead();
    bool result = m_BlobEntry.getValue( netCmd );
    
    return result && !netCmd.empty();
}
