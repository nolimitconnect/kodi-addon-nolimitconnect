//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxCommon.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxSktUtil.h>

#include <NetLib/VxSktBase.h>

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

namespace
{
	IHackReportCallbackInterface* g_IHackReportCallback = nullptr;
}

//============================================================================
void VxSetHackReportCallback( IHackReportCallbackInterface* hackReportCallback )
{
	g_IHackReportCallback = hackReportCallback;
}

//============================================================================
int32_t VxReportHack( EHackerLevel hackerLevel, EHackerReason hackerReason, std::shared_ptr<VxSktBase>& sktBase, const char* pDescription, ... )
{
	char as8Buf[2048];
	va_list argList;
	va_start( argList, pDescription );
	vsnprintf( as8Buf, sizeof( as8Buf ), pDescription, argList );
	as8Buf[sizeof( as8Buf ) - 1] = 0;
	va_end( argList );

	std::string ipAddr = sktBase ? sktBase->getRemoteIpAddress() : "";

	if( g_IHackReportCallback )
	{
		g_IHackReportCallback->reportHackOffense( hackerLevel, hackerReason, ipAddr, as8Buf );
	}
	else
	{
		LogModule( eLogHackers, LOG_ERROR, "%s %s %s %s", DescribeHackerLevel( hackerLevel ), DescribeHackerReason( hackerReason ),
			sktBase->describeSktConnection().c_str(),
			as8Buf );
	}

	return 0;
}

//============================================================================
int32_t VxReportHack( EHackerLevel hackerLevel, EHackerReason hackerReason, SOCKET skt, const char* ipAddr, const char* pDescription, ... )
{
	char as8Buf[2048];
	va_list argList;
	va_start( argList, pDescription );
	vsnprintf( as8Buf, sizeof( as8Buf ), pDescription, argList );
	as8Buf[sizeof( as8Buf ) - 1] = 0;
	va_end( argList );

	if( g_IHackReportCallback )
	{
		g_IHackReportCallback->reportHackOffense( hackerLevel, hackerReason, ipAddr, as8Buf );
	}
	else
	{
		LogModule( eLogHackers, LOG_ERROR, "%s %s handle %d ip %s %s", DescribeHackerLevel( hackerLevel ), DescribeHackerReason( hackerReason ),
			skt,
			ipAddr,
			as8Buf );
	}

	return 0;
}

//============================================================================
//! generate connection key from network identity
bool GenerateConnectionKey(  VxKey *				poRetKey,		// set this key
                             VxConnectId *			poConnectId,	// network identity
                             uint16_t				cryptoPort,
                             const char*			networkName )
{
    std::string strNetName = networkName;
    std::string strIP;

	EIpAddrType addrType{ eIpAddrTypeUnknown };
	if( poConnectId->getIpAddress( strIP, addrType ) && addrType != eIpAddrTypeUnknown )
	{
		uint16_t u16Port = poConnectId->getPort();

		return GenerateConnectionKey( poRetKey, strIP, u16Port, poConnectId->getOnlineId(), cryptoPort, strNetName );
	}

	LogMsg( LOG_ERROR, "GenerateConnectionKey Invalid ip address" );
	return false;
}

//============================================================================
bool GenerateConnectionKey(  VxKey *					poRetKey,		// set this key
                             std::string&               ipAddr,
                             uint16_t                   port,
                             VxGUID&                    onlineId,
                             uint16_t					cryptoPort,
                             std::string&				networkName )
{
    uint64_t u64IdLowPart = onlineId.getVxGUIDLoPart();
    //vx_assert( u64IdLowPart );
    uint64_t u64IdHiPart = onlineId.getVxGUIDHiPart();
    //vx_assert( u64IdHiPart );
    std::string strIP = ipAddr;
    vx_assert( port );

    std::string strPwd;
    StdStringFormat( strPwd, "%d%llx%llx%s%s%d",
        port,
        u64IdLowPart,
        u64IdHiPart,
        networkName.c_str(),
        strIP.c_str(),
        cryptoPort
    );

    // LogModule( eLogSktData, LOG_VERBOSE, "GenerateConnectionKey: setting Key %s for %s:%d %d", strPwd.c_str(), strIPv4.c_str(), port, cryptoPort );
    poRetKey->setKeyFromPassword( strPwd.c_str(), (int)strPwd.size() );
    return !strIP.empty() && cryptoPort;
}

//============================================================================
PluginPermission::PluginPermission()
{
	memset( m_au8Permissions, 0, sizeof( m_au8Permissions ) );
}

//============================================================================
bool PluginPermission::addToBlob( PktBlobEntry& blob )
{
    return blob.setValue( m_au8Permissions, (int)sizeof( m_au8Permissions ) );
}

//============================================================================
bool PluginPermission::extractFromBlob( PktBlobEntry& blob )
{
    int iBufLen = sizeof( m_au8Permissions );
    return blob.getValue(  (void *)m_au8Permissions, iBufLen );
}

//============================================================================
PluginPermission& PluginPermission::operator = ( const PluginPermission& rhs )
{
    if( this != &rhs )
    {
        memcpy( m_au8Permissions, rhs.m_au8Permissions, sizeof( m_au8Permissions ) );
    }

    return *this;
}

//============================================================================
//! set type of permission user has set for givin plugin
void PluginPermission::setPluginPermission( EPluginType pluginType, EFriendState eFriendState ) 
{
    if(( pluginType > 0 ) && ( pluginType < eMaxPermissionPluginType ) )
	{
		int pluginNum = (int)(pluginType - 1);
		int byteIdx = pluginNum >> 1;
		int byteShift = pluginNum & 0x01 ? 4 : 0;
		uint8_t byteWithPerm = m_au8Permissions[ byteIdx ];
		if( byteShift )
		{
			byteWithPerm &= 0x0f;
			byteWithPerm |= (eFriendState << byteShift);
		}
		else
		{
			byteWithPerm &= 0xf0;
			byteWithPerm |= (eFriendState);
		}

		if( ( byteIdx < PERMISSION_ARRAY_SIZE ) 
			&& ( 0 <= byteIdx ) )
		{
			m_au8Permissions[ byteIdx ] = byteWithPerm;
		}
		else
		{
			LogMsg( LOG_ERROR, "setPluginPermission index out of range %d", byteIdx );
		}
	}
    else
    {
        LogMsg( LOG_ERROR, "setPluginPermission invalid plugin %d", pluginType );
        vx_assert( false );
    }
}

//============================================================================
void PluginPermission::setPluginPermissionsToDefaultValues( void )	
{ 
	memset( m_au8Permissions, 0, sizeof( m_au8Permissions ) );

    setPluginPermission( ePluginTypeAboutMePageServer, eFriendStateGuest );
	setPluginPermission( ePluginTypeStoryboardServer, eFriendStateGuest );
    setPluginPermission( ePluginTypeCamServer, eFriendStateIgnore );
    setPluginPermission( ePluginTypeFileShareServer, eFriendStateIgnore );

    setPluginPermission( ePluginTypePersonFileXfer, eFriendStateFriend );
    setPluginPermission( ePluginTypeMessenger, eFriendStateGuest );
    setPluginPermission( ePluginTypeTruthOrDare, eFriendStateFriend );
    setPluginPermission( ePluginTypeVideoChat, eFriendStateFriend );
    setPluginPermission( ePluginTypeVoicePhone, eFriendStateFriend );

    setPluginPermission( ePluginTypePushToTalk, eFriendStateIgnore );

    setPluginPermission( ePluginTypeFriendRequest, eFriendStateAnonymous );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientPeerUser, eFriendStateGuest );
	setPluginPermission( ePluginTypeHostPeerUser, eFriendStateGuest );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientConnectTest, eFriendStateIgnore );
	setPluginPermission( ePluginTypeHostConnectTest, eFriendStateIgnore );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientChatRoom, eFriendStateGuest );
	setPluginPermission( ePluginTypeHostChatRoom, eFriendStateIgnore );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientGroup, eFriendStateGuest );
	setPluginPermission( ePluginTypeHostGroup, eFriendStateIgnore );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientNetwork, eFriendStateIgnore );
	setPluginPermission( ePluginTypeHostNetwork, eFriendStateIgnore );

    // clients outside of published permissions setPluginPermission( ePluginTypeClientRandomConnect, eFriendStateGuest );
	setPluginPermission( ePluginTypeHostRandomConnect, eFriendStateIgnore );
} 

//============================================================================
//============================================================================

#define IS_ADMIN_AVAIL_CHAT_ROOM_FLAG			0x01	
#define IS_ADMIN_AVAIL_GROUP_FLAG				0x02	
#define IS_ADMIN_AVAIL_RANDOM_CONNECT_FLAG		0x04	

#define IS_JOINED_CHAT_ROOM_FLAG				0x01	
#define IS_JOINED_GROUP_FLAG					0x02	
#define IS_JOINED_RANDOM_CONNECT_FLAG			0x04	

//============================================================================
VxNetIdent::VxNetIdent()
: m_u16AppVersion( htons( VxGetAppVersionShort() ) )	
{
}

//============================================================================
VxNetIdent::VxNetIdent(const VxNetIdent &rhs )
: VxConnectInfo( rhs )
, PluginPermission( rhs )
, VxOnlineStatusFlags( rhs )
, m_AdminAvailFlags( rhs.m_AdminAvailFlags )
, m_JoinedFlags( rhs.m_JoinedFlags )
, m_NetIdentRes1( rhs.m_NetIdentRes1 )
, m_u16AppVersion( rhs.m_u16AppVersion )
, m_u16PingTimeMs( rhs.m_u16PingTimeMs )
, m_LastSessionTimeGmtMs( rhs.m_LastSessionTimeGmtMs )
, m_GroupieInfoModifiedTimeMs( rhs.m_GroupieInfoModifiedTimeMs )

, m_TruthAcceptCnt( rhs.m_TruthAcceptCnt )
, m_TruthRejectCnt( rhs.m_TruthRejectCnt )
, m_DareAcceptCnt( rhs.m_DareAcceptCnt )
, m_DareRejectCnt( rhs.m_DareRejectCnt )

, m_NetIdentRes2( rhs.m_NetIdentRes2 )  
, m_NetIdentRes3( rhs.m_NetIdentRes3 )
, m_NetIdentRes4( rhs.m_NetIdentRes4 )
, m_NetIdentRes5( rhs.m_NetIdentRes4 )
{
}

//============================================================================
bool VxNetIdent::addToBlob( PktBlobEntry& blob )
{
	uint8_t startMagicNum = 98;
	bool result = blob.setValue( startMagicNum );
	result &= VxConnectInfo::addToBlob( blob );
	result &= PluginPermission::addToBlob( blob );
	result &= VxOnlineStatusFlags::addToBlob( blob );

	result &= blob.setValue( m_AdminAvailFlags );
	result &= blob.setValue( m_JoinedFlags );
	result &= blob.setValue( m_NetIdentRes1 );
	result &= blob.setValue( m_u16AppVersion );
	result &= blob.setValue( m_u16PingTimeMs );

	result &= blob.setValue( m_LastSessionTimeGmtMs );
	result &= blob.setValue( m_GroupieInfoModifiedTimeMs );

	result &= blob.setValue( m_TruthAcceptCnt );
	result &= blob.setValue( m_TruthRejectCnt );
	result &= blob.setValue( m_DareAcceptCnt );
	result &= blob.setValue( m_DareRejectCnt );

	result &= blob.setValue( m_NetIdentRes2 );
	result &= blob.setValue( m_NetIdentRes3 );
	result &= blob.setValue( m_NetIdentRes4 );
	result &= blob.setValue( m_NetIdentRes5 );

	uint8_t stopMagicNum = 99;
	result &= blob.setValue( stopMagicNum );
	return result;
}

//============================================================================
bool VxNetIdent::extractFromBlob( PktBlobEntry& blob )
{
	uint8_t startMagicNum;
	bool result = blob.getValue( startMagicNum );
	if( !result || startMagicNum != 98 )
	{
		LogMsg( LOG_ERROR, "VxNetIdent::%s startMagicNum not valid", __func__ );
		vx_assert( false );
		return false;
	}

	result &= VxConnectInfo::extractFromBlob( blob );
	result &= PluginPermission::extractFromBlob( blob );
	result &= VxOnlineStatusFlags::extractFromBlob( blob );

	result &= blob.getValue( m_AdminAvailFlags );
	result &= blob.getValue( m_JoinedFlags );
	result &= blob.getValue( m_NetIdentRes1 );

	result &= blob.getValue( m_u16AppVersion );
	result &= blob.getValue( m_u16PingTimeMs );

	result &= blob.getValue( m_LastSessionTimeGmtMs );
	result &= blob.getValue( m_GroupieInfoModifiedTimeMs );

	result &= blob.getValue( m_TruthAcceptCnt );
	result &= blob.getValue( m_TruthRejectCnt );
	result &= blob.getValue( m_DareAcceptCnt );
	result &= blob.getValue( m_DareRejectCnt );

	result &= blob.getValue( m_NetIdentRes2 );
	result &= blob.getValue( m_NetIdentRes3 );
	result &= blob.getValue( m_NetIdentRes4 );
	result &= blob.getValue( m_NetIdentRes5 );

	uint8_t stopMagicNum;
	result &= blob.getValue( stopMagicNum );
	if( !result || stopMagicNum != 99 )
	{
		LogMsg( LOG_ERROR, "VxNetIdent::%s stopMagicNum not valid", __func__ );
		return false;
	}

	return result;
}

//============================================================================
//! copy operator
VxNetIdent& VxNetIdent::operator =( const VxNetIdent& rhs )
{
	if( this != &rhs )
	{
		*( (VxConnectInfo*)this ) = *( (VxConnectInfo*)&rhs );
		*( (PluginPermission*)this ) = *( (PluginPermission*)&rhs );
		*( (VxOnlineStatusFlags*)this ) = *( (VxOnlineStatusFlags*)&rhs );

		m_AdminAvailFlags = rhs.m_AdminAvailFlags;
		m_JoinedFlags = rhs.m_JoinedFlags;
		m_NetIdentRes1 = rhs.m_NetIdentRes1;
		m_u16AppVersion = rhs.m_u16AppVersion;
		m_u16PingTimeMs = rhs.m_u16PingTimeMs;

		m_TruthAcceptCnt = rhs.m_TruthAcceptCnt;
		m_TruthRejectCnt = rhs.m_TruthRejectCnt;
		m_DareAcceptCnt = rhs.m_DareAcceptCnt;
		m_DareRejectCnt = rhs.m_DareRejectCnt;

		m_NetIdentRes2 = rhs.m_NetIdentRes2;
		m_NetIdentRes3 = rhs.m_NetIdentRes3;
		m_NetIdentRes4 = rhs.m_NetIdentRes4;
		m_NetIdentRes5 = rhs.m_NetIdentRes5;
		m_LastSessionTimeGmtMs = rhs.m_LastSessionTimeGmtMs;
	}

	return *this;
}


//============================================================================
bool VxNetIdent::isMyself( void )
{
	return GetPtoPEngine().getMyOnlineId() == getMyOnlineId();
}

//============================================================================
bool VxNetIdent::isDirectConnected( void )
{
	return GetPtoPEngine().getConnectIdListMgr().isDirectConnected( getMyOnlineId() );
}

//============================================================================
bool VxNetIdent::isRelayed( void )
{
	return GetPtoPEngine().getConnectIdListMgr().isRelayed( getMyOnlineId() );
}

//============================================================================
bool VxNetIdent::canDirectConnectToUser( void )
{
	return !requiresRelay() || isDirectConnected();
}

//============================================================================
//! return true if identity matches
bool VxNetIdent::isVxNetIdentMatch( const VxNetIdent& oOtherIdent ) const
{
	return ( *( (VxGUID*)&oOtherIdent.m_DirectConnectId ) == *( (VxGUID*)&m_DirectConnectId ) );
}

//============================================================================
bool VxNetIdent::operator ==( const VxNetIdent& a ) const
{
	return this->isVxNetIdentMatch( a );
}

//============================================================================
//! not equal operator
bool VxNetIdent::operator != ( const VxNetIdent& a ) const
{
	return !( this->isVxNetIdentMatch( a ) );
}

//============================================================================
void VxNetIdent::clearIsAdminAvail( void )
{
	m_AdminAvailFlags = 0;
}

//============================================================================
void VxNetIdent::setIsAdminAvail( EHostType hostType, bool isAdminAvail )
{
	uint8_t hostFlag{ 0 };
	switch( hostType )
	{
	case eHostTypeChatRoom:
		hostFlag = IS_ADMIN_AVAIL_CHAT_ROOM_FLAG;
		break;
	case eHostTypeGroup:
		hostFlag = IS_ADMIN_AVAIL_GROUP_FLAG;
		break;
	case eHostTypeRandomConnect:
		hostFlag = IS_ADMIN_AVAIL_RANDOM_CONNECT_FLAG;
		break;
	default:
		LogMsg( LOG_ERROR, "VxNetIdent:setIsAdminAvail invalid host type %d", hostType );
		return;
	}

	if( hostFlag )
	{
		if( isAdminAvail )
		{
			if( !( m_AdminAvailFlags & hostFlag ) )
			{
				m_AdminAvailFlags |= hostFlag;
				LogMsg( LOG_VERBOSE, "VxNetIdent:setIsAdminAvail %s admin IS available host type %s", getOnlineName(), DescribeHostType( hostType ) );
			}
		}
		else
		{
			if( m_AdminAvailFlags & hostFlag )
			{
				m_AdminAvailFlags &= ~hostFlag;
				LogMsg( LOG_VERBOSE, "VxNetIdent:setIsAdminAvail %s NOT available host type %s", getOnlineName(), DescribeHostType( hostType ) );
			}
		}
	}
}

//============================================================================
bool VxNetIdent::getIsAdminAvail( EHostType hostType )
{
	uint8_t hostFlag{ 0 };
	switch( hostType )
	{
	case eHostTypeChatRoom:
		hostFlag = IS_ADMIN_AVAIL_CHAT_ROOM_FLAG;
		break;
	case eHostTypeGroup:
		hostFlag = IS_ADMIN_AVAIL_GROUP_FLAG;
		break;
	case eHostTypeRandomConnect:
		hostFlag = IS_ADMIN_AVAIL_RANDOM_CONNECT_FLAG;
		break;
	default:
		LogMsg( LOG_ERROR, "VxNetIdent:getIsAdminAvail invalid host type %d", hostType );
	}

	return hostFlag & m_AdminAvailFlags;
}


//============================================================================
void VxNetIdent::clearIsJoined( void )
{
	m_JoinedFlags = 0;
}

//============================================================================
void VxNetIdent::setIsJoined( EHostType hostType, bool isJoined )
{
	uint8_t hostFlag{ 0 };
	switch( hostType )
	{
	case eHostTypeChatRoom:
		hostFlag = IS_JOINED_CHAT_ROOM_FLAG;
		break;
	case eHostTypeGroup:
		hostFlag = IS_JOINED_GROUP_FLAG;
		break;
	case eHostTypeRandomConnect:
		hostFlag = IS_JOINED_RANDOM_CONNECT_FLAG;
		break;
	default:
		LogMsg( LOG_ERROR, "VxNetIdent:setIsJoined invalid host type %d", hostType );
		return;
	}

	if( hostFlag )
	{
		if( isJoined )
		{
			if( !( m_JoinedFlags & hostFlag ) )
			{
				m_JoinedFlags |= hostFlag;
                if(LogEnabled(eLogUsers))LogModule( eLogUsers, LOG_VERBOSE, "VxNetIdent:setIsJoined %s joined host type %s", getOnlineName(), DescribeHostType( hostType ) );
			}
		}
		else
		{
			if( m_JoinedFlags & hostFlag )
			{
				m_JoinedFlags &= ~hostFlag;
				if(LogEnabled(eLogUsers))LogModule( eLogUsers, LOG_VERBOSE, "VxNetIdent:setIsJoined %s NOT joined host type %s", getOnlineName(), DescribeHostType( hostType ) );
			}
		}
	}
}

//============================================================================
bool VxNetIdent::getIsJoined( EHostType hostType )
{
	uint8_t hostFlag{ 0 };
	switch( hostType )
	{
	case eHostTypeChatRoom:
		hostFlag = IS_JOINED_CHAT_ROOM_FLAG;
		break;
	case eHostTypeGroup:
		hostFlag = IS_JOINED_GROUP_FLAG;
		break;
	case eHostTypeRandomConnect:
		hostFlag = IS_JOINED_RANDOM_CONNECT_FLAG;
		break;
	default:
		LogMsg( LOG_ERROR, "VxNetIdent:getIsJoined invalid host type %d", hostType );
	}

	return hostFlag & m_JoinedFlags;
}

//============================================================================
bool VxNetIdent::isJoinedAny( void )
{
	return m_JoinedFlags != 0;
}

//============================================================================
bool VxNetIdent::isValidNetIdent()
{
	bool result = getMyOnlineId().isValid();

	result &= isOnlineNameValid();

	return result;
} 

//============================================================================
bool VxNetIdent::isOnlineNameValid( void )
{
    bool result = !(m_OnlineName[0] == 0);
	if( result )
	{
		int asciiCnt = 0;
		bool foundTerminator{ false };
		bool invalidChar{ false };
		for( int i = 0; i < MAX_ONLINE_NAME_LEN; i++ )
		{
			if( 0 == m_OnlineName[i] )
			{
				foundTerminator = true;
				break;
			}
			else if( isascii( m_OnlineName[i] ) )
			{
				asciiCnt++;
			}
			else
			{
				// invalid char
				invalidChar = true;
				break;
			}
		}

		result &= !invalidChar && foundTerminator && asciiCnt >= 3;
	}

	return result;
}

//============================================================================
void VxNetIdent::setPingTimeMs( uint16_t pingTime )
{
	m_u16PingTimeMs = htons( pingTime );
}

//============================================================================
uint16_t VxNetIdent::getPingTimeMs( void )
{
	return ntohs( m_u16PingTimeMs );
}

//============================================================================
EPluginAccess	VxNetIdent::getHisAccessPermissionFromMe( EPluginType pluginType )
{
	EFriendState friendState = getMyFriendshipToHim();

	return getPluginAccessState( pluginType, friendState );
}

//============================================================================
EPluginAccess VxNetIdent::getMyAccessPermissionFromHim( EPluginType pluginType )
{
	EFriendState friendState = getHisFriendshipToMe();

	EPluginAccess accessState = getPluginAccessState( pluginType, friendState );
	if( ePluginAccessOk == accessState )
	{
		if( ( ePluginTypeFileShareServer == pluginType ) 
			&& ( false == hasSharedFiles() ) )
		{
			// no files shared
			return ePluginAccessInactive;
		}

		if( ( ePluginTypeCamServer == pluginType ) 
			&& ( false == hasSharedWebCam() ) )
		{
			// no shared web cam
			return ePluginAccessInactive;
		}

		if( ( ePluginTypeAboutMePageServer == pluginType )
			|| ( ePluginTypeStoryboardServer == pluginType ) )
		{
			if( false == isOnline() )
			{
				accessState = ePluginAccessRequiresOnline;
			}
		}
		else if( ePluginTypeMessenger != pluginType )
		{
			if( false == isOnline() )
			{
				accessState = ePluginAccessRequiresOnline;
			}
		}		
	}

	return accessState;
}

//============================================================================
bool VxNetIdent::isHisAccessAllowedFromMe( EPluginType pluginType )
{
	if( GetPtoPEngine().getIsPluginInTestState( pluginType, getMyOnlineId() ) )
	{
		return true;
	}

	EFriendState friendState = this->getMyFriendshipToHim();

    return ( ePluginAccessOk == GetPtoPEngine().getMyPktAnnounce().getPluginAccessState( pluginType, friendState ) );
}

//============================================================================
bool VxNetIdent::isMyAccessAllowedFromHim( EPluginType pluginType )
{
	if( GetPtoPEngine().getIsPluginInTestState( pluginType, getMyOnlineId() ) )
	{
		return true;
	}

	EFriendState friendState = this->getHisFriendshipToMe();
	return ( ePluginAccessOk == getPluginAccessState( pluginType, friendState ) );
}

//============================================================================
EPluginAccess VxNetIdent::getPluginAccessState( EPluginType pluginType, EFriendState eHisPermissionToMe )
{
	if( eFriendStateIgnore == eHisPermissionToMe )
	{
		return ePluginAccessIgnored;
	}

	if( eFriendStateAnonymous == eHisPermissionToMe && isJoinedAny() )
	{
		// upgrade to guest since is joined
		eHisPermissionToMe = eFriendStateGuest;
	}

	EFriendState ePermissionLevel = this->getPluginPermission( pluginType );
	if( eFriendStateIgnore == ePermissionLevel )
	{
		return ePluginAccessDisabled;
	}

	if( ePermissionLevel > eHisPermissionToMe )
	{
		return ePluginAccessLocked;
	}

	if( (ePluginTypeFileShareServer == pluginType) && 
		(false == hasSharedFiles()) )
	{
		// no files shared
		return ePluginAccessInactive;
	}

	if( (ePluginTypeCamServer == pluginType) && 
		(false == hasSharedWebCam()) )
	{
		// no files shared
		return ePluginAccessInactive;
	}

	return ePluginAccessOk;
}

//============================================================================
bool VxNetIdent::canRequestJoin( EHostType hostType )
{
	if( requiresRelay() )
	{
		return false;
	}

	EPluginType pluginType = HostTypeToHostPlugin( hostType );
	EFriendState pluginPermissionLevel = getPluginPermission( pluginType );
	return pluginPermissionLevel != eFriendStateIgnore && getHisFriendshipToMe() != eFriendStateIgnore &&
		getMyFriendshipToHim() != eFriendStateIgnore;
}

//============================================================================
bool VxNetIdent::canJoinImmediate( EHostType hostType ) // request to join will be granted immediate because have sufficient permission
{
	if( requiresRelay() )
	{
		return false;
	}

	EPluginType pluginType = HostTypeToHostPlugin( hostType );
	EFriendState pluginPermissionLevel = getPluginPermission( pluginType );
	return pluginPermissionLevel != eFriendStateIgnore && getHisFriendshipToMe() != eFriendStateIgnore &&
		getMyFriendshipToHim() != eFriendStateIgnore && getHisFriendshipToMe() >= pluginPermissionLevel;
}

//============================================================================
//! dump identity
void VxNetIdent::debugDumpIdent( void )
{
	std::string strIP; 
	EIpAddrType addrType{ eIpAddrTypeUnknown };
	m_DirectConnectId.getIpAddress( strIP, addrType );

	LogMsg( LOG_INFO, "Ident %s id %s my friendship %s his friendship %s search 0x%x ip %s port %d proxy flags 0x%x ",
		getOnlineName(),
		getMyOnlineId().describeVxGUID().c_str(),
		DescribeFriendState(getMyFriendshipToHim()),
		DescribeFriendState(getHisFriendshipToMe()),
		getSearchFlags(),
		strIP.c_str(),
		m_DirectConnectId.getPort(),
		m_u8RelayFlags
		);
}

//============================================================================
std::string VxNetIdent::describeUser( void )
{
	std::string userDesc = getOnlineName();
	userDesc += " id ";
	userDesc += getMyOnlineId().toOnlineIdString();
	return userDesc;
}

//============================================================================
bool VxNetIdent::userIsHosting( enum EHostType hostType )
{
	switch( hostType )
	{
	case eHostTypeConnectTest:
		return getHisAccessPermissionFromMe( ePluginTypeHostConnectTest ) != ePluginAccessDisabled;
	case eHostTypeNetwork:
		return getHisAccessPermissionFromMe( ePluginTypeHostNetwork ) != ePluginAccessDisabled;
    case eHostTypeGroup:
		return getHisAccessPermissionFromMe( ePluginTypeHostGroup ) != ePluginAccessDisabled;
    case eHostTypeChatRoom:
		return getHisAccessPermissionFromMe( ePluginTypeHostChatRoom ) != ePluginAccessDisabled;
    case eHostTypeRandomConnect:
		return getHisAccessPermissionFromMe( ePluginTypeHostRandomConnect ) != ePluginAccessDisabled;

	default:
		return false;
	}
}

//============================================================================
bool VxNetIdent::requiresAnOpenPort( void )
{
	return userIsHosting( eHostTypeNetwork ) || 
		userIsHosting( eHostTypeGroup ) || 
		userIsHosting( eHostTypeChatRoom ) || 
		userIsHosting( eHostTypeRandomConnect ) || 
		userIsHosting( eHostTypeConnectTest );
}

//============================================================================
bool VxNetIdent::isPluginEnabled( EPluginType ePlugin )
{
	return ( eFriendStateIgnore == getPluginPermission( ePlugin ) ) ? false : true;
}

//============================================================================
//! get type of permission user has set for givin plugin
EFriendState VxNetIdent::getPluginPermission( EPluginType pluginType )
{
	if( ( pluginType > 0 ) && ( pluginType < eMaxPermissionPluginType ) )
	{
		int pluginNum = (int)( pluginType - 1 );
		int byteIdx = pluginNum >> 1;
		int byteShift = pluginNum & 0x01 ? 4 : 0;
		uint8_t byteWithPerm = m_au8Permissions[byteIdx];

		EFriendState friendState = (EFriendState)( ( byteWithPerm >> byteShift ) & 0xf );

		return friendState;
	}
	else
	{
		if( pluginType == ePluginTypeClientChatRoom ||
			pluginType == ePluginTypeClientGroup ||
			pluginType == ePluginTypeClientRandomConnect )
		{
			// not a published permission but assume has at least guest if in group
			if( isJoinedAny() )
			{
				return eFriendStateGuest;
			}
		}
	}

	return eFriendStateIgnore;
}

//============================================================================
//! if was anonymouse upgrade to guest friendship
void VxNetIdent::upgradeToGuestFriendship( void )
{
	if( isAnonymous() )
	{
		setMyFriendshipToHim( eFriendStateGuest );
	}
}

//! return true if is ignored
bool			VxNetIdent::isIgnored() { return eFriendStateIgnore == getMyFriendshipToHim(); }
//! return true if is Anonymous
bool			VxNetIdent::isAnonymous() { return eFriendStateAnonymous == getMyFriendshipToHim(); }
//! return true if is VxNetIdent
bool			VxNetIdent::isGuest() { return eFriendStateGuest == getMyFriendshipToHim(); }
//! return true if is Friend
bool			VxNetIdent::isFriend() { return eFriendStateFriend == getMyFriendshipToHim(); }
//! return true if is Administrator
bool			VxNetIdent::isAdministrator() { return eFriendStateAdmin == getMyFriendshipToHim(); }
//! set my permissions to him as ignored
void			VxNetIdent::makeIgnored() { setMyFriendshipToHim( eFriendStateIgnore ); }
//! set my permissions to him as Anonymous
void			VxNetIdent::makeAnonymous() { setMyFriendshipToHim( eFriendStateAnonymous ); }
//! set my permissions to him as Guest
void			VxNetIdent::makeGuest() { setMyFriendshipToHim( eFriendStateGuest ); }
//! set my permissions to him as Friend
void			VxNetIdent::makeFriend() { setMyFriendshipToHim( eFriendStateFriend ); }
//! set my permissions to him as Administrator
void			VxNetIdent::makeAdministrator() { setMyFriendshipToHim( eFriendStateAdmin ); }
//! wants to be friend
bool			VxNetIdent::wantsToBeFriend() { return eFriendStateFriend == getHisFriendshipToMe(); }
//! wants to be friend
bool			VxNetIdent::wantsToBeAdministrator() { return eFriendStateAdmin == getHisFriendshipToMe(); }

//! set permission level I have given to friend
void			VxNetIdent::setMyFriendshipToHim( EFriendState eFriendState )
{
	m_u8FriendMatch &= 0xf0;
	m_u8FriendMatch |= eFriendState;
}

//! get permission level I have given to friend
EFriendState VxNetIdent::getMyFriendshipToHim( void )
{
	EFriendState friendState = (EFriendState)( m_u8FriendMatch & 0x0f );
	if( eFriendStateAnonymous == friendState && isJoinedAny() )
	{
		friendState = eFriendStateGuest;
	}

	return friendState;
}

//! set permission level he has given to me
void VxNetIdent::setHisFriendshipToMe( EFriendState eFriendState )
{
	m_u8FriendMatch &= 0x0f;
	m_u8FriendMatch |= ( eFriendState << 4 );
}

//! get permission level he has given to me
EFriendState VxNetIdent::getHisFriendshipToMe( void )
{
	EFriendState friendState = (EFriendState)( ( m_u8FriendMatch >> 4 ) & 0x0f );
	if( eFriendStateAnonymous == friendState && isJoinedAny() )
	{
		friendState = eFriendStateGuest;
	}

	return friendState;
}

//! reverse the permissions
void VxNetIdent::reversePermissions( void )
{
	uint8_t u8TmpPermission = m_u8FriendMatch << 4;
	m_u8FriendMatch = u8TmpPermission | ( ( m_u8FriendMatch >> 4 ) & 0x0f );
}
//! return string with friend state He has given Me
void VxNetIdent::describeHisFriendshipToMe( std::string& strRetPermission ) { strRetPermission = DescribeFriendState( getHisFriendshipToMe() ); }
//! return string with friend state He has given Me
const char* VxNetIdent::describeHisFriendshipToMe( void ) { return DescribeFriendState( getHisFriendshipToMe() ); }
//! return string with friend state I have given Him
void VxNetIdent::describeMyFriendshipToHim( std::string& strRetPermission ) { strRetPermission = DescribeFriendState( getMyFriendshipToHim() ); }
//! return string with friend state I have given Him
const char* VxNetIdent::describeMyFriendshipToHim( void ) { return DescribeFriendState( getMyFriendshipToHim() ); }

void VxNetIdent::dumpPermissions( bool justHosts )
{
    for( int i = 1; i < eMaxPermissionPluginType; i++ )
    {
        EPluginType pluginType = (EPluginType)i;
        if( !justHosts || (justHosts && PluginShouldAnnounceToNetwork( pluginType ) ) )
        {
			EFriendState permission = getPluginPermission( pluginType );
            LogMsg( LOG_VERBOSE, "VxNetIdent::dumpPermissions %d - %s permission %s",
                    i, DescribePluginType( pluginType ), DescribeFriendState( permission ) );
        }
    }
}
