//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktAnnounce.h"
#include "PktTypes.h"

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxTime.h>

#include <memory.h>

namespace
{
    const int64_t           g_u64NearbyUdpTimeoutMs       = 60000;	// if haven't received udp broadcast in these milli seconds then user is no longer nearby
    const int64_t           g_u64OnlineStatusTimeoutMs    = 600000;	// if haven't had udp or tcp activity in these milli seconds then user is no longer online
}

//============================================================================
P2PEngineVersion::P2PEngineVersion()
    : m_u8P2PEngineVersion( P2P_ENGINE_VERSION )
{
}

//============================================================================
P2PEngineVersion::P2PEngineVersion( const P2PEngineVersion& rhs )
    : m_u8P2PEngineVersion( rhs.m_u8P2PEngineVersion )
{
}

//============================================================================
P2PEngineVersion& P2PEngineVersion::operator =( const P2PEngineVersion& rhs )
{
    if( this != &rhs )
    {
        m_u8P2PEngineVersion = rhs.m_u8P2PEngineVersion;
    }

    return *this;
}

//============================================================================
bool P2PEngineVersion::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_u8P2PEngineVersion );
}

//============================================================================
bool P2PEngineVersion::extractFromBlob( PktBlobEntry& blob )
{
    return blob.getValue( m_u8P2PEngineVersion );
}

//============================================================================
uint8_t P2PEngineVersion::getP2PEngineVersion( void )
{ 
    return m_u8P2PEngineVersion;
};

//============================================================================
void P2PEngineVersion::getP2PEngineVersion( std::string& strRetP2PEngineVersion )
{ 
    return StdStringFormat( strRetP2PEngineVersion, "%d.%d", (m_u8P2PEngineVersion >> 4)&0x0f,  m_u8P2PEngineVersion & 0x0f );
}

//============================================================================
MyOSVersion::MyOSVersion()
{
#if defined( TARGET_OS_WINDOWS )
	m_u8OSVersion = 1;
#elif defined( TARGET_OS_ANDROID )
	m_u8OSVersion = 2;
#elif defined( TARGET_OS_LINUX )
    m_u8OSVersion = 3;
#elif defined( TARGET_OS_APPLE )
    m_u8OSVersion = 4;
#elif defined( TARGET_OS_RASBERRY_PI )
    m_u8OSVersion = 5;
#else
    m_u8OSVersion = 0;
#endif
}

//============================================================================
MyOSVersion::MyOSVersion( const MyOSVersion& rhs )
    : m_u8OSVersion( rhs.m_u8OSVersion )
{
}

//============================================================================
MyOSVersion& MyOSVersion::operator =( const MyOSVersion& rhs )
{
    if( this != &rhs )
    {
        m_u8OSVersion = rhs.m_u8OSVersion;
    }

    return *this;
}

//============================================================================
bool MyOSVersion::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_u8OSVersion );
}

//============================================================================
bool MyOSVersion::extractFromBlob( PktBlobEntry& blob )
{
    return blob.getValue( m_u8OSVersion );
}

//============================================================================
uint8_t MyOSVersion::getOSVersion( void )
{
	return m_u8OSVersion;
}

//============================================================================
void MyOSVersion::getOSVersion( std::string& strRetOSVersion )
{
	switch( m_u8OSVersion )
	{
	case 1:
		strRetOSVersion = "Windows";
		break;
	case 2:
		strRetOSVersion = "Android";
		break;
	case 3:
		strRetOSVersion = "Linux";
		break;
    case 4:
        strRetOSVersion = "Apple";
        break;
    case 5:
        strRetOSVersion = "RasberryPi";
        break;
	default:
		strRetOSVersion = "Unknown OS";
	}
}

//============================================================================
void		PktAnnBase::setIsPktAnnReplyRequested( bool bReqReply )		    { if( bReqReply )(m_u8RequestFlags |= FLAG_PKT_ANN_REQ_REPLY); else m_u8RequestFlags &= (~FLAG_PKT_ANN_REQ_REPLY); }
bool		PktAnnBase::getIsPktAnnReplyRequested( void )					{ return (m_u8RequestFlags & FLAG_PKT_ANN_REQ_REPLY)?true:false; }
void		PktAnnBase::setIsPktAnnRevConnectRequested( bool bReqConnect )  { if( bReqConnect )(m_u8RequestFlags |= FLAG_PKT_ANN_REQ_REV_CONNECT); else m_u8RequestFlags &= (~FLAG_PKT_ANN_REQ_REV_CONNECT); }
bool		PktAnnBase::getIsPktAnnRevConnectRequested( void )			    { return (m_u8RequestFlags & FLAG_PKT_ANN_REQ_REV_CONNECT)?true:false; }
void		PktAnnBase::setIsPktAnnStunRequested( bool bReqStun )			{ if( bReqStun )(m_u8RequestFlags |= FLAG_PKT_ANN_REQ_STUN); else m_u8RequestFlags &= (~FLAG_PKT_ANN_REQ_STUN); }
bool		PktAnnBase::getIsPktAnnStunRequested( void )					{ return (m_u8RequestFlags & FLAG_PKT_ANN_REQ_STUN)?true:false; }

void		PktAnnBase::setIsPktAnnTempConnection( bool isTemp )
{
    if( isTemp )
        ( m_u8RequestFlags |= FLAG_PKT_ANN_TEMP_CONNECT );
    else m_u8RequestFlags &= ( ~FLAG_PKT_ANN_TEMP_CONNECT );
}
bool		PktAnnBase::getIsPktAnnTempConnection( void )
{
    return ( m_u8RequestFlags & FLAG_PKT_ANN_TEMP_CONNECT ) ? true : false;
}

//============================================================================
PktAnnBase::PktAnnBase( const PktAnnBase& rhs )
    : VxPktHdr( rhs )
    , VxNetIdent( rhs )
    , m_TimeToLive( rhs.m_TimeToLive )
    , m_u8RequestFlags( rhs.m_u8RequestFlags )
    , m_HostAction( rhs.m_HostAction )
    , m_HostType( rhs.m_HostType )
    , m_HostOnlineId( rhs.m_HostOnlineId )
    , m_u32AppAliveTimeSec( rhs.m_u32AppAliveTimeSec )
    , m_AnnRes1( rhs.m_AnnRes1 )
    , m_AnnRes2( rhs.m_AnnRes2 )
    , m_AnnRes3( rhs.m_AnnRes3 )
    , m_AnnRes4( rhs.m_AnnRes4 )
{
}

//============================================================================
PktAnnBase&	PktAnnBase::operator = ( const PktAnnBase& rhs )
{
    if( this != &rhs )
    {
        *((VxPktHdr*)this) = *((VxPktHdr*)&rhs);
        *((VxNetIdent*)this) = *((VxNetIdent*)&rhs);
        m_TimeToLive = rhs.m_TimeToLive;
        m_u8RequestFlags = rhs.m_u8RequestFlags;
        m_HostAction = rhs.m_HostAction;
        m_HostType = rhs.m_HostType;
        m_HostOnlineId = rhs.m_HostOnlineId;
        m_u32AppAliveTimeSec = rhs.m_u32AppAliveTimeSec;
        m_AnnRes1 = rhs.m_AnnRes1;
        m_AnnRes2 = rhs.m_AnnRes2;
        m_AnnRes3 = rhs.m_AnnRes3;
        m_AnnRes4 = rhs.m_AnnRes4;
    }

    return *this;
}

//============================================================================
bool PktAnnBase::addToBlob( PktBlobEntry& blob )
{
    bool result = VxPktHdr::addToBlob( blob );
    result &= VxNetIdent::addToBlob( blob );
    result &= blob.setValue( m_TimeToLive );
    result &= blob.setValue( m_u8RequestFlags );
    result &= blob.setValue( m_HostAction );
    result &= blob.setValue( m_HostType );
    result &= blob.setValue( m_HostOnlineId );
    result &= blob.setValue( m_u32AppAliveTimeSec );
    result &= blob.setValue( m_AnnRes1 );
    result &= blob.setValue( m_AnnRes2 );
    result &= blob.setValue( m_AnnRes3 );
    result &= blob.setValue( m_AnnRes4 );
    return result;
}

//============================================================================
bool PktAnnBase::extractFromBlob( PktBlobEntry& blob )
{
    bool result = VxPktHdr::extractFromBlob( blob );
    result &= VxNetIdent::extractFromBlob( blob );
    result &= blob.getValue( m_TimeToLive );
    result &= blob.getValue( m_u8RequestFlags );
    result &= blob.getValue( m_HostAction );
    result &= blob.getValue( m_HostType );
    result &= blob.getValue( m_HostOnlineId );
    result &= blob.getValue( m_u32AppAliveTimeSec );
    result &= blob.getValue( m_AnnRes1 );
    result &= blob.getValue( m_AnnRes2 );
    result &= blob.getValue( m_AnnRes3 );
    result &= blob.getValue( m_AnnRes4 );
    return result;
}

//============================================================================
void PktAnnBase::setHostId( HostedId& hostedId )
{
    setHostInfo( hostedId.getHostOnlineId(), hostedId.getHostType() );
}

//============================================================================
void PktAnnBase::setHostInfo( VxGUID& hostOnlineId, EHostType hostType )
{
    clearTempValues();
    setHostType( hostType );
    setHostOnlineId( hostOnlineId );
}

//============================================================================
void PktAnnBase::clearTempValues( void )
{
    setIsPktAnnReplyRequested( false );
    setIsPktAnnRevConnectRequested( false );
    setIsPktAnnStunRequested( false );

    m_TimeToLive = 0;
    m_u8RequestFlags = 0;
    m_HostAction = 0;
    m_HostType = 0;
    m_HostOnlineId.clearVxGUID();
    m_AnnRes1 = 0;
    m_AnnRes2 = 0;
    m_AnnRes3 = 0;
    m_AnnRes4 = 0;
}

//============================================================================
//============================================================================


//============================================================================
PktAnnounce::PktAnnounce()	
{ 
#if defined(DEBUG)
static bool firstTime = true;
    if( firstTime )
    {
        firstTime = false;
        size_t hdr = sizeof( VxPktHdr );
        size_t connectBaseInfo = sizeof( VxConnectBaseInfo );
        size_t connectInfo = sizeof( VxConnectInfo );

        size_t netIdent = sizeof( VxNetIdent );
        size_t pktAnnBase = sizeof( PktAnnBase );  
        size_t pktAnn = sizeof( PktAnnounce );
        uint16_t remainder = sizeof( PktAnnounce ) & 0x0f;
        if( remainder )
        {
            LogMsg( LOG_ERROR, "ERROR Invalid PktAnn len %d hdr %d baseinfo %d connectinfo %d ident %d annbase %d pktann %d remainder %d",
                    sizeof( PktAnnounce ), hdr, connectBaseInfo, connectInfo, netIdent, pktAnnBase, pktAnn, remainder );
        }
        else
        {
            LogMsg( LOG_DEBUG, "OK PktAnn len %d hdr %d baseinfo %d connectinfo %d ident %d pktann %d remainder %d",
                    sizeof( PktAnnounce ), hdr, connectBaseInfo, connectInfo, netIdent, pktAnn, remainder );
        }
    }
#endif // defined(DEBUG)

	setPktLength( sizeof( PktAnnounce ) );
	setPktType(  PKT_TYPE_ANNOUNCE );
	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
PktAnnounce::PktAnnounce( const PktAnnounce& rhs )
    : PktAnnBase( rhs )
{
}

//============================================================================
PktAnnounce& PktAnnounce::operator = ( const PktAnnounce& rhs )
{
    if( this != &rhs )
    {
        *((PktAnnBase*)this) = *((PktAnnBase*)&rhs);
    }

    return *this;
}

//============================================================================
bool PktAnnounce::addToBlob( PktBlobEntry& blob )
{
    return PktAnnBase::addToBlob( blob );
}

//============================================================================
bool PktAnnounce::extractFromBlob( PktBlobEntry& blob )
{
    return PktAnnBase::extractFromBlob( blob );
}

//============================================================================
PktAnnounce * PktAnnounce::makeAnnCopy( void )
{
	vx_assert( sizeof( PktAnnounce ) ==  getPktLength() );
	vx_assert( PKT_TYPE_ANNOUNCE == getPktType() );
	char * pTemp = new char[ getPktLength() ];
	vx_assert( pTemp );
	memcpy( pTemp, this, getPktLength() );
	return ( PktAnnounce *)pTemp;
}

//============================================================================
PktAnnounce * PktAnnounce::makeAnnReverseCopy( void )
{
	PktAnnounce * pTemp = makeAnnCopy();
	pTemp->reversePermissions();
	return pTemp;
}

//============================================================================
bool PktAnnounce::isValidPktAnn( void )
{
	return ( ( getPktLength() == sizeof( PktAnnounce ) ) &&
			 ( getPktType() == PKT_TYPE_ANNOUNCE ) );
}

//============================================================================
//! online status expires if no activity for specified time 
bool PktAnnounce::isOnlineStatusExpired( void )
{
	if( isIgnored() )
	{
		return true;
	}
	if( false == isNearbyStatusExpired() )
	{
		return false;
	}
	if( g_u64OnlineStatusTimeoutMs >= ( GetGmtTimeMs() - getTimeLastTcpContactMs() ) )
	{
		return false;
	}
	return true;
}

//============================================================================
//! determine if udp timeout has expired ( used for guest permissions and nearby status )
bool PktAnnounce::isNearbyStatusExpired( void )
{
#if ENABLE_COMPONENT_NEARBY
	if( g_u64NearbyUdpTimeoutMs <= ( GetGmtTimeMs() - getTimeLastTcpContactMs() ) )
	{
		return true;
	}
#endif // ENABLE_COMPONENT_NEARBY

	return false;
}
//============================================================================
//! if nearby then make permissions at least guest
void PktAnnounce::updateNearbyPermissions( void )
{
#if ENABLE_COMPONENT_NEARBY
	if( false == isNearbyStatusExpired() )
	{
		if( false == isIgnored() )
		{
			if( eFriendStateGuest > getMyFriendshipToHim() )
			{
				setMyFriendshipToHim(eFriendStateGuest);
			}
			if( eFriendStateGuest > getHisFriendshipToMe() )
			{
				setHisFriendshipToMe(eFriendStateGuest);
			}
		}
	}
#endif // ENABLE_COMPONENT_NEARBY
}

//============================================================================
//! dump contents of pkt announce for debug
void PktAnnounce::DebugDump( void )
{
	std::string strName;
	std::string strDesc;
    std::string strIpAddr;
    EIpAddrType addrType{ eIpAddrTypeUnknown };
	std::string strId;

	this->getMyOnlineId( strId );
	this->getOnlineIpAddress( strIpAddr, addrType );
	uint16_t u16Port = this->getOnlinePort();
	strName = this->getOnlineName();
	strDesc = this->getOnlineDescription();
	//strNetwork = this->getNetworkKey();

	LogMsg( LOG_INFO, "PktAnnounce Len %d Version #%d name %s Ip %s Port %d desc %s",
			getPktLength(),		    // packet length
			getPktVersionNum(),		// version of program
			strName.c_str(),		
			strIpAddr.c_str(),		// IPv4 of announcer
			u16Port,			    // Port announcer listens on
			strDesc.c_str()
			);		
}
