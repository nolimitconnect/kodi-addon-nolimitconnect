//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginMgr.h"
#include <GuiInterface/IToGui.h>

#include <MediaProcessor/MediaProcessor.h>

#include <Plugins/PluginAboutMePageClient.h>
#include <Plugins/PluginAboutMePageServer.h>

#include <Plugins/PluginCamClient.h>
#include <Plugins/PluginCamServer.h>

#include <Plugins/PluginChatRoomClient.h>
#include <Plugins/PluginChatRoomHost.h>

#include <Plugins/PluginConnectionTestClient.h>
#include <Plugins/PluginConnectionTestHost.h>

#include <Plugins/PluginInvalid.h>

#include <Plugins/PluginFileShareClient.h>
#include <Plugins/PluginFileShareServer.h>

#include <Plugins/PluginFriendRequest.h>

#include <Plugins/PluginGroupClient.h>
#include <Plugins/PluginGroupHost.h>

#include <Plugins/PluginMessenger.h>

#include <Plugins/PluginNetworkHost.h>

#include <Plugins/PluginPeerUserClient.h>
#include <Plugins/PluginPeerUserHost.h>

#include <Plugins/PluginPersonFileXfer.h>

#include <Plugins/PluginPushToTalk.h>

#include <Plugins/PluginRandomConnectClient.h>
#include <Plugins/PluginRandomConnectHost.h>

#include <Plugins/PluginStoryboardServer.h>
#include <Plugins/PluginStoryboardClient.h>

#include <Plugins/PluginTruthOrDare.h>
#include <Plugins/PluginVideoPhone.h>
#include <Plugins/PluginVoicePhone.h>

#include <Plugins/PluginLibraryServer.h>

#include <P2PEngine/P2PEngine.h>

#include <BigListLib/BigListInfo.h>
#include <NetServices/NetServiceHdr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktTypes.h>

#include <string.h>

namespace
{
	bool IsAssetTransferPacketType( uint8_t pktType )
	{
		return pktType == PKT_TYPE_ASSET_SEND_REQ ||
			pktType == PKT_TYPE_ASSET_SEND_REPLY ||
			pktType == PKT_TYPE_ASSET_CHUNK_REQ ||
			pktType == PKT_TYPE_ASSET_CHUNK_REPLY ||
			pktType == PKT_TYPE_ASSET_SEND_COMPLETE_REQ ||
			pktType == PKT_TYPE_ASSET_SEND_COMPLETE_REPLY ||
			pktType == PKT_TYPE_ASSET_XFER_ERR;
	}
}
#include <stdarg.h>
#include <stdio.h>

#include <array>

//============================================================================
PluginMgr::PluginMgr( P2PEngine& engine )
: m_Engine( engine )
, m_BigListMgr( engine.getBigListMgr() )
, m_PktAnn( engine.getMyPktAnnounce() )
, m_PluginMgrInitialized( false )
, m_NetServiceUtils( engine )
{
}

//============================================================================
IToGui&	PluginMgr::getToGui( void )
{ 
    return m_Engine.getToGui(); 
}

//============================================================================
PluginMgr::~PluginMgr()
{
	std::vector<PluginBase*>::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		PluginBase* poPlugin = *(iter);
		delete poPlugin;		
	}

	m_aoPlugins.clear();
}

//============================================================================
void PluginMgr::pluginMgrStartup( void )
{
    uint32_t startTime = (uint32_t)GetApplicationAliveMs();
	LogMsg( LOG_VERBOSE, "pluginMgrStartup start %d ms", startTime );

	PluginBase* poPlugin;
	// invalid
	poPlugin = new PluginInvalid( m_Engine, *this, &this->m_PktAnn, ePluginTypeInvalid );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create library plugin" );
    m_aoPlugins.emplace_back( &m_Engine.getPluginLibraryServer() );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create file xfer plugin" );
    poPlugin = new PluginPersonFileXfer( m_Engine, *this, &this->m_PktAnn, ePluginTypePersonFileXfer );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create messenger plugin" );
    poPlugin = new PluginMessenger( m_Engine, *this, &this->m_PktAnn, ePluginTypeMessenger );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create about me server plugin" );
	poPlugin = new PluginAboutMePageServer( m_Engine, *this, &this->m_PktAnn, ePluginTypeAboutMePageServer );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create client peer user plugin" );
    poPlugin = new PluginPeerUserClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeClientPeerUser );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create host peer user plugin" );
    poPlugin = new PluginPeerUserHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostPeerUser );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create connection test client plugin" );
    poPlugin = new PluginConnectionTestClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeClientConnectTest );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create connection test host plugin" );
    poPlugin = new PluginConnectionTestHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostConnectTest );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create file share plugin" );
    m_aoPlugins.emplace_back( &m_Engine.getPluginFileShareServer() );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create file share client plugin" );
	poPlugin = new PluginFileShareClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeFileShareClient );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create chat room client plugin" );
    poPlugin = new PluginChatRoomClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeClientChatRoom );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create host chat room plugin" );
    poPlugin = new PluginChatRoomHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostChatRoom );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create host client plugin" );
    poPlugin = new PluginGroupClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeClientGroup );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create host group plugin" );
    poPlugin = new PluginGroupHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostGroup );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create host network plugin" );
    poPlugin = new PluginNetworkHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostNetwork );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create random connect client plugin" );
    poPlugin = new PluginRandomConnectClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeClientRandomConnect );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create random connect host plugin" );
    poPlugin = new PluginRandomConnectHost( m_Engine, *this, &this->m_PktAnn, ePluginTypeHostRandomConnect );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create storyboard server plugin" );
    poPlugin = new PluginStoryboardServer( m_Engine, *this, &this->m_PktAnn, ePluginTypeStoryboardServer );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create cam server plugin" );
	poPlugin = new PluginCamServer( m_Engine, *this, &this->m_PktAnn, ePluginTypeCamServer );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create cam client plugin" );
	poPlugin = new PluginCamClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeCamClient );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create voice phone plugin" );
	poPlugin = new PluginVoicePhone( m_Engine, *this, &this->m_PktAnn, ePluginTypeVoicePhone );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create push to talk plugin" );
	poPlugin = new PluginPushToTalk( m_Engine, *this, &this->m_PktAnn, ePluginTypePushToTalk );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create video phone plugin" );
	poPlugin = new PluginVideoPhone( m_Engine, *this, &this->m_PktAnn, ePluginTypeVideoChat );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create truth or dare plugin" );
	poPlugin = new PluginTruthOrDare( m_Engine, *this, &this->m_PktAnn, ePluginTypeTruthOrDare );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create about me viewer plugin" );
	poPlugin = new PluginAboutMePageClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeAboutMePageClient );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup create about me viewer plugin" );
	poPlugin = new PluginStoryboardClient( m_Engine, *this, &this->m_PktAnn, ePluginTypeStoryboardClient );
    m_aoPlugins.emplace_back( poPlugin );

    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup adding net services" );
	// net services pre created by engine
    m_aoPlugins.emplace_back( &m_Engine.getPluginNetServices() );

	LogModule( eLogStartup, LOG_VERBOSE, "pluginMgrStartup create friend request plugin" );
    poPlugin = new PluginFriendRequest( m_Engine, *this, &this->m_PktAnn, ePluginTypeFriendRequest );
    m_aoPlugins.emplace_back( poPlugin );

	m_PluginMgrInitialized = true;

	m_Engine.getConnectIdListMgr().wantConnectIdListCallback( this, true );

    uint32_t endTime = ( uint32_t)GetApplicationAliveMs();
    LogModule( eLogStartup, LOG_INFO, "pluginMgrStartup done in %d ms at %d ms", endTime - startTime, endTime );
}

//============================================================================
void PluginMgr::pluginMgrShutdown( void )
{
	if( m_PluginMgrInitialized )
	{
		m_Engine.getConnectIdListMgr().wantConnectIdListCallback( this, false );
		m_PluginMgrInitialized = false;
		std::vector<PluginBase*>::iterator iter;
		for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
		{
			PluginBase* poPlugin = *(iter);
			poPlugin->pluginShutdown();
		}
	}
}

//============================================================================
//! get plugin state 
EAppState PluginMgr::getPluginState( EPluginType pluginType )								
{ 
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		return plugin->getPluginState();
	}

	return eAppStateInvalid;
}

//============================================================================
PluginBase* PluginMgr::getPlugin( EPluginType pluginType )
{
	for( auto iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		PluginBase* plugin = *iter;
		if( pluginType == plugin->getPluginType() )
		{
			return *(iter);
		}
	}

	LogMsg( LOG_ERROR, "PluginMgr::getPlugin plugin type %d %s out of range", pluginType, DescribePluginType( pluginType ) );
	vx_assert( false );
	return NULL;
}

//============================================================================
//! set plugin state 
void PluginMgr::setPluginState( EPluginType pluginType, EAppState ePluginState )		
{ 
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		plugin->setPluginState( ePluginState );
	}
}

//============================================================================
EFriendState PluginMgr::getPluginPermission( EPluginType pluginType )							
{ 
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		return plugin->getPluginPermission(); 
	}

	return eFriendStateIgnore;
}

//============================================================================
void PluginMgr::setPluginPermission( EPluginType pluginType, EFriendState ePluginPermission )	
{ 
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		plugin->setPluginPermission( ePluginPermission ); 
	}
}

//============================================================================
bool PluginMgr::setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs )
{
    bool result = false;
    PluginBase* plugin = getPlugin( pluginSetting.getPluginType() );
    if( plugin )
    {
        result = plugin->setPluginSetting( pluginSetting, modifiedTimeMs );
    }

    return result;
}

//============================================================================
void PluginMgr::onPluginSettingChange( PluginSetting& pluginSetting, int64_t modifiedTimeMs )
{
    PluginBase* plugin = getPlugin( pluginSetting.getPluginType() );
    if( plugin )
    {
        plugin->setPluginSetting( pluginSetting, modifiedTimeMs );
    }
}

//============================================================================
void PluginMgr::pluginApiLog( EPluginType pluginType, const char* pMsg, ... )
{
	std::array<char, 2048> szBuffer;
	va_list argList;
	va_start(argList, pMsg);
	vsnprintf( szBuffer.data(), 2048, pMsg, argList);
	va_end(argList);
	LogMsg( (pluginType << 16) | LOG_INFO, "Plugin %d %s", (int)pluginType, szBuffer.data() );
}

//============================================================================
void PluginMgr::handleFirstNetServiceConnection( std::shared_ptr<VxSktBase>& sktBase )
{
	int iSktDataLen = sktBase->getSktBufDataLen();
	if( iSktDataLen < NET_SERVICE_HDR_LEN )
	{
		// not even header has arrived so return
		return;
	}
	
	char *	pSktBuf = (char *)sktBase->getSktReadBuf();
	int urlLen = m_NetServiceUtils.getTotalLengthFromNetServiceUrl( pSktBuf, iSktDataLen );
	if( 0 >= urlLen )
	{
		sktBase->sktBufAmountRead( 0 );
		LogMsg( LOG_ERROR, "handleFirstNetServiceConnection: not valid" );
		VxReportHack( eHackerLevelMedium, eHackerReasonNetSrvUrlInvalid, sktBase, "handleFirstNetServiceConnection: invalid url" );
		sktBase->closeSkt( eSktCloseNetSrvUrlInvalid );
		return;
	}

	sktBase->sktBufAmountRead( 0 );
	if( urlLen > iSktDataLen )
	{
		// not all of data here yet
		return;
	}

	bool httpConnectionWasHandled = false;
	sktBase->setIsFirstRxPacket( false );

	NetServiceHdr netServiceHdr;
	EPluginType pluginType = m_NetServiceUtils.parseHttpNetServiceUrl( sktBase, netServiceHdr );


    if( ( netServiceHdr.m_NetCmdType == eNetCmdQueryHostOnlineIdReq ) &&
        ( ePluginTypeHostNetwork == pluginType || ePluginTypeHostGroup == pluginType || 
            ePluginTypeHostChatRoom == pluginType || ePluginTypeHostRandomConnect == pluginType || ePluginTypeHostConnectTest == pluginType) )
    {
        // only allowed if Hosting feature is enabled
        PluginBase* poPlugin = getPlugin( pluginType );
        if( poPlugin )
        {
			bool hasAnnonService{ false };
			std::string onlineId;
			if( m_Engine.getHasAnyAnnonymousHostService() )
			{
				// if we have any anonymous hosting we send back valid id but will still set error if plugin is disabled
				onlineId = m_Engine.getMyOnlineId().toHexString();
				hasAnnonService = true;
			}
			else
			{
				VxGUID nullGuid;
				onlineId = nullGuid.toHexString();
			}

            if( eAppStatePermissionErr != poPlugin->getPluginState() )
            {
                std::string onlineId = m_Engine.getMyOnlineId().toHexString();
				m_NetServiceUtils.buildAndSendCmd( sktBase, eNetCmdQueryHostOnlineIdReply, onlineId );
				// flush then close
				sktBase->closeSkt( eSktCloseNetSrvQueryIdSent, true );
            }
            else
            {
				if( !hasAnnonService )
				{
					m_Engine.hackerOffense( eHackerLevelSuspicious, eHackerReasonNetSrvQueryIdPermission, nullptr, sktBase->getRemoteIpBinary(), "Hacker http attack from ip %s query host ID not allowed", sktBase->getRemoteIp().c_str() );
				}
   
                m_NetServiceUtils.buildAndSendCmd( sktBase, eNetCmdQueryHostOnlineIdReply, onlineId, ( eFriendStateIgnore == poPlugin->getPluginPermission() ) ? eNetCmdErrorServiceDisabled : eNetCmdErrorPermissionLevel );
                sktBase->dumpSocketStats();
                // flush then close
                sktBase->closeSkt( eSktCloseNetSrvQueryIdPermission, true );
            }
        }
        else
        {
            m_Engine.hackerOffense( eHackerLevelMedium, eHackerReasonNetSrvUrlInvalid, nullptr, sktBase->getRemoteIpBinary(), "Hacker http attack from ip %s invalid plugin", sktBase->getRemoteIp().c_str() );
            sktBase->closeSkt( eSktCloseNetSrvPluginInvalid, false );
        }

        return;
    }

	if( ePluginTypeInvalid != pluginType )
	{
		VxNetIdent* netIdent = NULL;
		if( netServiceHdr.m_OnlineId == m_Engine.getMyPktAnnounce().getMyOnlineId() )
		{
			netIdent = &m_Engine.getMyPktAnnounce();
			LogMsg( LOG_INFO, "PluginMgr::handleFirstNetServiceConnection: parseOnlineId was myself" );
		}
		else
		{
			netIdent = m_Engine.getBigListMgr().findBigListInfo( netServiceHdr.m_OnlineId );
		}
	
		netServiceHdr.m_Ident = netIdent;
		PluginBase* poPlugin = getPlugin( pluginType );
		if( poPlugin )
		{
			int32_t rc = -1;
			if( ePluginTypeNetServices == poPlugin->getPluginType() || ePluginTypeHostConnectTest == poPlugin->getPluginType() )
			{
				rc = poPlugin->handlePtopConnection( sktBase, netServiceHdr );
			}

			if( 0 == rc )
			{
				// socket was handled
				httpConnectionWasHandled = true;
			}
			else
			{
				VxGUID signatureGuid;
				if( netIdent )
				{
					signatureGuid = netIdent->getMyOnlineId();
				}
				else if( iSktDataLen >= 32 )
				{
					signatureGuid.fromRawData( ( uint8_t* )pSktBuf );
				}

				m_Engine.hackerOffense( eHackerLevelSuspicious, eHackerReasonHttpAttack, sktBase->getRemoteIpBinary(), signatureGuid, "Hacker http attack from ip %s", sktBase->getRemoteIp().c_str() );
				sktBase->closeSkt( eSktCloseHttpHandleError );
				return;
			}
		}
		else
		{
			LogMsg( LOG_INFO, "PluginMgr::handleFirstNetServiceConnection; unknown plugin type" );
            m_Engine.hackerOffense( eHackerLevelMedium, eHackerReasonNetSrvPluginInvalid, nullptr, sktBase->getRemoteIpBinary(), "Hacker http attack (unknown plugin)from ip %s", sktBase->getRemoteIp().c_str() );
            sktBase->dumpSocketStats();
			sktBase->closeSkt( eSktCloseHttpPluginInvalid );
		}
	}

	if( false == httpConnectionWasHandled )
	{
        m_Engine.hackerOffense( eHackerLevelSevere, eHackerReasonHttpAttack, nullptr, sktBase->getRemoteIpBinary(), "Hacker http attack from ip %s", sktBase->getRemoteIp().c_str() );
        sktBase->dumpSocketStats();
		sktBase->closeSkt( eSktCloseHttpHandleError );
	}
}

//============================================================================
//! this is called for all valid packets that are not net service packets
void PluginMgr::handleNonSystemPackets( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	//LogMsg( LOG_INFO, "PluginMgr::handleNonSystemPackets" );
	uint8_t u8PluginNum = pktHdr->getPluginNum();
	if( isValidPluginNum( u8PluginNum ) )
	{
		PluginBase* plugin = getPlugin( (EPluginType)u8PluginNum );
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( pktHdr->getSrcOnlineId() );
		if( netIdent && plugin )
		{
			if( plugin->isAccessAllowed( netIdent ) )
			{
				plugin->handlePkt( sktBase, pktHdr, netIdent );
			}
			else if( IsAssetTransferPacketType( pktHdr->getPktType() ) )
			{
				// Asset transfer packets are additionally validated in AssetBaseXferMgr using
				// session IDs and transfer state. Do not drop continuation packets here.
				LogMsg( LOG_WARN, "PluginMgr::%s access override for asset packet type %d plugin %d from user %s", __func__,
					pktHdr->getPktType(), u8PluginNum, netIdent->getOnlineName() );
				plugin->handlePkt( sktBase, pktHdr, netIdent );
			}
			else if( !netIdent->isMyself() )
			{
				LogMsg( LOG_ERROR, "PluginMgr::%s access denied for plugin %s (%d) from user %s with packet %s", __func__, 
					DescribePluginType( (EPluginType)u8PluginNum ), u8PluginNum, netIdent->getOnlineName(), pktHdr->describePktHdr().c_str() );
				VxReportHack( eHackerLevelSuspicious, eHackerReasonAccessDenied, sktBase, netIdent->getOnlineName() );
			}
		}
		else // TODO BRJ handle case of valid netIdent not needed?
		{
			LogMsg( LOG_ERROR, "PluginMgr::%s unknown ident %s or plugin %d", __func__,
				pktHdr->getSrcOnlineId().toOnlineIdString().c_str(), u8PluginNum );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginMgr::%s invalid plugin num %d", __func__, u8PluginNum );
	}
}

//============================================================================
bool PluginMgr::isValidPluginNum( uint8_t u8PluginNum )
{
    return (ePluginTypeInvalid < u8PluginNum ) && (ePluginTypeMaxNetRange > u8PluginNum );
}

//============================================================================
//! get permission/access state for remote user
EPluginAccess PluginMgr::pluginApiGetPluginAccessState( EPluginType pluginType, VxNetIdent* netIdent )
{
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		EPluginAccess eAccess = m_PktAnn.getHisAccessPermissionFromMe( pluginType );
		if( ePluginAccessOk == eAccess )
		{
			eAccess = plugin->canAcceptNewSession( netIdent );
		}

		return eAccess;
	}

	return ePluginAccessDisabled;
}

//============================================================================
VxNetIdent* PluginMgr::pluginApiGetMyIdentity( void )
{
	return &m_PktAnn;
}

//============================================================================
VxNetIdent* PluginMgr::pluginApiFindUser( const char* pUserName )
{
	return m_BigListMgr.findBigListInfo( pUserName );
}

//============================================================================
void PluginMgr::fromGuiUserLoggedOn( void )
{
    vx_assert( m_PluginMgrInitialized );
	// set all plugin permissions
	std::vector<PluginBase* >::iterator iter;
	for( PluginBase* pluginBase : m_aoPlugins )
	{
        vx_assert( pluginBase );
        pluginBase->setPluginPermission( m_PktAnn.getPluginPermission( pluginBase->getPluginType() ) );
	}

	// now tell plugins we are logged on
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		PluginBase* pluginBase = ( *iter );
        vx_assert( pluginBase );
		pluginBase->fromGuiUserLoggedOn();
	}

	// tell all plugins to startup
	onAppStateChange( eAppStateStartup );
}

//============================================================================
void PluginMgr::onAppStateChange( EAppState eAppState )
{
	switch( eAppState )
	{
	case eAppStateStartup:
		onAppStartup();
		break;
	case eAppStateShutdown:
		onAppShutdown();
		break;
	case eAppStatePause:
		//fromGuiAppPause();
		break;
	case eAppStateResume:
		//fromGuiAppResume();
		break;
	default:
		break;
	}
}

//============================================================================
void PluginMgr::onAppStartup( void )
{
	std::vector<PluginBase* >::iterator iter;
	int pluginIdx = 0;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
        //LogMsg( LOG_INFO, "pluginMgr::onAppStartup idx %d\n", pluginIdx );
		PluginBase* pluginBase = (*iter);
		pluginBase->onAppStartup();
		pluginIdx++;
	}
}

//============================================================================
void PluginMgr::onAppShutdown( void )
{
	std::vector<PluginBase* >::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		(*iter)->onAppShutdown();
	}
}

//============================================================================
void PluginMgr::fromGuiListAction( EListAction listAction )
{
	std::vector<PluginBase* >::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		( *iter )->fromGuiListAction( listAction );
	}
}

//============================================================================
void PluginMgr::onMyOnlineUrlIsValid( bool iValid )
{
	for( auto& pluginBase : m_aoPlugins )
	{
		pluginBase->onMyOnlineUrlIsValid( iValid );
	}
}

//============================================================================
void PluginMgr::onOncePerSecond( void )
{
	//NOTE: TODO ?
	//std::vector<PluginBase* >::iterator iter;
	//for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	//{
	//	(*iter)->onOncePerSecond();
	//}
}

//============================================================================
void PluginMgr::onThreadOncePer15Minutes( void )
{
    std::vector<PluginBase* >::iterator iter;
    for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
    {
		if( !VxIsAppShuttingDown() )
		{
			( *iter )->onThreadOncePer15Minutes();
		}
		else
		{
			break;
		}
    }
}

//============================================================================
void PluginMgr::onAfterUserLogOnThreaded( void )
{
	std::vector<PluginBase* >::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		if( !VxIsAppShuttingDown() )
		{
			( *iter )->onAfterUserLogOnThreaded();
		}
		else
		{
			break;
		}
	}
}

//============================================================================
void PluginMgr::onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	for( auto& pluginBase : m_aoPlugins )
	{
		pluginBase->onContactWentOnline( netIdent, sktBase );
	}
}

//============================================================================
void PluginMgr::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	for( auto& pluginBase : m_aoPlugins )
	{
		pluginBase->onContactWentOffline( netIdent, sktBase );
	}
}

//============================================================================
void PluginMgr::callbackConnectionStatusChange( ConnectId& connectId, bool isOnline )
{
	if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "PluginMgr::%s online %d groupie %s ", __func__,
		isOnline, m_Engine.describeConnectId( connectId ).c_str() );
	for( auto& pluginBase : m_aoPlugins )
	{
		pluginBase->onContactOnlineStatusChange( connectId, isOnline );
	}
}

//============================================================================
void PluginMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	for( auto iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		(*iter)->onConnectionLost( sktBase );
	}
}

//============================================================================
void PluginMgr::fromGuiRelayPermissionCount( int userPermittedCount, int anonymousCount )
{
	for( auto iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		(*iter)->fromGuiRelayPermissionCount( userPermittedCount, anonymousCount );
	}
}

//============================================================================
bool PluginMgr::fromGuiSendAsset( AssetBaseInfo& assetInfo )
{
    if( ePluginTypeInvalid == assetInfo.getPluginType() )
	{
        LogMsg( LOG_ERROR, "PluginMgr::%s invalid plugin type %d", __func__, assetInfo.getPluginType() );
        vx_assert( false );
        return false;
	}

	bool sendResult{ false };
    PluginBase* pluginBase = findPlugin( assetInfo.getPluginType() );
    if( pluginBase )
    {
		if(LogEnabled(eLogAssets))LogModule( eLogAssets, LOG_VERBOSE,
			"PluginMgr::%s assetId %s assetType %s plugin %s host %s creator %s admin %s sendTo %s detail %s",
			__func__,
			assetInfo.getAssetUniqueId().toHexString().c_str(),
			DescribeAssetType( assetInfo.getAssetType() ),
			DescribePluginType( assetInfo.getPluginType() ),
			DescribeHostType( PluginTypeToHostType( assetInfo.getPluginType() ) ),
			m_Engine.getBigListMgr().getOnlineName( assetInfo.getCreatorId() ).c_str(),
			m_Engine.getBigListMgr().getOnlineName( assetInfo.getAdminId() ).c_str(),
			m_Engine.getBigListMgr().getOnlineName( assetInfo.getSendToId() ).c_str(),
			assetInfo.describe().c_str() );
        sendResult = pluginBase->fromGuiSendAsset( assetInfo );
    }

	return sendResult;
}

//============================================================================
PluginBase* PluginMgr::findPlugin( EPluginType pluginType )
{
	std::vector<PluginBase* >::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		if( pluginType == (*iter)->getPluginType() )
		{
			return (*iter);
		}
	}

	return nullptr;
}

//============================================================================
PluginBase* PluginMgr::findHostClientPlugin( EHostType hostType )
{
    return  hostClientToPlugin( hostType );
}

//============================================================================
PluginBase* PluginMgr::findHostServicePlugin( EHostType hostType )
{
    return hostServiceToPlugin( hostType );
}

//============================================================================
PluginBase* PluginMgr::hostClientToPlugin( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeChatRoom:
        return findPlugin( ePluginTypeClientChatRoom );
    case eHostTypeConnectTest:
        return findPlugin( ePluginTypeClientConnectTest );
    case eHostTypeGroup:
        return findPlugin( ePluginTypeClientGroup );
    case eHostTypeNetwork:
        return findPlugin( ePluginTypeClientNetwork );
    case eHostTypeRandomConnect:
        return findPlugin( ePluginTypeClientRandomConnect );
	case eHostTypePeerUser:
        return findPlugin( ePluginTypeMessenger );
    default:
        return nullptr;
    }
}

//============================================================================
PluginBase* PluginMgr::hostServiceToPlugin( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeChatRoom:
        return findPlugin( ePluginTypeHostChatRoom );
    case eHostTypeConnectTest:
        return findPlugin( ePluginTypeHostConnectTest );
    case eHostTypeGroup:
        return findPlugin( ePluginTypeHostGroup );
    case eHostTypeNetwork:
        return findPlugin( ePluginTypeHostNetwork );
    case eHostTypeRandomConnect:
        return findPlugin( ePluginTypeHostRandomConnect );
    default:
        return nullptr;
    }
}

//============================================================================
bool PluginMgr::fromGuiMultiSessionAction( EMSessionAction mSessionAction, VxGUID& onlineId, int pos0to100000, VxGUID lclSessionId )
{
	bool result = false;
	PluginBase* plugin = findPlugin( ePluginTypeMessenger );
	if( plugin )
	{
		BigListInfo * bigInfo = m_BigListMgr.findBigListInfo( onlineId );
		if( bigInfo )
		{
			result = plugin->fromGuiMultiSessionAction( onlineId, mSessionAction, pos0to100000, lclSessionId );
		}
	}

	return result;
}

//============================================================================
//! return true if access ok
bool PluginMgr::canAccessPlugin( EPluginType pluginType, VxNetIdent* netIdent )
{
	LogMsg( LOG_VERBOSE, "PluginMgr::%s", __func__ );
	EFriendState eHisFriendshipToMe = netIdent->getHisFriendshipToMe();
	EFriendState ePluginPermission = netIdent->getPluginPermission(pluginType);
	if( ( ePluginPermission != eFriendStateIgnore ) &&
		( ePluginPermission <= eHisFriendshipToMe ) )
	{
		return true;
	}

	return false;
}

//============================================================================
void PluginMgr::pluginApiPlayJpgVideo( EPluginType pluginType, VxNetIdent* netIdent, std::shared_ptr<CamJpgVideo>& jpgVideo )
{
	//LogMsg( LOG_INFO, "PluginMgr::pluginApiPlayJpgVideo\n" );
	IToGui::getIToGui().toGuiPlayJpgVideo( netIdent->getMyOnlineId(), jpgVideo );
}

//============================================================================
void PluginMgr::pluginApiWantMediaInput( EPluginType pluginType, EMediaInputType mediaType, EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput )
{
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		m_Engine.getMediaProcessor().wantMediaInput( m_Engine.getMyOnlineId(), mediaType, plugin, mediaModule, mediaSessionId, wantInput );
	}
}

//============================================================================
//! called to start service or session with remote friend
bool PluginMgr::fromGuiStartPluginSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId )
{
    bool result{false};
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		VxNetIdent* netIdent = m_BigListMgr.findNetIdent( onlineId );
		if( netIdent )
		{
            result = plugin->fromGuiStartPluginSession( onlineId, lclSessionId );
		}
		else
		{
			LogMsg( LOG_ERROR, "PluginMgr::fromGuiStartPluginSession: id not found NOT FOUND %s my id %s", 
				onlineId.describeVxGUID().c_str(), 
				m_PktAnn.getMyOnlineId().describeVxGUID().c_str() );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginMgr::fromGuiStartPluginSession: invalid plugin type %d", pluginType );
    }

    return result;
}

//============================================================================
//! called to stop service or session with remote friend
void PluginMgr::fromGuiStopPluginSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId )
{
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		plugin->fromGuiStopPluginSession( onlineId, lclSessionId );	
	}
	else
	{
		LogMsg( LOG_ERROR, "PluginMgr::fromGuiStopPluginSession: invalid plugin type %d", pluginType );
	}
}

//============================================================================
//! return true if is plugin session
bool PluginMgr::fromGuiIsPluginInSession( EPluginType pluginType, VxGUID& onlineId, VxGUID lclSessionId )
{
	bool inSession = false;
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		inSession = plugin->fromGuiIsPluginInSession( onlineId, lclSessionId );	
	}

	return inSession;
}

//============================================================================
bool PluginMgr::fromGuiTodGameActionSend( EPluginType pluginType, VxGUID& onlineId, ETodGameAction todGameAction )
{
	bool bResult = false;
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		bResult = plugin->fromGuiTodGameActionSend( onlineId, todGameAction );
	}

	return bResult;
}

//============================================================================
int PluginMgr::fromGuiDeleteFile( std::string& fileName, bool shredFile )
{
	std::vector<PluginBase* >::iterator iter;
	for( iter = m_aoPlugins.begin(); iter != m_aoPlugins.end(); ++iter )
	{
		(*iter)->fromGuiDeleteFile( fileName, shredFile );
	}

	return 0;
}

//============================================================================
EPluginAccess PluginMgr::canAcceptNewSession( EPluginType pluginType, VxNetIdent* netIdent )
{
	EPluginAccess canAcceptSession = ePluginAccessDisabled;
	PluginBase* plugin = getPlugin( pluginType );
	if( plugin )
	{
		canAcceptSession = plugin->canAcceptNewSession( netIdent );
	}

	return canAcceptSession;
}

//============================================================================
//! get identity from socket connection
VxNetIdent* PluginMgr::pluginApiOnlineIdToIdentity( VxGUID& oOnlineId )
{
	BigListInfo * poInfo = m_BigListMgr.findBigListInfo( oOnlineId );
	if( poInfo )
	{
		return poInfo;
	}
	LogMsg( LOG_ERROR, "PluginMgr::pluginApiSktToIdentity: NOT FOUND");
	return NULL;
}

//============================================================================
void PluginMgr::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	for( auto pluginBase : m_aoPlugins )
	{
		pluginBase->replaceConnection( netIdent, poOldSkt, poNewSkt );
	}
}

//============================================================================
void PluginMgr::leavePreviousHost( GroupieId& groupieId )
{
	PluginBaseHostClient* plugin = dynamic_cast< PluginBaseHostClient* >( findPlugin( HostTypeToClientPlugin( groupieId.getHostType() ) ) );
	if( plugin )
	{
		plugin->sendLeaveHost( groupieId );
	}

	m_Engine.getConnectIdListMgr().disconnectIfIsOnlyUser( groupieId );
}

//============================================================================
void PluginMgr::onNetworkConnectionReady( bool requiresRelay )
{
	for( auto pluginBase : m_aoPlugins )
	{
		pluginBase->onNetworkConnectionReady( requiresRelay );
	}
}

//============================================================================
void PluginMgr::fromGuiUpdatePluginPermission( EPluginType pluginType, EFriendState pluginPermission )
{
	PluginBase* pluginBase = findPlugin( pluginType );
	if( pluginBase )
	{
		pluginBase->fromGuiUpdatePluginPermission( pluginType, pluginPermission );
	}
}


//============================================================================
void PluginMgr::fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing )
{
	PluginBase* pluginBase = findPlugin( pluginType );
	if( pluginBase )
	{
		pluginBase->fromGuiAdminViewHost( pluginType, adminIsViewing );
	}
}
