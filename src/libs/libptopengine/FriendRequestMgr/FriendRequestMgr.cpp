//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FriendRequestMgr.h"
#include "FriendRequestCallbackInterface.h"
#include "FriendRequestInfo.h"

#include <P2PEngine/P2PEngine.h>
#include <BigListLib/BigListInfo.h>
#include <Plugins/PluginMgr.h>
#include <Plugins/PluginBase.h>
#include <Plugins/PluginBaseNetworkService.h>

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsFriendRequest.h>

//============================================================================
int32_t FriendRequestMgr::friendRequestMgrStartup( void)
{
    m_FriendRequestList.clear();
    return 0;
}

//============================================================================
int32_t FriendRequestMgr::friendRequestMgrShutdown( void )
{
    return 0;
}

//============================================================================
void FriendRequestMgr::removeFriendRequest( VxGUID& friendRequestId )
{
    VxGUID onlineId;
    bool wasRemoved{ false };
    lockList();
    for( auto iter = m_FriendRequestList.begin(); iter != m_FriendRequestList.end(); )
    {
        if( (*iter)->getRequestId() == friendRequestId )
        {
            onlineId = (*iter)->getUserOnlineId();
            m_FriendRequestList.erase( iter );
            wasRemoved = true;
            break;
        }
        else
        {
            ++iter;
        }
    }

    unlockList();
    if( wasRemoved )
    {
        announceFriendRequestInfoRemoved( onlineId, friendRequestId );
    }
}

//============================================================================
void FriendRequestMgr::addFriendRequestMgrClient( FriendRequestCallbackInterface* client, bool enable )
{
    lockClientList();
    for( auto iter = m_FriendRequestClients.begin(); iter != m_FriendRequestClients.end(); ++iter )
    {
        if( *iter == client )
        {
            m_FriendRequestClients.erase( iter );
            break;
        }
    }

    if( enable )
    {
        m_FriendRequestClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void FriendRequestMgr::announceFriendRequestInfoUpdated( std::shared_ptr<FriendRequestInfo>& friendRequest )
{
    if( friendRequest.get() )
    {
        lockClientList();
        for( auto& client : m_FriendRequestClients )
        {
            client->callbackFriendRequestUpdated( friendRequest );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "HostServerJoinMgr::%s invalid param", __func__ );
    }
}

//============================================================================
void FriendRequestMgr::announceFriendRequestInfoRemoved( VxGUID& friendOnlineId, VxGUID& requestId )
{
    // removeFromDatabase( hostOnlineId, hostType, false );
    lockClientList();
    for( auto& client : m_FriendRequestClients )
    {
        client->callbackFriendRequestRemoved( friendOnlineId, requestId );
    }

    unlockClientList();
}

//============================================================================
bool FriendRequestMgr::fromGuiQueryFriendRequest( std::vector<std::shared_ptr<FriendRequestInfo>>& friendRequestList, VxGUID& onlineIdIfNullThenAll )
{
    lockList();
    for( auto& friendRequest : m_FriendRequestList )
    {
        if( !onlineIdIfNullThenAll.isValid() || onlineIdIfNullThenAll == friendRequest->getUserOnlineId() )
        {
            friendRequestList.emplace_back( friendRequest );
        }
    }
    
    unlockList();

	return !friendRequestList.empty();
}

//============================================================================
bool FriendRequestMgr::fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestText, EFriendState myFriendshipToHim )
{
    return false;
}

//============================================================================
void FriendRequestMgr::rxedFriendRequest( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent& netIdentIn, VxGUID& requestId, std::string& requestMsg )
{
    BigListMgr& bigListMgr = GetPtoPEngine().getBigListMgr();
    if( bigListMgr.isUserIgnored( netIdentIn.getMyOnlineId() ) )
    {
        VxReportHack( eHackerLevelMedium, eHackerReasonFriendRequestFromIgnoredUser, sktBase, "rxed ignored friend request" );
        return;
    }

    VxNetIdent* netIdent{ nullptr };
    if( !netIdentIn.isMyself() )
    {
        bool newContact = GetPtoPEngine().getBigListMgr().updateTempIdent( netIdentIn );
        netIdent = GetPtoPEngine().getBigListMgr().findNetIdent( netIdentIn.getMyOnlineId() );
        if( !netIdent )
        {
            LogMsg( LOG_ERROR, "%s failed find net ident", __func__ );
            return;
        }
    }
    else
    {
        netIdent = GetPtoPEngine().getMyPktAnnounce().getVxNetIdent();
    }

    std::shared_ptr<VxNetIdent> netIdentShared( new VxNetIdent( *netIdent ) );
    std::shared_ptr<FriendRequestInfo> friendRequest( new FriendRequestInfo( netIdentIn.getMyOnlineId(), requestId, requestMsg, eFriendRequestRxed, netIdentShared, GetGmtTimeMs() ) );
    lockList();
    m_FriendRequestList.emplace_back( friendRequest );
    unlockList();
    announceFriendRequestInfoUpdated( friendRequest );
}

//============================================================================
void FriendRequestMgr::friendRequestAcked( VxGUID requestId, bool wasAccepted )
{
    VxGUID onlineId;
    bool foundRequest{ false };
    lockList();
    for( auto iter = m_FriendRequestList.begin(); iter != m_FriendRequestList.end(); ++iter )
    {
        if( (*iter)->getRequestId() == requestId )
        {
            foundRequest = true;
            onlineId = (*iter)->getUserOnlineId();
            m_FriendRequestList.erase( iter );
            break;
        }
    }
   
    unlockList();
    if( foundRequest )
    {
        announceFriendRequestInfoRemoved( onlineId, requestId );
    }
}
