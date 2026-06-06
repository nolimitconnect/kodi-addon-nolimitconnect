//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RelayMgr.h"

#include <BigListLib/BigListInfo.h>
#include <HostServerJoinMgr/HostServerJoinMgr.h>
#include <Membership/MemberActiveMgr.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsRelay.h>

//============================================================================
RelayMgr::RelayMgr( P2PEngine& engine )
	: m_Engine( engine )
{
	LogMsg( LOG_VERBOSE, "RelayMgr::RelayMgr" );
}

//============================================================================
bool RelayMgr::handleRelayPkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	VxGUID destOnlineId = pktHdr->getDestOnlineId();

	// verify src is someone who has connected
	BigListInfo* srcBigInfo = m_Engine.getBigListMgr().findBigListInfo( srcOnlineId );
	if( !srcBigInfo )
	{
		// this is someone we have never encountered
		VxReportHack( eHackerLevelSevere, eHackerReasonInvalidPkt, sktBase, "attempted relay pkt %s null src bigListInfo", pktHdr->describePktHdr().c_str(), srcOnlineId.toOnlineIdString().c_str() );
		sktBase->closeSkt( eSktCloseHackLevelSevere );
		return true;
	}

	// verify dest is someone who has connected
	BigListInfo* destBigInfo = m_Engine.getBigListMgr().findBigListInfo( destOnlineId );
	if( !destBigInfo )
	{
		// this is not someone recently connected but if was not friend or admin they would not have been restored on startup
		LogMsg( LOG_WARN, "RelayMgr::handleRelayPkt null destBigInfo pkt %s from %s %s to %s", pktHdr->describePktHdr().c_str(),
				srcBigInfo->getOnlineName(), srcOnlineId.toOnlineIdString().c_str(), destOnlineId.toOnlineIdString().c_str() );
		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrUserNotOnline );
		return true;
	}

	if( srcBigInfo->isIgnored() )
	{
		VxReportHack( eHackerLevelSuspicious, eHackerReasonAccessDenied, sktBase, "ignored user %s %s attempted relay pkt %s", 
					  srcBigInfo->getOnlineName(), srcOnlineId.toOnlineIdString().c_str(), pktHdr->describePktHdr().c_str() );

		if( !m_Engine.getConnectIdListMgr().isConnectionInUse( sktBase->getSocketId() ) )
		{
			sktBase->closeSkt( eSktCloseBlockedUser );
		}

		return true;
	}

	if( !isJoinedToRelayHost( srcOnlineId ) )
	{
		LogMsg( LOG_WARN, "RelayMgr::handleRelayPkt  pkt %s from %s %s to %s %s src user is not joined to host", pktHdr->describePktHdr().c_str(),
				srcBigInfo->getOnlineName(), srcOnlineId.toOnlineIdString().c_str(), destBigInfo->getOnlineName(), destOnlineId.toOnlineIdString().c_str() );

		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrSrcNotJoined );
		return true;
	}

	if( !isJoinedToRelayHost( destOnlineId ) )
	{
		LogMsg( LOG_WARN, "RelayMgr::handleRelayPkt  pkt %s from %s %s to %s %s destination user is not joined to host", pktHdr->describePktHdr().c_str(),
				srcBigInfo->getOnlineName(), srcOnlineId.toOnlineIdString().c_str(), destBigInfo->getOnlineName(), destOnlineId.toOnlineIdString().c_str() );

		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrDestNotJoined );
		return true;
	}

    std::shared_ptr<VxSktBase> sktBaseRelay = m_Engine.getConnectIdListMgr().findDirectConnection( destOnlineId );
	if( !sktBaseRelay )
	{
		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrUserNotOnline );
		return true;
	}
	else if( sktBaseRelay->getIsPeerPktAnnSet() && sktBaseRelay->getPeerOnlineId() != destOnlineId )
	{
		LogModule( eLogRelay, LOG_ERROR, "handleRelayPkt wrong socket found" );
		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrUserNotOnline );
		return true;
	}

	if( pktHdr->getPktType() == PKT_TYPE_ANNOUNCE )
	{
		PktAnnounce* pktAnn = (PktAnnounce*)pktHdr;
		if( !onRelayPktAnnounce( sktBase, pktAnn, srcBigInfo, destBigInfo ) )
		{
			sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrInvalidPktAnn );
			return true;
		}
	}

	if( 0 != sktBaseRelay->txPacket( destOnlineId, pktHdr ) )
	{
        LogModule( eLogRelay, LOG_VERBOSE, "handleRelayPkt FAILED sent relay pkt %s srcId %s %s destId %s %s", pktHdr->describePktHdr().c_str(),
			srcOnlineId.toOnlineIdString().c_str(), sktBase->getPeerOnlineName().c_str(), destOnlineId.toOnlineIdString().c_str(), sktBaseRelay->getPeerOnlineName().c_str() );

		sendRelayError( pktHdr, srcOnlineId, destOnlineId, sktBase, eRelayErrUserNotOnline );
		return true;
	}

    if( LogEnabled( eLogRelay ) )LogModule( eLogRelay, LOG_VERBOSE, "handleRelayPkt sent relay pkt %s src %s %s dest %s %s", pktHdr->describePktHdr().c_str(),
		srcOnlineId.toOnlineIdString().c_str(), sktBase->getPeerOnlineName().c_str(), destOnlineId.toOnlineIdString().c_str(), sktBaseRelay->getPeerOnlineName().c_str() );
	return true;
}

//============================================================================
bool RelayMgr::isJoinedToRelayHost( VxGUID& onlineId )
{
	return m_Engine.getMemberActiveMgr().isUserJoinedToRelayHost( onlineId, m_Engine.getMyOnlineId() );
}

//============================================================================
bool RelayMgr::onRelayPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, BigListInfo* srcBigInfo, BigListInfo* destBigInfo )
{
	if( LogEnabled( eLogRelay ) )LogModule( eLogRelay, LOG_VERBOSE, "RelayMgr::onRelayPktAnnounce from %s %s to %s %s",
			srcBigInfo->getOnlineName(), srcBigInfo->getMyOnlineId().toOnlineIdString().c_str(),
			destBigInfo->getOnlineName(), destBigInfo->getMyOnlineId().toOnlineIdString().c_str() );
	// TODO extra validation
	return true;
}

//============================================================================
bool RelayMgr::sendRelayError( VxPktHdr* pktHdr, VxGUID& srcOnlineId, VxGUID& destOnlineId, std::shared_ptr<VxSktBase>& sktBase, ERelayErr relayErr )
{
	PktRelayUserDisconnect pktReply;
	pktReply.setSrcOnlineId( m_Engine.getMyOnlineId() );

	pktReply.setRelayedPktType( pktHdr->getPktType() );
	pktReply.setDestUserOnlineId( destOnlineId );
	pktReply.setHostOnlineId( m_Engine.getMyOnlineId() );
	pktReply.setRelayError( relayErr );

	return 0 == sktBase->txPacket( srcOnlineId, &pktReply );
}
