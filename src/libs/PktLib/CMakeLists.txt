
cmake_minimum_required (VERSION 3.22.1)

project(pktlib, LANGUAGES CXX)

set(pktlib_SOURCES)

list(APPEND pktlib_SOURCES 
    HostUserSessionId.cpp
    MatchParams.cpp
    PktAdminAvail.cpp
	PktAnnList.cpp
    PktAnnounce.cpp
    PktChatReq.cpp
    PktDebugHelpers.cpp
	PktsOfferXfer.cpp
    PktPluginHandlerBase.cpp
    PktSysHandlerBase.cpp
    PktTcpPunch.cpp
    PktVoiceReply.cpp
    PktVoiceReq.cpp
    PktsAssetXfer.cpp
    PktsBaseXfer.cpp
	PktsBlobXfer.cpp
    PktsFileInfo.cpp
    PktsFileList.cpp
    PktsFileShare.cpp
	PktsFriendOffer.cpp
    PktsFriendRequest.cpp
    PktsGroupie.cpp
    PktsHostInfo.cpp
    PktsHostInvite.cpp
	PktsHostJoin.cpp
    PktsHostSearch.cpp
    PktsHostUser.cpp
    PktsImAlive.cpp
    PktsMembership.cpp
    PktsMultiSession.cpp
    PktsPing.cpp
    PktsPluginOffer.cpp
    PktsPushToTalk.cpp
    PktsQueryHostUrl.cpp
    PktsRandConnect.cpp
    PktsRelay.cpp
	PktsScan.cpp
	PktsSearch.cpp
    PktsSession.cpp
    PktsStreamCtrl.cpp
    PktsTestConnection.cpp
    PktsThumbXfer.cpp
    PktsTodGame.cpp
    PktsVideoFeed.cpp
    PktsWebServer.cpp
	PluginIdList.cpp
	SearchParams.cpp
    VxCommon.cpp
    VxConnectId.cpp
    VxConnectInfo.cpp
    VxFriendMatch.cpp
    VxOnlineStatusFlags.cpp
    VxPktHdr.cpp
    VxRelayFlags.cpp
    VxSearchFlags.cpp
)

add_library(pktlib STATIC ${pktlib_SOURCES})

target_include_directories(pktlib
  PRIVATE
    ../libptopengine
)


