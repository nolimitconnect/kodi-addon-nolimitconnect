//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ConnectedInfo.h"
#include "HandshakeInfo.h"

#include <BigListLib/BigListInfo.h>
#include <NetServices/NetServicesMgr.h>
#include <NetServices/NetServiceHdr.h>
#include <P2PEngine/P2PEngine.h>

#include <NetLib/VxSktBase.h>

#include <CoreLib/VxParse.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>

#include <algorithm>

//============================================================================
ConnectedInfo::ConnectedInfo( P2PEngine& engine, const VxGUID& socketId, BigListInfo* bigListInfo )
    : m_Engine( engine )
    , m_SocketId( socketId )
    , m_BigListInfo( bigListInfo )
{
    if( bigListInfo )
    {
        m_PeerOnlineId = bigListInfo->getMyOnlineId();
    }
}

//============================================================================
ConnectedInfo::ConnectedInfo( const ConnectedInfo& rhs )
    : m_Engine( rhs.m_Engine )
    , m_SocketId( rhs.m_SocketId )
    , m_BigListInfo( rhs.m_BigListInfo )
    , m_PeerOnlineId( rhs.m_PeerOnlineId )
{
    m_CallbackList = rhs.m_CallbackList;
}

//============================================================================
ConnectedInfo& ConnectedInfo::operator=( const ConnectedInfo& rhs )
{
    if( this != &rhs )
    {
        m_SocketId = rhs.m_SocketId;
        m_BigListInfo = rhs.m_BigListInfo;
        m_PeerOnlineId = rhs.m_PeerOnlineId;
        m_CallbackList = rhs.m_CallbackList;
    }

    return *this;
}

//============================================================================
bool ConnectedInfo::operator==( const ConnectedInfo& rhs )
{
    return  m_PeerOnlineId == rhs.m_PeerOnlineId;
}

//============================================================================
std::shared_ptr<VxSktBase> ConnectedInfo::getSktBase( void )
{
    if( m_CallbackList.size() )
    {
        return m_CallbackList[0].getSktBase();
    }

    return std::shared_ptr<VxSktBase>(nullptr);
}

//============================================================================
void ConnectedInfo::addConnectReason( HandshakeInfo& shakeInfo )
{
    m_CallbackList.emplace_back( shakeInfo );
}

//============================================================================
void ConnectedInfo::onHandshakeComplete( HandshakeInfo& shakeInfo )
{
    addConnectReason( shakeInfo );
}

//============================================================================
bool ConnectedInfo::removeConnectReason( VxGUID& sessionId, IConnectRequestCallback* callback, EConnectReason connectReason, std::shared_ptr<VxSktBase>& retSktBaseIfDisconnected )
{
    bool sktDisconnected{ false };
    std::vector<HandshakeInfo>  completedList;
    for( auto iter = m_CallbackList.begin(); iter != m_CallbackList.end();  )
    {
        if( iter->getSessionId() == sessionId && iter->getCallback() == callback && iter->getConnectReason() == connectReason )
        {
            iter->onContactSessionDone();
            completedList.emplace_back( *iter );
            iter = m_CallbackList.erase(iter);
            break;
        }
        else
        {
            ++iter;
        }
    }

    if( m_CallbackList.empty() && !completedList.empty())
    {
        HandshakeInfo& shakeInfo = completedList[0];
        if( shakeInfo.getSktBase() && shakeInfo.getSktBase()->isConnected() )
        {
            // return skt so caller can close it if disconnected to avoid mutex deadlock
            sktDisconnected = true;
            retSktBaseIfDisconnected = shakeInfo.getSktBase();
        }
    }

    return sktDisconnected;
}

//============================================================================
void ConnectedInfo::onSktDisconnected( const VxGUID& socketId )
{
    // this connected info is about to be erased in ConnectedListAll::onSktDisconnected
}

//============================================================================
void ConnectedInfo::aboutToDelete( void )
{
    // TODO callback those using the connection to say it is now unavailable
}
