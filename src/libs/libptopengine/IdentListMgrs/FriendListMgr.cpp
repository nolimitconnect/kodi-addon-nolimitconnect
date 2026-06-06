//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FriendListMgr.h"
#include <P2PEngine/P2PEngine.h>
#include <CoreLib/VxDebug.h>

//============================================================================
FriendListMgr::FriendListMgr( P2PEngine& engine )
    : IdentListMgrBase( engine )
{
    setIdentListType( eUserViewTypeFriendsOnline );
}

//============================================================================
bool FriendListMgr::isFriend( VxGUID& onlineId )
{
    bool isFriend = false;
    lockIdentList();
    for( auto iter = m_FriendIdentList.begin(); iter != m_FriendIdentList.end(); ++iter )
    {
        if( iter->first == onlineId )
        {
            isFriend = true;
            break;
        }
    }

    unlockIdentList();
    return isFriend;
}

//============================================================================
void FriendListMgr::updateIdent( VxGUID& onlineId, int64_t timestamp )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "IgnoreListMgr::updateIgnoreIdent invalid id" );
        return;
    }

    if( onlineId == m_Engine.getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, "IgnoreListMgr::updateIgnoreIdent cannot change friendship to myself" );
        return;
    }

    bool wasInserted = false;
    bool wasErased = false;
    bool timestampUpdated = false;
    lockIdentList();
    for( auto iter = m_FriendIdentList.begin(); iter != m_FriendIdentList.end(); )
    {
        if( iter->first == onlineId )
        {
            iter = m_FriendIdentList.erase( iter );
            wasErased = true;
        }
        else if( !wasInserted )
        {
            if( timestamp > iter->second )
            {
                iter = m_FriendIdentList.insert( iter, std::make_pair( onlineId, timestamp ) );
                timestampUpdated = true;
                wasInserted = true;
            }
            else
            {
                ++iter;
            }
        }
        else
        {
            ++iter;
        }

        if( wasErased && wasInserted )
        {
            break;
        }
    }

    if( !wasInserted )
    {
        m_FriendIdentList.emplace_back( std::make_pair( onlineId, timestamp ) );
        wasInserted = true;
    }

    unlockIdentList();

    if( timestampUpdated || ( wasInserted && !wasErased ) )
    {
        onUpdateIdent( onlineId, timestamp );
    }
}

//============================================================================
void FriendListMgr::removeIdent( VxGUID& onlineId )
{
    bool wasRemoved = false;
    lockIdentList();
    for( auto iter = m_FriendIdentList.begin(); iter != m_FriendIdentList.end(); ++iter )
    {
        if( iter->first == onlineId )
        {
            m_FriendIdentList.erase( iter );
            wasRemoved = true;
            break;
        }
    }

    unlockIdentList();

    if( wasRemoved )
    {
        onRemoveIdent( onlineId );
    }
}
