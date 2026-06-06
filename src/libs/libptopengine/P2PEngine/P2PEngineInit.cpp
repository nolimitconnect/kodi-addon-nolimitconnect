//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include <FileMgr/FileMgr.h>
#include <FriendRequestMgr/FriendRequestMgr.h>
#include <Membership/MemberActiveMgr.h>
#include <OfferBase/OfferMgr.h>
#include <PushToTalk/PushToTalkMgr.h>
#include <RandConnect/RandConnectMgr.h>
#include <SendQueue/SendQueueMgr.h>

#include <NetLib/VxPeerMgr.h>

//============================================================================
VxPeerMgr& GetVxPeerMgr( void )
{
    static VxPeerMgr g_VxPeerMgr;
    return  g_VxPeerMgr;
}

//============================================================================
P2PEngine& GetPtoPEngine()
{
    static MemberActiveMgr memberActiveMgr;
    static OfferMgr offerMgr;
    static PushToTalkMgr pushToTalkMgr;
    static RandConnectMgr randConnectMgr;
    static SendQueueMgr sendQueueMgr;
    static FriendRequestMgr friendRequstMgr;
    static FileMgr fileMgr;
    static P2PEngine g_P2PEngine( GetVxPeerMgr(), memberActiveMgr, offerMgr, pushToTalkMgr, randConnectMgr, sendQueueMgr, friendRequstMgr, fileMgr );
    return g_P2PEngine;
}
