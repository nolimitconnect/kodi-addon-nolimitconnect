//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HandshakeList.h"
#include "HandshakeInfo.h"

#include <NetLib/VxSktBase.h>
#include <CoreLib/VxGuidPairTimeList.h>
#include <CoreLib/VxGUIDList.h>

namespace
{
    const int HANDSHAKE_TIMEOUT_MS = ( 5 * 60 * 1000 );
}

//============================================================================
void HandshakeList::getAndRemoveHandshakeInfo( const VxGUID& socketId, VxGUID onlineId, std::vector<HandshakeInfo>& shakeList, std::vector<HandshakeInfo>& timedOutList )
{
    VxGuidPairTimeList removeList;
    uint64_t timeNow = GetTimeStampMs();
    for( auto iter = m_ShakeList.begin(); iter != m_ShakeList.end(); ++iter )
    {
        HandshakeInfo& shakeInfo = iter->second;
        if( timeNow - shakeInfo.getTimeStamp() > HANDSHAKE_TIMEOUT_MS )
        {
            timedOutList.push_back( shakeInfo );
        }
        else
        {
            shakeList.push_back( shakeInfo );
        }

        removeList.addGuid( socketId, shakeInfo.getSessionId() );
    }

    auto& sessionList = removeList.getGuidList();
    for( auto& sesId : sessionList )
    {
        removeHandshakeInfo( sesId.first, sesId.second.first );
    }
}

//============================================================================
void HandshakeList::removeHandshakeInfo( const VxGUID& socketId, const VxGUID& sessionId )
{
    auto iter = m_ShakeList.find( std::make_pair( socketId, sessionId ) );
    if( iter != m_ShakeList.end() )
    {
        m_ShakeList.erase( iter );
    }
}

//============================================================================
void HandshakeList::removeHandshakeSession( const VxGUID& sessionId )
{
    for( auto iter = m_ShakeList.begin(); iter != m_ShakeList.end(); )
    {
        if( iter->first.second == sessionId )
        {
            iter = m_ShakeList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }
}

//============================================================================
void HandshakeList::addHandshake( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sessionId, VxGUID onlineId, IConnectRequestCallback* callback, 
                                  EConnectReason connectReason, EHostType hostType )
{
    uint64_t timeNow = GetTimeStampMs();
    HandshakeInfo shakeInfo( sktBase, sessionId, onlineId, callback, connectReason, timeNow, hostType );
    std::pair<VxGUID, VxGUID> socketIdSessionIdPair( std::make_pair( sktBase->getSocketId(), sessionId ) );
    m_ShakeList[ socketIdSessionIdPair ] = shakeInfo;
}

//============================================================================
void HandshakeList::removeHandshake( std::shared_ptr<VxSktBase>& sktBase )
{
    for ( auto iter = m_ShakeList.begin(); iter != m_ShakeList.end(); ++iter )
    {
        HandshakeInfo& shakeInfo = iter->second;
        if ( shakeInfo.getSktBase() == sktBase )
        {
            m_ShakeList.erase( iter );
            break;
        }
    }
}

//============================================================================
void HandshakeList::onSktDisconnected( const VxGUID& socketId )
{
    for( auto iter = m_ShakeList.begin(); iter != m_ShakeList.end(); ++iter )
    {
        if( iter->first.first == socketId )
        {
            iter->second.onSktDisconnected();
        }    
    }
}

//============================================================================
void HandshakeList::getAndRemoveHandshakeInfo( std::shared_ptr<VxSktBase>& sktBase, std::vector<HandshakeInfo>& disconnectedList )
{
    std::vector<std::pair<VxGUID,VxGUID>> removeList;
    for( auto iter = m_ShakeList.begin(); iter != m_ShakeList.end(); ++iter )
    {
        HandshakeInfo& shakeInfo = iter->second;
        if( shakeInfo.getSktBase() == sktBase )
        {
            disconnectedList.push_back( shakeInfo );
            removeList.push_back( std::make_pair( sktBase->getSocketId(), shakeInfo.getSessionId() ) );
        }
    }

    for( auto sessionPair : removeList )
    {
        removeHandshakeInfo( sessionPair.first, sessionPair.second );
    }
}
