//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPeerMgr.h"
#include "VxSktConnect.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGUID.h>
#include <PktLib/VxPktHdr.h>

#ifdef TARGET_OS_LINUX
    // turn broke pipe exception on disconnected sockets into error instead
    #include <signal.h>
#endif

namespace
{
	void VxPeerMgrRxCallbackHandler( std::shared_ptr<VxSktBase>&  sktBase, void * pvUserCallbackData )
	{
		VxPeerMgr * peerMgr = ( VxPeerMgr * )pvUserCallbackData;
		if( peerMgr )
		{
			peerMgr->handleSktCallback( sktBase );
		}
	}
}

//============================================================================
static void sigpipe_handler(int unused)
{
    LogMsg( LOG_WARN, "SIGPIPE Error.. probably broke socket connection");
}

//============================================================================
VxPeerMgr::VxPeerMgr()
{
#ifdef TARGET_OS_LINUX
    // turn broke pipe exception on disconnected sockets into error instead
    // signal( SIGPIPE, SIG_IGN );

    // commented out because not allowed in FLATPAKBUILD
    //sigaction(SIGPIPE, &((struct sigaction){sigpipe_handler}), NULL);
#endif

	setReceiveCallback( VxPeerMgrRxCallbackHandler, this );
}

//============================================================================
void VxPeerMgr::sktMgrStartup( bool ipv6 )
{
	m_ClientMgr.sktMgrStartup( ipv6 );
	VxServerMgr::sktMgrStartup( ipv6 );
}

//============================================================================
void VxPeerMgr::sktMgrShutdown( void )
{
	m_ClientMgr.sktMgrShutdown();
	VxServerMgr::sktMgrShutdown();
}

//============================================================================
void VxPeerMgr::setReceiveCallback( VX_SKT_CALLBACK pfnReceive, void* pvUserData )
{
	VxServerMgr::setReceiveCallback( pfnReceive, pvUserData );
	m_ClientMgr.setReceiveCallback( pfnReceive, pvUserData );
}

//============================================================================
void VxPeerMgr::setSktMgrStatusCallback( VX_SKT_MGR_STATUS_CALLBACK pfnSktMgrStatus, void* pvUserData )
{
	VxServerMgr::setSktMgrStatusCallback( pfnSktMgrStatus, pvUserData );
	m_ClientMgr.setSktMgrStatusCallback( pfnSktMgrStatus, pvUserData );
}

//============================================================================
int VxPeerMgr::getActiveSktCnt( void )
{
	return VxServerMgr::getActiveSktCnt() + m_ClientMgr.getActiveSktCnt();
}

//============================================================================
int VxPeerMgr::getToDeleteSktCnt( void )
{
	return VxServerMgr::getToDeleteSktCnt() + m_ClientMgr.getToDeleteSktCnt();
}

//============================================================================
//! make a new socket... give derived classes a chance to override
std::shared_ptr<VxSktBase> VxPeerMgr::makeNewSkt( void )
{ 
	std::shared_ptr<VxSktBase> sharedSkt( new VxSktConnect() );
	sharedSkt->setThisSkt( sharedSkt ); // so skt can do callbacks without look up in manager
	return sharedSkt;
}

//============================================================================
// find a socket.. assumes list has been locked
std::shared_ptr<VxSktBase> VxPeerMgr::findSktBase( const VxGUID& connectId, bool acceptSktsOnly )
{
	if( !connectId.isValid() )
	{
        LogMsg( LOG_ERROR, "VxPeerMgr::findSktBase invalid connectId" );
        std::shared_ptr<VxSktBase> nullSktBase;
        return nullSktBase;
	}

	std::shared_ptr<VxSktBase> sktBase = VxSktBaseMgr::findSktBase( connectId, acceptSktsOnly );
	if( !sktBase )
	{
		sktBase = m_ClientMgr.findSktBase( connectId, acceptSktsOnly );
	}

	return sktBase;
}

//============================================================================
//! Connect to ip or url and return socket.. if cannot connect return NULL
std::shared_ptr<VxSktBase> VxPeerMgr::connectTo(	const char*		pIpOrUrl,				// remote ip or url 
													uint16_t		u16Port,				// port to connect to
													int				iTimeoutMilliSeconds )	// milli seconds before connect attempt times out
{
	if( NULL ==  m_pfnUserReceive )
	{
		LogMsg( LOG_INFO, "VxPeerMgr::VxConnectTo: you must call setReceiveCallback first" );
		vx_assert( m_pfnUserReceive );
	}
		
	std::shared_ptr<VxSktBase> sktBase	= makeNewSkt();
	sktBase->m_SktMgr		= this;
	sktBase->setReceiveCallback( m_pfnOurReceive, this );
	sktBase->setTransmitCallback( m_pfnOurTransmit, this );
	int32_t rc = sktBase->connectTo(	m_LclIp,
									pIpOrUrl, 
									u16Port, 
									iTimeoutMilliSeconds );
	if( rc )
	{
		sktBase.reset();
	}
	else
	{
		addSkt( sktBase );
	}

	return sktBase;
}

//============================================================================
std::shared_ptr<VxSktBase> VxPeerMgr::createConnectionUsingSocket( SOCKET skt, const char* rmtIp, uint16_t port )
{
	if( NULL ==  m_pfnUserReceive )
	{
		LogMsg( LOG_ERROR, "VxPeerMgr::createConnectionUsingSocket: you must call setReceiveCallback first" );
		vx_assert( m_pfnUserReceive );
	}

	std::shared_ptr<VxSktBase> sktBase	= makeNewSkt();
	sktBase->m_SktMgr		= this;
	//VxVerifyCodePtr( m_pfnOurReceive );
	sktBase->setReceiveCallback( m_pfnOurReceive, this );
	sktBase->setTransmitCallback( m_pfnOurTransmit, this );
	sktBase->createConnectionUsingSocket( skt, rmtIp, port );
	addSkt( sktBase );
	LogMsg( LOG_INFO, "VxPeerMgr::createConnectionUsingSocket: done skt id %d rmt ip %s port %d", sktBase->getSktNumber(), rmtIp, port  );
	return sktBase;
}

//============================================================================
void VxPeerMgr::handleSktCallback( std::shared_ptr<VxSktBase>& sktBase )
{
}

//============================================================================
bool VxPeerMgr::txPacket(	std::shared_ptr<VxSktBase>&		sktBase,
							const VxGUID&					destOnlineId,
							VxPktHdr*						pktHdr )
{
	pktHdr->setDestOnlineId( destOnlineId );
	return txPacketWithDestId( sktBase, pktHdr );
}

//============================================================================
bool VxPeerMgr::txPacketWithDestId(	std::shared_ptr<VxSktBase>&		sktBase,
									VxPktHdr*						pktHdr )
{
    if( !sktBase || false == isSktActive( sktBase ) )
	{

		if( false == m_ClientMgr.isSktActive( sktBase ) )
		{
            LogMsg( LOG_ERROR, "ERROR VxPeerMgr::txPacketWithDestId: skt no longer active" );
			if( sktBase && sktBase->isConnected() )
			{
				sktBase->setIsConnected( false );
				sktBase->closeSkt( eSktCloseTxFailed );
			}
		}

        return false;
	}

    if( sktBase->isConnected() )
    {
        int32_t rc = sktBase->txPacketWithDestId( pktHdr );
        if( 0 != rc )
        {
            LogMsg( LOG_VERBOSE, "VxPeerMgr::txPacketWithDestId: skt %d returned error %d %s", sktBase->getSktNumber(), rc, sktBase->describeSktError( rc ) );
			if( sktBase->isFatalSocketError( rc ) )
			{
				sktBase->setIsConnected( false );
				sktBase->closeSkt( eSktCloseTxFailed );
			}
        }

        return  ( 0 == rc );
    }
    else
    {
        LogMsg( LOG_ERROR, "ERROR VxPeerMgr::txPacketWithDestId: skt no longer connected" );
    }

    return false;
}

//============================================================================
void VxPeerMgr::dumpSocketStats( const char*reason, bool fullDump )
{
    std::string reasonMsg = reason ? reason : "";
    VxSktBaseMgr::dumpSocketStats( std::string( reasonMsg + " server: " ).c_str(), fullDump );
    m_ClientMgr.dumpSocketStats( std::string( reasonMsg + " client: " ).c_str(), fullDump );
}

//============================================================================
bool VxPeerMgr::closeConnection( VxGUID& connectId, ESktCloseReason closeReason )
{
	bool wasClosed = m_ClientMgr.closeConnection( connectId, closeReason );
	if( !wasClosed )
	{
		wasClosed = VxSktBaseMgr::closeConnection( connectId, closeReason );
	}

	return wasClosed;
}

//============================================================================
void VxPeerMgr::onOncePer30Seconds( VxGUID& myOnlineId )
{
	VxSktBaseMgr::onOncePer30Seconds( myOnlineId );
	m_ClientMgr.onOncePer30Seconds( myOnlineId );
}

//============================================================================
void VxPeerMgr::getSktStatRecords( std::vector<VxSktStatRecord>& sktStatList )
{
	sktStatList.clear();
	VxSktBaseMgr::getSktStatRecords( sktStatList );
	m_ClientMgr.getSktStatRecords( sktStatList );
}
