//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HandshakeInfo.h"

#include <BigListLib/BigListInfo.h>
#include <NetServices/NetServicesMgr.h>
#include <NetServices/NetServiceHdr.h>
#include <P2PEngine/P2PEngine.h>

#include <NetLib/VxSktBase.h>

#include <CoreLib/VxParse.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>

//============================================================================
HandshakeInfo::HandshakeInfo(std::shared_ptr<VxSktBase>&sktBase, VxGUID& sessionId, VxGUID onlineId, IConnectRequestCallback* callback, 
                              EConnectReason connectReason, uint64_t timeStamp, EHostType hostType )
    : m_SktBase(sktBase)
    , m_SocketId( sktBase->getSocketId() )
    , m_SessionId(sessionId)
    , m_OnlineId(onlineId)
    , m_Callback(callback)
    , m_ConnectReason(connectReason)
    , m_TimeStamp(timeStamp)
    , m_HostType(hostType)
{
}

//============================================================================
HandshakeInfo::HandshakeInfo( const HandshakeInfo& rhs )
    : m_SktBase( rhs.m_SktBase )
    , m_SocketId( rhs.m_SocketId )
    , m_SessionId( rhs.m_SessionId )
    , m_OnlineId( rhs.m_OnlineId )
    , m_Callback( rhs.m_Callback )
    , m_ConnectReason( rhs.m_ConnectReason )
    , m_TimeStamp( rhs.m_TimeStamp )
    , m_HanshakeComplete( rhs.m_HanshakeComplete )
    , m_HostType( rhs.m_HostType )
{
}

//============================================================================
HandshakeInfo& HandshakeInfo::operator=( const HandshakeInfo& rhs )
{
    if( this != &rhs )
    {
        m_SktBase = rhs.m_SktBase;
        m_SocketId = rhs.m_SocketId;
        m_SessionId = rhs.m_SessionId;
        m_OnlineId = rhs.m_OnlineId;
        m_Callback = rhs.m_Callback;
        m_ConnectReason = rhs.m_ConnectReason;
        m_TimeStamp = rhs.m_TimeStamp;
        m_HanshakeComplete = rhs.m_HanshakeComplete;
        m_HostType = rhs.m_HostType;
    }

    return *this;
}

//============================================================================
bool HandshakeInfo::operator==( const HandshakeInfo& rhs )
{
    return  m_SessionId == rhs.m_SessionId;
}

//============================================================================
void HandshakeInfo::onContactConnected( void )
{
    if( m_Callback )
    {
        m_Callback->onContactConnected( m_SessionId, m_SktBase, m_OnlineId, m_ConnectReason );
    }
}

//============================================================================
void HandshakeInfo::onHandshakeTimeout( void )
{
    if( m_Callback )
    {
        m_Callback->onHandshakeTimeout( m_SessionId, m_SktBase, m_OnlineId, m_ConnectReason );
    }
}

//============================================================================
void HandshakeInfo::onContactSessionDone()
{
    if( m_Callback )
    {
        m_Callback->onContactSessionDone( m_SessionId, m_SktBase, m_OnlineId, m_ConnectReason );
    }
}

//============================================================================
void HandshakeInfo::onSktDisconnected( void )
{
    if( m_Callback )
    {
        m_Callback->onContactDisconnected( m_SessionId, m_SktBase, m_OnlineId, m_ConnectReason );
    }
}
