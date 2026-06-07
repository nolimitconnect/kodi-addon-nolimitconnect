cmake_minimum_required (VERSION 3.22.1)

project(ptopengine, LANGUAGES CXX)

set(ptopengine_HEADERS)
list(APPEND ptopengine_HEADERS
    AppMonitor/AppMonitor.h
    SendQueue/SendQueInfo.h
    SendQueue/SendQueueCallback.h
    SendQueue/SendQueueDb.h
    SendQueue/SendQueueMgr.h
    MediaProcessor/CamJpgVideo.h
    MediaProcessor/MediaProcessor.h
    Plugins/PluginFriendRequest.h
)

set(ptopengine_SOURCES)

list(APPEND ptopengine_SOURCES 
    AppMonitor/AppMonitor.cpp
    AssetBase/AssetBaseInfo.cpp
    AssetBase/AssetBaseInfoDb.cpp
    AssetBase/AssetBaseMgr.cpp
    AssetBase/AssetBaseRxSession.cpp
    AssetBase/AssetBaseTxSession.cpp
    AssetBase/AssetBaseXferDb.cpp
    AssetBase/AssetBaseXferMgr.cpp
    AssetBase/AssetBaseXferPktCreate.cpp
    AssetBase/AssetBaseXferSession.cpp
    AssetBase/AssetPlaySession.h
    AssetBase/AssetPlaySession.cpp
    AssetMgr/AssetInfo.cpp
    AssetMgr/AssetInfoDb.cpp
    AssetMgr/AssetMgr.cpp
    AssetMgr/AssetRxSession.cpp
    AssetMgr/AssetTxSession.cpp
	AssetMgr/AssetXferPktCreate.cpp
    AssetMgr/AssetXferDb.cpp
    AssetMgr/AssetXferMgr.cpp
    AssetMgr/AssetXferSession.cpp
	BaseInfo/BaseHostInfo.cpp
	BaseInfo/BaseJoinInfo.cpp
	BaseInfo/BaseInfo.cpp
	BaseInfo/BaseSessionInfo.cpp
    BigListLib/BigList.cpp
    BigListLib/BigListDb.cpp
    BigListLib/BigListInfo.cpp
    BigListLib/BigListMgr.cpp
    BlobXferMgr/BlobInfo.cpp
    BlobXferMgr/BlobInfoDb.cpp
    BlobXferMgr/BlobMgr.cpp
    BlobXferMgr/BlobRxSession.cpp
    BlobXferMgr/BlobTxSession.cpp
    BlobXferMgr/BlobXferDb.cpp
    BlobXferMgr/BlobXferMgr.cpp
    BlobXferMgr/BlobXferSession.cpp
    ConnectIdListMgr/ConnectIdListMgr.cpp
	Connections/ConnectedInfo.cpp
	Connections/ConnectedListAll.cpp
	Connections/ConnectedListBase.cpp
	Connections/ConnectionMgr.cpp
	Connections/ConnectReqInfo.cpp
	Connections/HandshakeInfo.cpp
	Connections/HandshakeList.cpp
    ConnectMgr/ConnectInfo.cpp
    ConnectMgr/ConnectInfoDb.cpp
    ConnectMgr/ConnectMgr.cpp

    FileMgr/FileMgr.cpp
    FromGuiMgr/FromGuiAction.cpp
    FromGuiMgr/FromGuiMgr.cpp
    FriendRequestMgr/FriendRequestInfo.cpp
    FriendRequestMgr/FriendRequestMgr.cpp

    GroupieListMgr/GroupieInfo.cpp
    GroupieListMgr/GroupieListDb.cpp
    GroupieListMgr/GroupieListMgr.cpp
    HostServerJoinMgr/HostJoinInfo.cpp
    HostServerJoinMgr/HostJoinedLastDb.cpp
    HostServerJoinMgr/HostJoinInfoDb.cpp
    HostServerJoinMgr/HostServerJoinMgr.cpp
    HostListMgr/HostedInfo.cpp
    HostListMgr/HostedListDb.cpp
    HostListMgr/HostedListMgr.cpp
    HostListMgr/HostUrlInfo.cpp
    HostListMgr/HostUrlListDb.cpp
    HostListMgr/HostUrlListMgr.cpp
    IdentListMgrs/FriendListMgr.cpp
    IdentListMgrs/IdentListMgrBase.cpp
    IdentListMgrs/IgnoredHostInfo.cpp
    IdentListMgrs/IgnoredHostsDb.cpp
    IdentListMgrs/IgnoreListMgr.cpp
    MediaProcessor/AudioUtil.cpp
    MediaProcessor/MediaProcessor.cpp
    Membership/MemberActiveMgr.cpp
    Membership/MemberConfirmMgr.cpp
    NetServices/NetActionBase.cpp
    NetServices/NetActionIdle.cpp
    NetServices/NetActionIsMyPortOpen.cpp
    NetServices/NetActionRenewPortForward.h
    NetServices/NetActionRenewPortForward.cpp
    NetServices/NetActionResolveConnectTestUrl.cpp
    NetServices/NetActionResolveDefaultUserHosts.cpp
    NetServices/NetActionResolveNetworkHostUrl.cpp
    NetServices/NetActionWaitForInternet.cpp
    NetServices/NetServiceUtils.cpp
    NetServices/NetServicesMgr.cpp

    Network/ConnectRequest.cpp
    Network/Firewall.cpp
    Network/NetworkMgr.cpp
    Network/StayConnected.h
    Network/StayConnected.cpp

    NetworkMonitor/NetStatusAccum.cpp
    NetworkMonitor/NetworkMonitor.cpp
	NetworkTest/IsPortOpenTest.cpp
    NetworkTest/NetworkTestBase.cpp
    NetworkTest/PingResponseServer.cpp
    NetworkTest/RunUrlAction.cpp

    OfferBase/OfferBaseInfo.cpp
    OfferBase/OfferBaseInfoDb.cpp
    OfferBase/OfferBaseMgr.cpp
    OfferBase/OfferBaseRxSession.cpp
    OfferBase/OfferBaseTxSession.cpp
    OfferBase/OfferBaseXferDb.cpp
    OfferBase/OfferBaseXferMgr.cpp
    OfferBase/OfferBaseXferSession.cpp
    OfferBase/OfferMgr.cpp
    P2PEngine/ContactList.cpp
    P2PEngine/EngineParams.cpp
    P2PEngine/EngineSettings.cpp
    P2PEngine/FileShareSettings.cpp
    P2PEngine/P2PConnectList.cpp

    P2PEngine/P2PEngine.cpp
    P2PEngine/P2PEngineAssetMgrCallbacks.cpp
    P2PEngine/P2PEngineConnect.cpp
    P2PEngine/P2PEngineConnections.cpp
    P2PEngine/P2PEngineContact.cpp
    P2PEngine/P2PEngineFromGui.cpp
    P2PEngine/P2PEngineHostList.cpp
    P2PEngine/P2PEngineIgnoredAndHackers.cpp
    P2PEngine/P2PEngineInit.cpp
    P2PEngine/P2PEnginePktAnnChanged.cpp
    P2PEngine/P2PEnginePktAnnHandlers.cpp
    P2PEngine/P2PEnginePktHandlers.cpp
    P2PEngine/P2PEngineRelay.cpp
    P2PEngine/P2PEngineScan.cpp
    P2PEngine/P2PEngineSearch.cpp
    P2PEngine/P2PEngineStartup.cpp
    P2PEngine/P2PEngineSktCallback.cpp
    P2PEngine/P2PEngineTimerThread.cpp

	P2PEngine/VxSktLoopback.cpp
	PluginSettings/PluginSetting.cpp
	PluginSettings/PluginSettingDb.cpp
	PluginSettings/PluginSettingMgr.cpp
    Plugins/AudioJitterBuffer.cpp
    Plugins/FileInfo.cpp
    Plugins/FileInfoBaseMgr.cpp
    Plugins/FileInfoClientFilesMgr.cpp
    Plugins/FileInfoDb.cpp
    Plugins/FileInfoSharedFilesMgr.cpp
    Plugins/FileInfoXferMgr.cpp
    Plugins/FileInfoPersonFileXferMgr.cpp
    Plugins/FileRxSession.cpp
    Plugins/FileShareXferSession.cpp
    Plugins/FileToXfer.cpp
    Plugins/FileTxSession.cpp
    Plugins/FileXferPluginSession.cpp
	Plugins/HostBaseMgr.cpp
	Plugins/HostClientMgr.cpp
	Plugins/HostClientSearchMgr.cpp
	Plugins/HostSearchEntry.cpp
	Plugins/HostServerMgr.cpp
	Plugins/HostServerSearchMgr.cpp
    Plugins/P2PSession.cpp
    Plugins/PluginAboutMePageClient.cpp
    Plugins/PluginAboutMePageServer.cpp
    Plugins/PluginBase.cpp
    Plugins/PluginBaseFiles.cpp
    Plugins/PluginBaseFilesClient.cpp
    Plugins/PluginBaseFileShare.cpp
    Plugins/PluginBaseFilesServer.cpp
	Plugins/PluginBaseHostClient.cpp
    Plugins/PluginBaseHostService.cpp
	Plugins/PluginBaseMultimedia.cpp
    Plugins/PluginBaseNetworkService.cpp
	Plugins/PluginBasePktHandlers.cpp
    Plugins/PluginBaseService.cpp
    Plugins/PluginCamClient.cpp
    Plugins/PluginCamServer.cpp
    Plugins/PluginChatRoomClient.cpp
	Plugins/PluginChatRoomHost.cpp
    Plugins/PluginPeerUserClient.cpp
    Plugins/PluginConnectionTestClient.cpp
	Plugins/PluginConnectionTestHost.cpp
    Plugins/PluginFileShareClient.cpp
    Plugins/PluginFileShareServer.cpp
    Plugins/PluginFriendRequest.cpp
    Plugins/PluginInvalid.cpp
    Plugins/PluginGroupClient.cpp
	Plugins/PluginGroupHost.cpp
    Plugins/PluginLibraryServer.cpp
    Plugins/PluginMgr.cpp
    Plugins/PluginMgrSktApi.cpp
    Plugins/PluginMessenger.cpp
    Plugins/PluginNetServices.cpp
	Plugins/PluginNetworkHost.cpp

    Plugins/PluginPeerUserHost.cpp
    Plugins/PluginPersonFileXfer.cpp
    Plugins/PluginPushToTalk.cpp
	Plugins/PluginRandomConnectClient.cpp
	Plugins/PluginRandomConnectHost.cpp
    Plugins/PluginStoryboardServer.cpp
    Plugins/PluginSessionBase.cpp
    Plugins/PluginSessionMgr.cpp
    Plugins/PluginStoryboardClient.cpp
    Plugins/PluginTruthOrDare.cpp
    Plugins/PluginVideoPhone.cpp
    Plugins/PluginVoicePhone.cpp
    Plugins/PushToTalkFeedMgr.cpp
    Plugins/RxSession.cpp
    Plugins/SessionMgrBase.cpp
    Plugins/TodGameSession.cpp
    Plugins/TxSession.cpp
    Plugins/VideoFeedMgr.cpp
    Plugins/VoiceFeedMgr.cpp
    PushToTalk/PushToTalkMgr.cpp
    RandConnect/RandConnectMgr.cpp
    Relay/RelayMgr.cpp
    Search/RcScan.cpp
    Search/RcScanAction.cpp
    Search/RcScanMatchedConnection.cpp
    SendQueue/SendQueInfo.cpp
    SendQueue/SendQueueDb.cpp
    SendQueue/SendQueueMgr.cpp
    SktLists/SktListMgr.cpp
    SktLists/SktListMgrBase.cpp
	ThumbMgr/ThumbInfo.cpp
    ThumbMgr/ThumbInfoDb.cpp
    ThumbMgr/ThumbMgr.cpp
    ThumbMgr/ThumbRxSession.cpp
	ThumbMgr/ThumbTxSession.cpp
	ThumbMgr/ThumbXferDb.cpp
    ThumbMgr/ThumbXferInterface.cpp
    ThumbMgr/ThumbXferMgr.cpp
	ThumbMgr/ThumbXferPktCreate.cpp
	ThumbMgr/ThumbXferSession.cpp
    UrlMgr/UrlMgr.cpp
	User/User.cpp
	User/UserList.cpp
    UserJoinMgr/LastJoined.cpp
    UserJoinMgr/UserJoinedLastDb.cpp
	UserJoinMgr/UserJoinInfo.cpp
	UserJoinMgr/UserJoinInfoDb.cpp
	UserJoinMgr/UserJoinMgr.cpp
    WebPageMgr/WebPageMgr.cpp 
)

if(DEFINED TARGET_OS_ANDROID)
find_package(Qt6 REQUIRED COMPONENTS Core)
endif()

add_library(ptopengine STATIC ${ptopengine_SOURCES} ${ptopengine_HEADERS})

if(DEFINED TARGET_OS_ANDROID)
    message(STATUS "ptopengine prefix directory ${CMAKE_PREFIX_PATH}")
    message(STATUS "ptopengine find root directory ${CMAKE_FIND_ROOT_PATH}")
    message(STATUS "ptopengine toolchain file ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "ptopengine platform ${DANDROID_PLATFORM}")
    message(STATUS "ptopengine abi ${ANDROID_ABI}")
    message(STATUS "ptopengine ndk ${ANDROID_NDK}")
    message(STATUS "ptopengine sdk root ${ANDROID_SDK_ROOT}")
    message(STATUS "ptopengine cpu ${NLC_CPU}")

    target_link_libraries(ptopengine Qt::Core Qt::CorePrivate)
endif()

target_compile_definitions(ptopengine
    PRIVATE
    ENABLE_NLC_PLAYER
)

if(MSVC)
    # Fix UNICODE definitions that may have been inherited from linked libraries (like Qt) or global settings
    string(REPLACE "/DUNICODE" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/D_UNICODE" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # This forcefully UNDEFINES the macros at the compiler level
    # and ensures they are overridden even if they appear earlier in the command line.
    add_compile_options(/UUNICODE /U_UNICODE)

    add_compile_options(
        "$<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:C>>:/W0>"  # This turns off all warnings for windows builds
        "$<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:C>>:/Zc:strictStrings->"  # Disables specific check for string literal const-qualifiers
        "$<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:C>>:/permissive>" # Allows non-const char* conversion (Legacy behavior).. Usually set by /std:c++17 or later
    )
endif()


target_include_directories(ptopengine
  PRIVATE
        ./
    ../
    ../libplayer-nlc/xbmc
    ../ffmpeg
)


