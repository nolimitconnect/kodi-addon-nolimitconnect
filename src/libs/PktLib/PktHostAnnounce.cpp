//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktHostAnnounce.h"
#include "PktTypes.h"

#include <CoreLib/VxParse.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxTime.h>

#include <memory.h>

#define PKT_HOST_ANNOUNCE_VERSION 1

//============================================================================
//============================================================================
PktHostAnnounce::PktHostAnnounce()
    : PktAnnounce()
{ 
	setPktLength( sizeof( PktHostAnnounce ) );
	setPktType(  PKT_TYPE_HOST_ANNOUNCE );
    setPktVersionNum( PKT_HOST_ANNOUNCE_VERSION );
    // LogMsg( LOG_DEBUG, "PktHostAnnounce size %d", sizeof( PktHostAnnounce ) );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktHostAnnounce* PktHostAnnounce::makeHostAnnCopy( void )
{
	vx_assert( getPktLength() );
    vx_assert( (getPktLength() & 0x0f) == 0 );
	vx_assert( PKT_TYPE_HOST_ANNOUNCE == getPktType() );
	char * pTemp = new char[ getPktLength() ];
	vx_assert( pTemp );
	memcpy( pTemp, this, getPktLength() );
	return ( PktHostAnnounce *)pTemp;
}

//============================================================================
void PktHostAnnounce::setPktAnn( PktAnnounce& pktAnn )
{
    const char * pktAnnData = ( const char *)&pktAnn;
    int pktAnnDataLen = pktAnn.getPktLength() - sizeof( VxPktHdr );
    pktAnnData += sizeof( VxPktHdr );
    char * pktHost = ( char * )this;
    pktHost += sizeof( VxPktHdr );
    if( pktAnnDataLen > 0 && pktAnnDataLen < (int)sizeof( PktHostAnnounce ) )
    {
        memcpy( pktHost, pktAnnData, pktAnnDataLen );
    }

    setSrcOnlineId( pktAnn.getSrcOnlineId() );
}

//============================================================================
bool PktHostAnnounce::fillPktHostAnn( PktHostAnnounce& pktAnn )
{
    bool result = false;
    char * pktAnnData = ( char * )&pktAnn;
    char * pktHost = ( char * )this;

    int pktAnnDataLen = getPktLength();
    if( pktAnnDataLen > 0 && pktAnnDataLen <= (int)sizeof( PktHostAnnounce ) )
    {
        memcpy( pktAnnData, pktHost, pktAnnDataLen );
        result = true;
    }

    return result;
}

//============================================================================
void PktHostAnnounce::calcPktLen( void )
{
    setPktLength( ROUND_TO_16BYTE_BOUNDRY( sizeof( PktHostAnnounce ) - ( BLOB_PLUGIN_SETTING_MAX_STORAGE_LEN + 16 ) + m_SettingLength ) );
}

//============================================================================
bool PktHostAnnounce::getPluginSettingBinary( BinaryBlob& settingBinary )
{
    if( m_SettingLength && m_SettingLength <= BLOB_PLUGIN_SETTING_MAX_STORAGE_LEN )
    {
        settingBinary.setBlobData( m_SettingData, m_SettingLength, false, false );
        return true;
    }

    return false;
}

//============================================================================
bool PktHostAnnounce::setPluginSettingBinary( BinaryBlob& settingBinary )
{
    if( settingBinary.getBlobLen() && settingBinary.getBlobLen() <= BLOB_PLUGIN_SETTING_MAX_STORAGE_LEN )
    {
        m_SettingLength = settingBinary.getBlobLen();
        memcpy( m_SettingData, settingBinary.getBlobData(), settingBinary.getBlobLen() );
        calcPktLen();
        return true;
    }

    return false;
}

//============================================================================
PktHostAnnounce * PktHostAnnounce::makeHostAnnReverseCopy( void )
{
	PktHostAnnounce * pTemp = makeHostAnnCopy();
	pTemp->reversePermissions();
	return pTemp;
}

//============================================================================
bool PktHostAnnounce::isValidPktHostAnn( void )
{
	return ( ( getPktLength() == sizeof( PktHostAnnounce ) ) &&
			 ( getPktType() == PKT_TYPE_HOST_ANNOUNCE ) );
}

//============================================================================
//! dump contents of pkt announce for debug
void PktHostAnnounce::DebugHostDump( void )
{
	std::string strName;
	std::string strDesc;
	std::string strIp;
	std::string strId;

	this->getMyOnlineId( strId );
	this->getMyOnlineIPv4( strIp );
	uint16_t u16Port = this->getOnlinePort();
	strName = this->getOnlineName();
	strDesc = this->getOnlineDescription();
	//strNetwork = this->getNetworkKey();

	LogMsg( LOG_INFO, "PktHostAnnounce Len %d Version #%d name %s Ip %s Port %d desc %s\n",
			getPktLength(),		// packet length
			getPktVersionNum(),		// version of program
			strName.c_str(),		
			strIp.c_str(),		// IP of announcer
			u16Port,			// Port announcer listens on
			strDesc.c_str()
			);		
}

