//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RandConnectMgr.h"

#include "RandConnectCallback.h"
#include <Membership/MemberActiveMgr.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>

#include <algorithm>

namespace
{
    bool IsOfferAction( enum ERandAction randAction )
    {
        return randAction >= eRandActionOfferRequest && randAction < eMaxRandAction;
    }
}

//============================================================================
void RandConnectMgr::onEngineStartup( void )
{
    GetPtoPEngine().getMemberActiveMgr().wantMemberActiveCallbacks( this, true );
}

//============================================================================
void RandConnectMgr::onEngineShutdown( void )
{
    GetPtoPEngine().getMemberActiveMgr().wantMemberActiveCallbacks( this, false );
}

//============================================================================
void RandConnectMgr::updateRandConnectStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction )
{
    VxGUID sessionId = VxGUID::nullVxGUID();
    updateRandConnectStatus( groupieId, toUserOnlineId, randAction, sessionId, 0, eOfferTypeUnknown );
}

//============================================================================
void RandConnectMgr::updateRandConnectStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction, VxGUID& sessionId, uint64_t timeRequestedMs, EOfferType offerType )
{
    if( isSelectionAction( randAction ) )
    {
        GroupieId toUserGroupie( groupieId );
        toUserGroupie.setUserOnlineId( toUserOnlineId );
        updateRandConnectStatus( groupieId, randAction );
        updateRandConnectStatus( toUserGroupie, randAction );
        return;
    }

    if( IsOfferAction( randAction ) )
    {
        updateRandConnectOfferStatus( groupieId, toUserOnlineId, randAction, sessionId, timeRequestedMs, offerType );
        announceRandConnectOffer( groupieId, toUserOnlineId, sessionId, randAction, timeRequestedMs, offerType );
    }
}

//============================================================================
bool RandConnectMgr::isSelectionAction( enum ERandAction randAction )
{
    return eRandActionNone == randAction || eRandActionSelectUser == randAction || eRandActionDeselectUser == randAction;
}

//============================================================================
bool RandConnectMgr::isTerminalOfferAction( enum ERandAction randAction )
{
    switch( randAction )
    {
    case eRandActionOfferAccept:
    case eRandActionOfferReject:
    case eRandActionOfferCancel:
    case eRandActionOfferNoResponse:
    case eRandActionOfferMissed:
        return true;

    default:
        return false;
    }
}

//============================================================================
void RandConnectMgr::updateRandConnectOfferStatus( GroupieId& groupieId, VxGUID& toUserOnlineId, enum ERandAction randAction, VxGUID& sessionId, uint64_t timeRequestedMs, EOfferType offerType )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "RandConnectMgr::updateRandConnectOfferStatus invalid groupieId" );
        return;
    }

    lockOfferList();

    auto iter = std::find_if( m_OfferList.begin(), m_OfferList.end(),
                              [&]( const RandConnectOfferEntry& offerEntry )
                              {
                                  return offerEntry.m_SessionId == sessionId;
                              } );

    if( eRandActionOfferRequest == randAction )
    {
        if( iter == m_OfferList.end() )
        {
            RandConnectOfferEntry offerEntry;
            offerEntry.m_GroupieId = groupieId;
            offerEntry.m_ToUserOnlineId = toUserOnlineId;
            offerEntry.m_SessionId = sessionId;
            offerEntry.m_RandAction = randAction;
            offerEntry.m_TimeRequestedMs = timeRequestedMs;
            offerEntry.m_OfferType = offerType;
            m_OfferList.emplace_back( offerEntry );
        }
        else
        {
            iter->m_GroupieId = groupieId;
            iter->m_ToUserOnlineId = toUserOnlineId;
            iter->m_RandAction = randAction;
            iter->m_TimeRequestedMs = timeRequestedMs;
            iter->m_OfferType = offerType;
        }
    }
    else if( iter != m_OfferList.end() )
    {
        if( isTerminalOfferAction( randAction ) )
        {
            m_OfferList.erase( iter );
        }
        else
        {
            iter->m_RandAction = randAction;
            if( timeRequestedMs )
            {
                iter->m_TimeRequestedMs = timeRequestedMs;
            }

            if( offerType != eOfferTypeUnknown )
            {
                iter->m_OfferType = offerType;
            }
        }
    }

    unlockOfferList();
}

//============================================================================
void RandConnectMgr::updateRandConnectStatus( GroupieId& groupieId, enum ERandAction randAction )
{
    if( !groupieId.isValid() )
    {
        LogMsg( LOG_ERROR, "RandConnectMgr::updateSktIdent invalid skt id" );
        return;
    }

    bool wasUpdated = false;
    lockMemberList();

    bool found{ false };
    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); ++iter )
    {
        if( iter->first == groupieId )
        {
            found = true;
            if( randAction == eRandActionNone )
            {
                m_MemberList.erase( iter );
                wasUpdated = true;
            }
            else if( iter->second != randAction )
            {
                iter->second = randAction;
                wasUpdated = true;
            }

            break;
        }
    }

    if( randAction != eRandActionNone && !found )
    {
        m_MemberList.emplace_back( std::make_pair( groupieId, randAction ) );
        wasUpdated = true;
    }

    unlockMemberList();

    if( wasUpdated )
    {
        announceRandConnect( groupieId, randAction );
    }
}

//============================================================================
void RandConnectMgr::announceRandConnect( GroupieId& groupieId, enum ERandAction randAction )
{
    lockClientList();

    for( auto& client : m_MemberClients )
    {
        client->callbackRandConnect( groupieId, randAction );
    }

    unlockClientList();
}

//============================================================================
void RandConnectMgr::announceRandConnectOffer( GroupieId& groupieId, VxGUID& toUserOnlineId, VxGUID& sessionId, enum ERandAction randAction, uint64_t timeRequestedMs, EOfferType offerType )
{
    lockClientList();

    for( auto& client : m_MemberClients )
    {
        client->callbackRandConnectOffer( groupieId, toUserOnlineId, sessionId, randAction, timeRequestedMs, offerType );
    }

    unlockClientList();
}

//============================================================================
void RandConnectMgr::wantRandConnectCallbacks( RandConnectCallback* client, bool enable )
{
    lockClientList();

    bool found{ false };
    for( auto iter = m_MemberClients.begin(); iter != m_MemberClients.end(); ++iter )
    {
        if( *iter == client )
        {
            found = true;
            if( !enable )
            {
                m_MemberClients.erase( iter );
            }
            else
            {
                LogMsg( LOG_ERROR, "RandConnectMgr::wantRandConnectCallbacks ignored because already in list" );
            }

            break;
        }
    }
    
    if( !found && enable )
    {
        m_MemberClients.emplace_back( client );
    }

    unlockClientList();
}

//============================================================================
void RandConnectMgr::callbackConnectionStatusChange( ConnectId& connectId, bool isOnline )
{
    if( isOnline )
    {
        return;
    }
    
    VxGUID onlineId = connectId.getUserOnlineId();
    std::vector<std::pair<GroupieId,ERandAction>> offlineMemberList;
    std::vector<RandConnectOfferEntry> offlineOffers;
    lockMemberList();

    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); )
    {
        if( iter->first.getHostOnlineId() == onlineId  || iter->first.getUserOnlineId() == onlineId )
        {
            offlineMemberList.emplace_back( *iter );
            iter = m_MemberList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockMemberList();

    lockOfferList();
    for( auto iter = m_OfferList.begin(); iter != m_OfferList.end(); )
    {
        if( iter->m_GroupieId.getHostOnlineId() == onlineId
            || iter->m_GroupieId.getUserOnlineId() == onlineId
            || iter->m_ToUserOnlineId == onlineId )
        {
            offlineOffers.emplace_back( *iter );
            iter = m_OfferList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }
    unlockOfferList();

    for( auto& randPair : offlineMemberList )
    {
        announceRandConnect( randPair.first, eRandActionNone );
    }

    for( auto& offerEntry : offlineOffers )
    {
        announceRandConnectOffer( offerEntry.m_GroupieId,
                                  offerEntry.m_ToUserOnlineId,
                                  offerEntry.m_SessionId,
                                  eRandActionOfferCancel,
                                  offerEntry.m_TimeRequestedMs,
                                  offerEntry.m_OfferType );
    }
}

//============================================================================
void RandConnectMgr::callbackMemberActive( GroupieId& groupieId, bool isActive )
{
    if( groupieId.getHostType() != eHostTypeRandomConnect )
    {
        return;
    }

    ERandAction randAction = isActive ? eRandActionDeselectUser : eRandActionNone;
    lockMemberList();
    auto iter = std::find_if( m_MemberList.begin(), m_MemberList.end(),
                              [&]( const std::pair<GroupieId, enum ERandAction>& randPair ) { return randPair.first == groupieId; } );
    bool foundMember = iter != m_MemberList.end();
    if( !foundMember )
    {
        m_MemberList.emplace_back( std::make_pair(groupieId, randAction ) );
    }

    unlockMemberList();
    if( !foundMember )
    {
        announceRandConnect( groupieId, randAction );
    }
}
