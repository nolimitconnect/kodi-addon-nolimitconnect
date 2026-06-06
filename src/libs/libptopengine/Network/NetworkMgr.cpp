//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetworkMgr.h"
#include "NetworkDefs.h"

#include <P2PEngine/P2PEngine.h>
#include <P2PEngine/P2PConnectList.h>
#include <BigListLib/BigListMgr.h>
#include <BigListLib/BigListInfo.h>
#include <NetworkMonitor/NetworkMonitor.h>

#include <NetLib/VxSktBase.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktConnect.h>
#include <NetLib/VxSktCrypto.h>

#include <PktLib/VxCommon.h>
#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsRelay.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

namespace
{
	void NetworkPeerSktCallbackHandler( std::shared_ptr<VxSktBase>&  sktBase, void * pvUserCallbackData )
	{
        if( pvUserCallbackData )
		{
            NetworkMgr * netMgr = (NetworkMgr *)pvUserCallbackData;
            netMgr->handleTcpSktCallback( sktBase );
		}
	}

	void NetworkSktMgrStatusCallbackHandler( const char* sktAction, SOCKET sktHandle, void* pvUserCallbackData )
	{
		if( pvUserCallbackData )
		{
			NetworkMgr* netMgr = (NetworkMgr*)pvUserCallbackData;
			netMgr->handleSktMgrStatusCallback( sktAction, sktHandle );
		}
	}
}

//============================================================================
NetworkMgr::NetworkMgr( P2PEngine&		engine, 
						VxPeerMgr&		peerMgr,
						BigListMgr&		bigListMgr
						)
: m_Engine( engine )
, m_PktAnn( engine.getMyPktAnnounce() )
, m_PeerMgr( peerMgr )
, m_BigListMgr( bigListMgr )
{
	m_PeerMgr.setReceiveCallback( NetworkPeerSktCallbackHandler, this );
	m_PeerMgr.setSktMgrStatusCallback( NetworkSktMgrStatusCallbackHandler, this );
}

//============================================================================
void NetworkMgr::updateFromEngineSettings( EngineSettings& engineSettings )
{
    // TODO: should probably use a mutex here
	uint16_t u16TcpPort = engineSettings.getTcpIpPort();
	m_PktAnn.setOnlinePort( u16TcpPort );

    m_Engine.getNetStatusAccum().setIpPort( u16TcpPort );
	m_Engine.getPeerMgr().setUpnpEnable( engineSettings.getUseUpnpPortForward() );

	bool ipv6 = m_Engine.getEngineSettings().getUseIpv6();
	EFirewallTestType firewallTestType = engineSettings.getFirewallTestSetting();
	m_Engine.getNetStatusAccum().setFirewallTestType( firewallTestType );
	if( eFirewallTestAssumeNoFirewall == firewallTestType )
	{
		std::string externIp;
		engineSettings.getUserSpecifiedExternIpAddr( externIp, ipv6 );
		if( !externIp.empty() )
		{
			m_Engine.getNetStatusAccum().setDirectConnectTested( true, false, externIp );
		}
	}

	std::string networkKey;
	engineSettings.getNetworkKey( networkKey );
    if( !networkKey.empty() && ( networkKey != getNetworkKey() ) )
    {
        setNetworkKey( networkKey.c_str() );
        // will restore only if network key has changed
        m_Engine.getBigListMgr().dbRestoreAll();
    }	
}

//============================================================================
void NetworkMgr::handleTcpSktCallback( std::shared_ptr<VxSktBase>& sktBase )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	sktBase->setLastActiveTimeMs( GetGmtTimeMs() );

	switch( sktBase->getCallbackReason() )
	{
	case eSktCallbackReasonConnectError:
        LogModule( eLogSkt, LOG_ERROR, "NetworkMgr:TCP skt %d skt id %d connect error %s thread 0x%x",
                    sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->describeSktError( sktBase->getLastSktError() ), VxGetCurrentThreadId() );
		break;

	case eSktCallbackReasonConnected:
        LogModule( eLogSkt, LOG_INFO, "NetworkMgr:TCP skt %d skt id %d %s port %d to local port %d thread 0x%x",
                    sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->describeSktType().c_str(), sktBase->m_RmtIp.getPort(), sktBase->m_LclIp.getPort(), VxGetCurrentThreadId() );
		break;

	case eSktCallbackReasonData:
		m_Engine.handleTcpData( sktBase );
		break;

	case eSktCallbackReasonClosed:
        LogModule( eLogSkt, LOG_INFO, "NetworkMgr:TCP skt handle %d num %d id %s closed %s thread 0x%x",
                    sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->getSocketIdText().c_str(), sktBase->describeSktError( sktBase->getLastSktError() ), VxGetCurrentThreadId() );
		m_Engine.onConnectionLost( sktBase );
		break;

	case eSktCallbackReasonError:
 		LogModule( eLogSkt, LOG_ERROR, "NetworkMgr:TCP skt %d skt id %d error %s thread 0x%x",
                    sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->describeSktError( sktBase->getLastSktError() ), VxGetCurrentThreadId() );
		break;

	case eSktCallbackReasonClosing:
        LogModule( eLogSkt, LOG_INFO, "NetworkMgr:TCP eSktCallbackReasonClosing skt handle %d num %d id %s thread 0x%x", 
				   sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->getSocketIdText().c_str(), VxGetCurrentThreadId() );
		m_Engine.onConnectionClosing( sktBase ); 
		break;

	case eSktCallbackReasonConnecting:
        LogModule( eLogSkt, LOG_INFO, "NetworkMgr:TCP eSktCallbackReasonConnecting skt %d skt id %d thread 0x%x", sktBase->getSktHandle(), sktBase->getSktNumber(), VxGetCurrentThreadId() );
		break;

	default:
		LogMsg( LOG_ERROR, "NetworkMgrTCP: UNKNOWN CallbackReason %d skt %d skt id %d error %s thread 0x%x", 
                sktBase->getCallbackReason(), sktBase->getSktHandle(), sktBase->getSktNumber(), sktBase->describeSktError( sktBase->getLastSktError() ), VxGetCurrentThreadId() );
		break;
	}
}

//============================================================================
void NetworkMgr::handleSktMgrStatusCallback( const char* sktAction, SOCKET sktHandle )
{
	if( sktAction )
	{
		std::string sktMgrParam( sktAction );
		if( !sktMgrParam.empty() )
		{
			m_Engine.sktMgrStatusCallback( sktMgrParam, sktHandle );
		}
	}
}
