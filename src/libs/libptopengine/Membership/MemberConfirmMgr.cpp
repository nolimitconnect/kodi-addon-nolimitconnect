//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MemberConfirmMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <NetLib/VxSktBase.h>

#include <algorithm>

//============================================================================
MemberConfirmMgr& GetMemberConfirmMgr()
{
    static MemberConfirmMgr g_MemberConfirmMgr;
    return g_MemberConfirmMgr;
}

//============================================================================
void MemberConfirmMgr::addMemberConfirm( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{

    lockMemberList();

    bool found{ false };
    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); ++iter )
    {
        if( iter->second == onlineId )
        {
            iter->first = sktBase;
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_MemberList.emplace_back( std::make_pair( sktBase, onlineId ) );
    }

    unlockMemberList();
}

//============================================================================
void MemberConfirmMgr::onOncePerSecond( void )
{
    if( m_MemberList.size() )
    {
        lockMemberList();
        std::pair<std::shared_ptr<VxSktBase>, VxGUID> pair = m_MemberList.front();
        m_MemberList.erase( m_MemberList.begin() );
        unlockMemberList();

        if( pair.first && pair.first->isConnected() )
        {
            PktAnnounce pktAnn;
            GetPtoPEngine().getConnectionMgr().sendMyPktAnnounce( pair.second, pair.first, true, false, false, false );
        }
    }
}

//============================================================================
void MemberConfirmMgr::pktAnnRecieved( VxGUID onlineId )
{
    lockMemberList();
    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); )
    {
        if( iter->second == onlineId )
        {
            iter = m_MemberList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

     unlockMemberList();
}

//============================================================================
void MemberConfirmMgr::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
    lockMemberList();
    for( auto iter = m_MemberList.begin(); iter != m_MemberList.end(); )
    {
        if( iter->first == sktBase )
        {
            iter = m_MemberList.erase( iter );
        }
        else
        {
            ++iter;
        }
    }

    unlockMemberList();
}
