//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include "P2PEngine.h"

#include <BigListLib/BigListInfo.h>
#include <Membership/MemberActiveMgr.h>
#include <Membership/MemberConfirmMgr.h>
#include <SendQueue/SendQueueMgr.h>

#include <NetLib/VxSktBase.h>
#include <CoreLib/VxDebug.h>

//============================================================================
bool P2PEngine::onFirstPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo, ConnectId& connectId )
{
    pktAnn->clearIsJoined();
    if( pktAnn->getMyOnlineId() == getMyOnlineId() )
    {
        VxReportHack( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, sktBase, "P2PEngine::%s myself so closing connection", __func__ );
        sktBase->closeSkt( eSktCloseHackLevelSevere );
        return false;
    }

    bool updateOk{ true };

    if(LogEnabled( eLogConnect ))LogModule( eLogConnect, LOG_VERBOSE, "%s name %s %s at ip %s pktAnn his friendship %s my friendship %s", __func__,
                bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str(),
                DescribeFriendState( pktAnn->getHisFriendshipToMe() ),
                DescribeFriendState( bigListInfo->getMyFriendshipToHim() ) );


    if( !sktBase->getIsPeerPktAnnSet() )
    {
        if(LogEnabled( eLogConnect ))LogModule( eLogConnect, LOG_VERBOSE, "%s set peer %s", __func__, pktAnn->describeUser().c_str() );

        if( sktBase->setPeerPktAnn( *pktAnn ) )
        {
            if( pktAnn->getIsPktAnnTempConnection() )
            {
                sktBase->setIsTempConnection( true );
            }

            if( !sktBase->isTempConnection() )
            {
                getConnectIdListMgr().addConnection( connectId );
                getConnectList().addConnection( sktBase, bigListInfo, (ePktAnnUpdateTypeNewContact == pktAnnUpdateType) );
            }

            getConnectionMgr().onSktConnectedWithPktAnn( sktBase, bigListInfo );
        }
        else
        {
            getConnectList().addConnection( sktBase, bigListInfo, (ePktAnnUpdateTypeNewContact == pktAnnUpdateType) );
        }
    }

    return updateOk && onPktAnnounceCommonHandler( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
}

//============================================================================
bool P2PEngine::onConnectionPktAnnounceUpdated( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo )
{
    if( pktAnn->getMyOnlineId() == getMyOnlineId() )
    {
        VxReportHack( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, sktBase, "P2PEngine::onConnectionPktAnnounceUpdated" );
        sktBase->closeSkt( eSktCloseHackLevelSevere );
        return false;
    }

    // the updates to user should have been done in m_BigListMgr.updatePktAnn
    if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "%s %s %s at ip %s", __func__,
               bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
    
    // update the pkt ann set into the connection
    sktBase->setPeerPktAnn( *pktAnn ); 

    // again request thumbs in case they have changed
    getThumbMgr().requestThumbs( sktBase, bigListInfo->getVxNetIdent() );
    // tell gui the user has updated

    return true;
}

//============================================================================
bool P2PEngine::onHostedUserPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo, ConnectId& connectId )
{
    if( pktAnn->getMyOnlineId() == getMyOnlineId() )
    {
        VxReportHack( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, sktBase, "P2PEngine::%s", __func__ );
        sktBase->closeSkt( eSktCloseHackLevelSevere );
        return false;
    }

    bool updateOk{ true };

    GroupieId groupieId( connectId.getGroupieId() );

    if( !sktBase->isTempConnection() )
    {
        EHostType hostType = connectId.getHostType();
        if( IsHostARelayForUsers( hostType ) )
        {
            pktAnn->setIsJoined( hostType, true );
            bigListInfo->setIsJoined( hostType, true );
        }    

        if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "P2PEngine::%s %s %s at ip %s", __func__,
                   bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
        if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "P2PEngine::%s %s %s", __func__, pktAnn->describeUser().c_str(), describeGroupieId( groupieId ).c_str() );
    }

    getConnectIdListMgr().addConnection( connectId );
    if( !sktBase->isTempConnection() )
    {
        bool newMember = getMemberActiveMgr().updateMemberActive( groupieId, true );

        if( newMember && !getConnectIdListMgr().isDirectConnected( pktAnn->getMyOnlineId() ) )
        {
            GetMemberConfirmMgr().addMemberConfirm( sktBase, pktAnn->getMyOnlineId() );
        }
    }

    return updateOk && onPktAnnounceCommonHandler( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
}

//============================================================================
bool P2PEngine::onRelayedUserPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo )
{
    if( pktAnn->getMyOnlineId() == getMyOnlineId() )
    {
        VxReportHack( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, sktBase, "P2PEngine::onRelayedUserPktAnnounce" );
        sktBase->closeSkt( eSktCloseHackLevelSevere );
        return false;
    }

    if( !sktBase->isTempConnection() )
    {
        // the updates to user should have been done in m_BigListMgr.updatePktAnn
        if( LogEnabled( eLogConnect ) )LogModule( eLogConnect, LOG_VERBOSE, "onRelayedUserPktAnnounce %s %s at ip %s",
                   bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );

        if( LogEnabled( eLogOnline ) )LogModule( eLogOnline, LOG_VERBOSE, "P2PEngine::%s %s from %s", __func__,
            pktAnn->describeUser().c_str(), bigListInfo->describeUser().c_str() );
    }

    GetMemberConfirmMgr().pktAnnRecieved( pktAnn->getMyOnlineId() );

    return onPktAnnounceCommonHandler( sktBase, pktAnn, pktAnnUpdateType, bigListInfo );
}

//============================================================================
bool P2PEngine::onUnexpectedPktAnnounce( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo )
{
    if( pktAnn->getMyOnlineId() == getMyOnlineId() )
    {
        VxReportHack( eHackerLevelSevere, eHackerReasonPktOnlineIdMeFromAnotherIp, sktBase, "P2PEngine::onUnexpectedPktAnnounce" );
        sktBase->closeSkt( eSktCloseHackLevelSevere );
        return false;
    }

    bool updateOk{ true };

    LogModule( eLogConnect, LOG_ERROR, "onUnexpectedPktAnnounce %s %s at ip %s",
               bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str() );
    return updateOk;
}

//============================================================================
bool P2PEngine::onPktAnnounceCommonHandler( std::shared_ptr<VxSktBase>& sktBase, PktAnnounce* pktAnn, enum EPktAnnUpdateType pktAnnUpdateType, BigListInfo* bigListInfo )
{
    bool updateOk{ true };
    
    if( !sktBase->isTempConnection() )
	{
        
        LogModule( eLogConnect, LOG_VERBOSE, "onPktAnnounceCommonHandler %s %s at ip %s my frienship %s his friendship %s",
               bigListInfo->getOnlineName(), bigListInfo->getMyOnlineId().toOnlineIdString().c_str(), sktBase->getRemoteIp().c_str(),
               DescribeFriendState( bigListInfo->getMyFriendshipToHim() ), DescribeFriendState( bigListInfo->getHisFriendshipToMe() ) );

        getToGui().toGuiContactAdded( bigListInfo->getVxNetIdent() );

        getConnectIdListMgr().pktAnnRecieved( sktBase, pktAnn, bigListInfo->getVxNetIdent() );
        getThumbMgr().requestThumbs( sktBase, bigListInfo->getVxNetIdent() );

        getSendQueueMgr().pktAnnRecieved( pktAnn->getMyOnlineId() );
	}	
   
    return updateOk;
}
