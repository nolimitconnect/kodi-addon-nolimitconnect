//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostServerSearchMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginBase.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsHostSearch.h>
#include <PktLib/PluginIdList.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxTime.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
HostServerSearchMgr::HostServerSearchMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase )
: HostBaseMgr(engine, pluginMgr, myIdent, pluginBase)
{
}

//============================================================================
void HostServerSearchMgr::updateHostSearchList( EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
    if( netIdent->requiresRelay() )
    {
        LogModule( eLogHostJoin, LOG_VERBOSE, "Host %s announce requries open port from %s %s", DescribeHostType( hostType ), netIdent->getOnlineName(), sktBase ? sktBase->describeSktConnection().c_str() : "Null Skt");
        return;
    }

    if( haveHostAnnList( hostType ) && netIdent->getMyOnlineId().isValid() )
    {
        EPluginType pluginType = getSearchPluginType( hostType );
        m_SearchMutex.lock();
        std::map<PluginId, HostSearchEntry>& searchMap = getHostAnnList( hostType );
        PluginId pluginId( netIdent->getMyOnlineId(), pluginType );
        auto iter = searchMap.find( pluginId );
        if( iter != searchMap.end() )
        {
            HostedInfo& hostInfo = iter->second.m_HostedInfo;
            int64_t prevTimestamp  = hostInfo.getHostInfoTimestamp();
            if( fillSearchEntry( iter->second, hostType, hostAnn, netIdent, false ) )
            {
                if( hostInfo.isHostInviteValid() )
                {
                    onHostInviteAnnounceUpdated( hostType, iter->second.m_HostedInfo, netIdent, sktBase, prevTimestamp != hostInfo.getHostInfoTimestamp() );
                }
                else
                {
                    LogMsg( LOG_ERROR, "HostServerSearchMgr Invalid Host Invite Announce from %s", netIdent->getOnlineName() );
                }
            }
            else
            {
                LogMsg( LOG_ERROR, "HostServerSearchMgr fillSearchEntry failed from Announce from %s", netIdent->getOnlineName() );
            }
        }
        else
        {
            HostSearchEntry searchEntry;
            if( fillSearchEntry( searchEntry, hostType, hostAnn, netIdent, true ) )
            {
                if( searchEntry.m_HostedInfo.isHostInviteValid() )
                {
                    searchMap[pluginId] = searchEntry;
                    onHostInviteAnnounceAdded( hostType, searchEntry.m_HostedInfo, netIdent, sktBase );
                }
                else
                {
                    LogMsg( LOG_ERROR, "HostServerSearchMgr Invalid Host Invite Announce from %s", netIdent->getOnlineName() );
                }
            }
            else
            {
                LogMsg( LOG_ERROR, "HostServerSearchMgr fillSearchEntry failed from %s", netIdent->getOnlineName() );
            }
        }

        m_SearchMutex.unlock();
        LogModule( eLogHostConnect, LOG_VERBOSE, "HostServerSearchMgr host ann %s %s updated ", pluginId.describePluginId().c_str(), netIdent->getOnlineName() );
    }
}

//============================================================================
ECommErr HostServerSearchMgr::searchRequest( SearchParams& searchParams, PktHostSearchReply& searchReply, std::string& searchStr, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    EHostType hostType = searchParams.getHostType();
    ECommErr searchErr = haveHostAnnList(hostType) ? eCommErrNone : eCommErrInvalidHostType;
    if( eCommErrNone == searchErr )
    {
        unsigned int matchCnt = 0;
        PluginIdList toRemoveList;
        PluginIdList matchList;
        uint64_t timeNow = GetGmtTimeMs();
        searchReply.setIsGuidPluginTypePairs( true );
        
        m_SearchMutex.lock();
        std::map<PluginId, HostSearchEntry>& searchMap = getHostAnnList( hostType );
        for( std::map<PluginId, HostSearchEntry>::iterator iter = searchMap.begin(); iter != searchMap.end(); ++iter )
        {
            if( iter->second.announceTimeExpired( timeNow ) )
            {
                toRemoveList.addPluginId( iter->first );
            }
            else if( iter->second.searchHostedMatch( searchParams, searchStr ) )
            {
                const PluginId& pluginId = iter->first;
                searchReply.addPluginId( pluginId );
                matchCnt++;
                LogModule( eLogHostConnect, LOG_DEBUG, "HostServerSearchMgr match %d plugin %s ", matchCnt, pluginId.describePluginId().c_str() );
            }
        }

        removeEntries( searchMap, toRemoveList );
        m_SearchMutex.unlock();
        searchReply.calcPktLen();
    }

    return searchErr;
}

//============================================================================
ECommErr HostServerSearchMgr::settingsRequest( PluginId& pluginId, PktPluginSettingReply& settingReply, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    /*
    EHostType hostType = settingReply.getHostType();
    ECommErr searchErr = haveBlob(hostType) ? eCommErrNone : eCommErrInvalidHostType;
    if( eCommErrNone == searchErr )
    {
        unsigned int matchCnt = 0;
        PluginIdList toRemoveList;
        PluginIdList matchList;
        uint64_t timeNow = GetGmtTimeMs();
        PktBlobEntry& blobEntry = settingReply.getBlobEntry();
        blobEntry.resetWrite();
        
        m_SearchMutex.lock();
        std::map<PluginId, HostSearchEntry>& searchMap = getSearchList( hostType );
        for( std::map<PluginId, HostSearchEntry>::iterator iter = searchMap.begin(); iter != searchMap.end(); ++iter )
        {
            if( timeNow - iter->second.m_LastRxTime > MIN_HOST_RX_UPDATE_TIME_MS )
            {
                toRemoveList.addPluginId( iter->first );
            }
            else if( iter->first == pluginId )
            {
                if( iter->second.m_Ident.addToBlob( blobEntry ) )
                {
                    if( iter->second.m_PluginSetting.addToBlob( blobEntry ) )
                    {
                        settingReply.setTotalMatches( settingReply.getTotalMatches() + 1 );
                        LogModule( eLogHostConnect, LOG_DEBUG, "HostServerSearchMgr match %d plugin %s ", matchCnt, pluginId.describePluginId().c_str() );
                    }
                }    
            }
        }

        removeEntries( searchMap, toRemoveList );
        m_SearchMutex.unlock();
        settingReply.calcPktLen();
    }

    return searchErr;
    */

    return eCommErrInvalidPkt;
}

//============================================================================
void HostServerSearchMgr::removeEntries( std::map<PluginId, HostSearchEntry>& searchMap, PluginIdList& toRemoveList )
{
    for( auto iter = toRemoveList.getPluginIdList().begin(); iter != toRemoveList.getPluginIdList().end(); ++iter )
    {
        auto iter2 = searchMap.find( *iter );
        if( iter2 != searchMap.end() )
        {
            searchMap.erase( iter2 );
        }
    }
}

//============================================================================
bool HostServerSearchMgr::fillSearchEntry( HostSearchEntry& searchEntry, EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, bool forced )
{
    searchEntry.updateLastRxTime();
    return searchEntry.updateHostedInfo( hostAnn );
}

//============================================================================
bool HostServerSearchMgr::haveHostAnnList( EHostType hostType )
{
    return hostType == eHostTypeChatRoom || hostType == eHostTypeGroup || hostType == eHostTypeRandomConnect;
}

//============================================================================
std::map<PluginId, HostSearchEntry>& HostServerSearchMgr::getHostAnnList( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeChatRoom:
        return m_ChatRoomHostAnnList;
    case eHostTypeGroup:
        return m_GroupHostAnnList;
    case eHostTypeRandomConnect:
        return m_RandConnectHostAnnList;
    default:
        return m_NullList;
    }
}

//============================================================================
EPluginType HostServerSearchMgr::getSearchPluginType( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeChatRoom:
        return ePluginTypeHostChatRoom;
    case eHostTypeGroup:
        return ePluginTypeHostGroup;
    case eHostTypeRandomConnect:
        return ePluginTypeHostRandomConnect;
    default:
        return ePluginTypeInvalid;
    }
}

//============================================================================
void HostServerSearchMgr::fromGuiSendAnnouncedList( EHostType hostType, VxGUID& sessionId )
{
    LogMsg( LOG_DEBUG, "HostServerMgr fromGuiSendAnnouncedList" );

    uint64_t timeNow = GetGmtTimeMs();
    m_SearchMutex.lock();
    std::map<PluginId, HostSearchEntry>& searchMap = getHostAnnList( hostType );
    for( std::map<PluginId, HostSearchEntry>::iterator iter = searchMap.begin(); iter != searchMap.end();  )
    {
        // if currently active
        if( !iter->second.announceTimeExpired( timeNow ) )
        {
            m_Engine.getToGui().toGuiHostSearchResult( hostType, sessionId, iter->second.m_HostedInfo );
            iter++;
        }
        else
        {
            iter = searchMap.erase( iter );
        }
    }

    m_SearchMutex.unlock();
    m_Engine.getToGui().toGuiHostSearchComplete( hostType, sessionId );
}

//============================================================================
void HostServerSearchMgr::fromGuiListAction( EListAction listAction )
{
    LogMsg( LOG_DEBUG, "HostServerSearchMgr fromGuiListAction" );

    doFromGuiListAction( listAction, ePluginTypeHostGroup, m_GroupHostAnnList );
    doFromGuiListAction( listAction, ePluginTypeHostChatRoom, m_ChatRoomHostAnnList );
    doFromGuiListAction( listAction, ePluginTypeHostRandomConnect, m_RandConnectHostAnnList );
}

//============================================================================
void HostServerSearchMgr::doFromGuiListAction( EListAction listAction, EPluginType pluginType, std::map<PluginId, HostSearchEntry>& hostedList )
{
    if( eListActionAnnounced == listAction )
    {
        hostedList.clear();
        std::map<PluginId, HostSearchEntry>& hostAnnList = getHostAnnList( PluginTypeToHostType( pluginType ) );

        uint64_t timeNow = GetGmtTimeMs();
        int entryNum = 0;
        LogMsg( LOG_INFO, "== Announced Hosts %s count %zu ==", DescribePluginType( pluginType ), hostedList.size() );
        m_SearchMutex.lock();
        for( auto iter = hostAnnList.begin(); iter != hostAnnList.end(); ++iter )
        {
            if( iter->first.getPluginType() == pluginType )
            {
                entryNum++;
                VxGUID onlineId = iter->second.m_HostedInfo.getAdminOnlineId();
                std::string onlineName( "" );
                m_Engine.getBigListMgr().getOnlineName( onlineId, onlineName );
                LogMsg( LOG_INFO, " #%d - %lld sec ago title %s desc %s user %s", entryNum, ( timeNow - iter->second.m_LastRxTime ) / 1000,
                    iter->second.m_HostedInfo.getHostTitle().c_str(), iter->second.m_HostedInfo.getHostDescription().c_str(), onlineName.c_str() );

                PluginId pluginId( onlineId, pluginType );
                hostedList[pluginId] = iter->second;
            }
        }

        m_SearchMutex.unlock();
    }
}

//============================================================================
bool HostServerSearchMgr::fillSearchPktBlob( PktBlobEntry& blobEntry, HostedInfo& hostedInfo )
{
    return hostedInfo.fillSearchBlob( blobEntry );
}

//============================================================================
bool HostServerSearchMgr::extractSearchBlobToHostedInfo( PktBlobEntry& blobEntry, HostedInfo& hostedInfo )
{
    return hostedInfo.extractFromSearchBlob( blobEntry );
}

//============================================================================
bool HostServerSearchMgr::onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteSearchReply pktReply;
    PktHostInviteSearchReq* pktReq = ( PktHostInviteSearchReq* )pktHdr;
    EHostType hostType = pktReq->getHostType();
    EPluginType pluginType = HostTypeToHostPlugin( hostType );
    VxGUID searchHostId = pktReq->getSpecificOnlineId();

    pktReply.setHostType( hostType );
    pktReply.setSearchSessionId( pktReq->getSearchSessionId() );
    if( !haveHostAnnList( hostType ) )
    {

        pktReply.setCommError( eCommErrInvalidHostType );
    }
    else
    {
        std::map<PluginId, HostSearchEntry>& hostAnnList = getHostAnnList( hostType );
        uint64_t timeNow = GetGmtTimeMs();

        m_SearchMutex.lock();
        if( searchHostId.isValid() )
        {
            // user wants just a specific host
            PluginId pluginId( searchHostId, pluginType );
            auto iter = hostAnnList.find( pluginId );
            if( iter != hostAnnList.end() )
            {
                if( !iter->second.announceTimeExpired( timeNow ) && fillSearchPktBlob( pktReply.getBlobEntry(), iter->second.m_HostedInfo ) )
                {
                    pktReply.incrementInviteCount();
                }
                else
                {
                    pktReply.setCommError( eCommErrSearchNoMatch );
                }
            }
            else
            {
                pktReply.setCommError( eCommErrSearchNoMatch );
            }
        }
        else
        {
            for( auto iter = hostAnnList.begin(); iter != hostAnnList.end(); )
            {
                if( !iter->second.announceTimeExpired( timeNow ) )
                {
                    if( fillSearchPktBlob( pktReply.getBlobEntry(), iter->second.m_HostedInfo ) )
                    {
                        pktReply.incrementInviteCount();
                    }
                    else
                    {
                        pktReply.setMoreInvitesExist( true );
                        pktReply.setNextSearchOnlineId( iter->second.m_HostedInfo.getAdminOnlineId() );
                        break;
                    }

                    ++iter;
                }
                else
                {
                    iter = hostAnnList.erase( iter );
                }
            }
        }

        m_SearchMutex.unlock();
    }

    if( 0 == pktReply.getInviteCountThisPkt() )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::%s NO hosts found of %s for %s %s", __func__, DescribeHostType( hostType ),
                    sktBase->getRemoteIpAddress(), netIdent->getOnlineName());
    }
    else
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::%s found %d of %s for %s %s", __func__, pktReply.getInviteCountThisPkt(),
                    DescribeHostType( hostType ), sktBase->getRemoteIpAddress(), netIdent->getOnlineName() );
    }

    pktReply.calcPktLen();
    EPluginType clientPluginType = HostTypeToClientPlugin( hostType );
    pktReply.setPluginNum( (uint8_t)clientPluginType );
    return m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, &pktReply, clientPluginType );
}

//============================================================================
void HostServerSearchMgr::onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteSearchReply* pktReply = ( PktHostInviteSearchReply* )pktHdr;
    if( pktReply->getCommError() )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::%s comm err %s", __func__, DescribeCommError( pktReply->getCommError() ) );
        logCommError( pktReply->getCommError(), "PktHostInviteSearchReply", sktBase, netIdent );
        m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getCommError() );
    }
    else
    {
        updateFromHostInviteSearchBlob( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getBlobEntry(), pktReply->getInviteCountThisPkt() );
        
        if( pktReply->getMoreInvitesExist() )
        {
            if( !requestMoreHostInvitesFromNetworkHost( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getNextSearchOnlineId() ) )
            {
                m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
            }
        }
        else
        {
            m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
        }
    }
}

//============================================================================
bool HostServerSearchMgr::onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteMoreReply pktReply;
    PktHostInviteMoreReq* pktReq = ( PktHostInviteMoreReq* )pktHdr;
    EHostType hostType = pktReq->getHostType();
    EPluginType pluginType = HostTypeToHostPlugin( hostType );
    VxGUID nextHostId = pktReq->getNextSearchOnlineId();

    LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::onPktHostInviteMoreReq comm err %s", DescribeHostType( hostType ) );

    pktReply.setHostType( hostType );
    pktReply.setSearchSessionId( pktReq->getSearchSessionId() );
    if( !haveHostAnnList( hostType ) || !nextHostId.isValid() )
    {
        pktReply.setCommError( eCommErrInvalidHostType );
    }
    else
    {
        PluginId pluginId( nextHostId, pluginType );
        std::map<PluginId, HostSearchEntry>& hostAnnList = getHostAnnList( hostType );
        uint64_t timeNow = GetGmtTimeMs();

        m_SearchMutex.lock();
        auto iter = hostAnnList.find( pluginId );
        if( iter != hostAnnList.end() )
        {
            while( iter != hostAnnList.end() )
            {
                if( !iter->second.announceTimeExpired( timeNow ) )
                {
                    if( fillSearchPktBlob( pktReply.getBlobEntry(), iter->second.m_HostedInfo ) )
                    {
                        pktReply.incrementInviteCount();
                    }
                    else
                    {
                        pktReply.setMoreInvitesExist( true );
                        pktReply.setNextSearchOnlineId( iter->second.m_HostedInfo.getAdminOnlineId() );
                        break;
                    }

                    ++iter;
                }
                else
                {
                    iter = hostAnnList.erase( iter );
                }
            }
        }
        else
        {
            pktReply.setCommError( eCommErrSearchNoMatch );
        }

        m_SearchMutex.unlock();
    }

    pktReply.calcPktLen();
    EPluginType clientPluginType = HostTypeToClientPlugin( hostType );
    pktReply.setPluginNum( (uint8_t)clientPluginType );
    return m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, &pktReply, clientPluginType );
}

//============================================================================
void HostServerSearchMgr::onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteMoreReply* pktReply = ( PktHostInviteMoreReply* )pktHdr;
    if( pktReply->getCommError() )
    {
        logCommError( pktReply->getCommError(), "PktHostInviteSearchReply", sktBase, netIdent );
        m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getCommError() );
    }
    else
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::onPktHostInviteMoreReply %d hosts", pktReply->getInviteCountThisPkt() );

        updateFromHostInviteSearchBlob( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getBlobEntry(), pktReply->getInviteCountThisPkt() );
        if( pktReply->getMoreInvitesExist() )
        {
            if( !requestMoreHostInvitesFromNetworkHost( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getNextSearchOnlineId() ) )
            {
                m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
            }
        }
        else
        {
            m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
        }
    }
}

//============================================================================
void HostServerSearchMgr::logCommError( ECommErr commErr, const char* desc, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    LogMsg( LOG_ERROR, "%s %s from %s %s", desc, DescribeCommError( commErr ), netIdent->getOnlineName(), sktBase->describeSktConnection().c_str() );
}

//============================================================================
void HostServerSearchMgr::updateFromHostInviteSearchBlob( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, PktBlobEntry& blobEntry, int hostInfoCount )
{
    LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr::%s rxed %d %s hosts", __func__, hostInfoCount, DescribeHostType( hostType ) );
    blobEntry.resetRead();
    for( int i = 0; i < hostInfoCount; i++ )
    {
        HostedInfo hostedInfo;
        if( extractSearchBlobToHostedInfo( blobEntry, hostedInfo ) )
        {
            if( eHostTypeUnknown == hostedInfo.getHostType() && eHostTypeUnknown != hostType )
            {
                hostedInfo.setHostType( hostType );
            }

            m_Engine.getHostedListMgr().hostSearchResult( hostType, searchSessionId, sktBase, netIdent, hostedInfo );
        }
        else
        {
            LogMsg( LOG_ERROR, "Could not extract HostInviteSearchBlob" );
            break;
        }
    }
}

//============================================================================
bool HostServerSearchMgr::requestMoreHostInvitesFromNetworkHost( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& nextHostOnlineId )
{
    PktHostInviteMoreReq pktReq;
    pktReq.setHostType( hostType );
    pktReq.setSearchSessionId( searchSessionId );
    pktReq.setNextSearchOnlineId( nextHostOnlineId );
    EPluginType clientPluginType = HostTypeToClientPlugin( hostType );
    pktReq.setPluginNum( (uint8_t)ePluginTypeHostNetwork );
    LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr:%s %s ", __func__, DescribeHostType(hostType) );

    return m_Plugin.txPacket( netIdent->getMyOnlineId(), sktBase, &pktReq, ePluginTypeHostNetwork );
}

//============================================================================
void HostServerSearchMgr::onHostInviteAnnounceAdded( EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
    LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr:%s %s url %s ", __func__, DescribeHostType(hostType), hostedInfo.getHostInviteUrl().c_str() );
    // NOTE: search mutex is still locked
    m_Engine.getHostedListMgr().onHostInviteAnnounceAdded( hostType, hostedInfo, netIdent, sktBase );
}

//============================================================================
void HostServerSearchMgr::onHostInviteAnnounceUpdated( EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, bool infoChanged )
{
    LogModule( eLogHostSearch, LOG_DEBUG, "HostServerSearchMgr:%s %s url %s ", __func__, DescribeHostType(hostType), hostedInfo.getHostInviteUrl().c_str() );
    // NOTE: search mutex is still locked
    m_Engine.getHostedListMgr().onHostInviteAnnounceUpdated( hostType, hostedInfo, netIdent, sktBase, infoChanged );
}

//============================================================================
int HostServerSearchMgr::getAnnouncedHostCount( EHostType hostType )
{
    int count{ 0 };
    m_SearchMutex.lock();
    switch( hostType )
    {
    case eHostTypeUnknown:
        count = (int)(m_ChatRoomHostAnnList.size() + m_GroupHostAnnList.size() + m_RandConnectHostAnnList.size());
        break;
    case eHostTypeChatRoom:
        count = (int)m_ChatRoomHostAnnList.size();;
    case eHostTypeGroup:
        count = (int)m_GroupHostAnnList.size();;
    case eHostTypeRandomConnect:
        count = (int)m_RandConnectHostAnnList.size();;
    default:
        break;
    }

    m_SearchMutex.unlock();
    return count;
}
