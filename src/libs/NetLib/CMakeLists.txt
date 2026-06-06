
cmake_minimum_required (VERSION 3.22.1)

project(netlib, LANGUAGES CXX)

set(netlib_SOURCES)

list(APPEND netlib_SOURCES 
    NetHostSetting.cpp
    NetSettings.cpp
    VxBaseSktInfo.cpp
    VxClientMgr.cpp
    VxFileXferInfo.cpp
    VxGetRandomPort.cpp
	VxHackerMgr.cpp
	VxHackerRecord.cpp
    VxHttpParse.cpp
    VxHttpRequest.cpp
    VxHttpValuePair.cpp
    VxListenLogic.cpp
    VxParseUrlComponents.cpp
    VxPeerMgr.cpp
    VxPortForward.cpp
    VxProxy.cpp
    VxServerMgr.cpp
    VxSktAccept.cpp
    VxSktBase.cpp
    VxSktBaseMgr.cpp
    VxSktBuf.cpp
    VxSktConnect.cpp
    VxSktConnectSimple.cpp
    VxSktCrypto.cpp
    VxSktStatRecord.cpp
    VxSktThrottle.cpp
    VxSktUdp.cpp
    VxSktUdpBroadcast.cpp
    VxSktWaitingList.cpp
)

add_library(netlib STATIC ${netlib_SOURCES})

target_include_directories(netlib
  PRIVATE
    ../libptopengine
)


