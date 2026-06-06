//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "UserList.h"

#include <P2PEngine/P2PEngine.h>

#include <PktLib/VxCommon.h>

//============================================================================
User::User()
    : m_Engine( GetPtoPEngine() )
{
}

//============================================================================
User::User( P2PEngine& engine )
    : m_Engine( engine )
{
}

//============================================================================
User::User( P2PEngine& engine, VxNetIdent* netIdent )
    : m_Engine( engine )
    , m_NetIdent( netIdent )
{
    if( m_NetIdent )
    {
        m_MyOnlineId = m_NetIdent->getMyOnlineId();
    }
}

//============================================================================
User::User( P2PEngine& engine, VxNetIdent* netIdent, BaseSessionInfo& sessionInfo )
    : m_Engine( engine )
    , m_NetIdent( netIdent )
{
    if( m_NetIdent )
    {
        m_MyOnlineId = m_NetIdent->getMyOnlineId();
    }

    m_SessionList.push_back( sessionInfo );
}

//============================================================================
User::User( const User& rhs )
    : m_Engine( rhs.m_Engine )
    , m_NetIdent( rhs.m_NetIdent )
    , m_MyOnlineId( rhs.m_MyOnlineId )
    , m_SessionList( rhs.m_SessionList )
{
}

//============================================================================
User& User::operator=( const User& rhs ) 
{	
    if( this != &rhs )
    {
        m_NetIdent = rhs.m_NetIdent;
        m_MyOnlineId = rhs.m_MyOnlineId;
        m_SessionList = rhs.m_SessionList;   
    }

    return *this;
}

//============================================================================
void User::setNetIdent( VxNetIdent* netIdent )   
{ 
    m_NetIdent = netIdent; 
    if( m_NetIdent )
    {
        m_MyOnlineId = netIdent->getMyOnlineId(); 
    }
    else
    {
        m_MyOnlineId.clearVxGUID();
    }
}

//============================================================================
bool User::addSession( BaseSessionInfo& sessionInfo )
{
    lockUser();
    for( BaseSessionInfo& session : m_SessionList )
    {
        if( session == sessionInfo )
        {
            unlockUser();
            return false;
        }
    }

    m_SessionList.push_back( sessionInfo );
    unlockUser();
    return true;
}

//============================================================================
bool User::removeSession( BaseSessionInfo& sessionInfo )
{
    lockUser();
    for( auto iter = m_SessionList.begin(); iter != m_SessionList.end(); ++iter)
    {
        if( *iter == sessionInfo )
        {
            m_SessionList.erase( iter );
            unlockUser();
            return true;
        }
    }

    unlockUser();
    return false;
}
