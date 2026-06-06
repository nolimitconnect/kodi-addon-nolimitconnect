#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <memory>

class GroupieInfo;
class P2PEngine;
class PktAnnounce;
class PluginBaseRelay;
class VxSktBase;
class VxPktHdr;
class BigListInfo;
class VxGUID;

class RelayMgr
{
public:
	RelayMgr( P2PEngine& engine );
	virtual ~RelayMgr() = default;

	bool						handleRelayPkt( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr );

	bool						onRelayPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, BigListInfo* srcBigInfo, BigListInfo* destBigInfo );

	bool						isJoinedToRelayHost( VxGUID& onlineId );

	bool						sendRelayError( VxPktHdr* pktHdr, VxGUID& srcOnlineId, VxGUID& destOnlineId, std::shared_ptr<VxSktBase>& sktBase, ERelayErr relayErr );

	//=== vars ====//
	P2PEngine&					m_Engine;
};
