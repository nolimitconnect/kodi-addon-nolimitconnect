#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostBaseMgr.h"
#include "HostSearchEntry.h"

#include <CoreLib/PluginId.h>

#include <CoreLib/VxGUIDList.h>
#include <CoreLib/VxMutex.h>

#include <map>

#define MIN_HOST_RX_UPDATE_TIME_MS         (30 * 60 * 1000) // must rx a host announce in this time frame or host is considered offline

class ConnectionMgr;
class P2PEngine;
class PluginBase;
class PluginId;
class PluginIdList;
class PluginMgr;
class VxPktHdr;
class PktHostSearchReply;
class PktPluginSettingReply;

class HostServerSearchMgr : public HostBaseMgr
{
public:
    HostServerSearchMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase );
	virtual ~HostServerSearchMgr() = default;

    int                         getAnnouncedHostCount( EHostType hostType );

    void                        updateHostSearchList( EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

    virtual ECommErr            searchRequest( SearchParams& searchParams, PktHostSearchReply& searchReply, std::string& searchStr, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    virtual ECommErr            settingsRequest( PluginId& pluginId, PktPluginSettingReply& searchReply, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );

    virtual void				fromGuiSendAnnouncedList( EHostType hostType, VxGUID& sessionId );
    virtual void				fromGuiListAction( EListAction listAction );

    virtual bool				onPktHostInviteSearchReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual bool				onPktHostInviteMoreReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

protected:
    std::map<PluginId, HostSearchEntry>& getHostAnnList( EHostType hostType );
    bool                        haveHostAnnList( EHostType hostType );
    bool                        fillSearchEntry( HostSearchEntry& searchEntry, EHostType hostType, PktHostInviteAnnounceReq* hostAnn, VxNetIdent* netIdent, bool forced );

    void                        onHostInviteAnnounceAdded( EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
    void                        onHostInviteAnnounceUpdated( EHostType hostType, HostedInfo& hostedInfo, VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, bool infoChanged = true );

    // remove entries does not lock m_SearchMutex
    void                        removeEntries( std::map<PluginId, HostSearchEntry>& searchMap, PluginIdList& toRemoveList );
    EPluginType                 getSearchPluginType( EHostType hostType );
    void                        doFromGuiListAction( EListAction listAction, EPluginType pluginType, std::map<PluginId, HostSearchEntry>& hostedList );
    bool                        fillSearchPktBlob( PktBlobEntry& blobEntry, HostedInfo& hostedInfo );
    bool                        extractSearchBlobToHostedInfo( PktBlobEntry& blobEntry, HostedInfo& hostedInfo );

    void                        logCommError( ECommErr commErr, const char* desc, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
    void                        updateFromHostInviteSearchBlob( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, PktBlobEntry& blobEntry, int hostInfoCount );
    bool                        requestMoreHostInvitesFromNetworkHost( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& nextHostOnlineId );

    //=== vars ===//
    VxMutex                     m_SearchMutex;
    std::map<PluginId, HostSearchEntry>   m_ChatRoomHostAnnList;
    std::map<PluginId, HostSearchEntry>   m_GroupHostAnnList;
    std::map<PluginId, HostSearchEntry>   m_RandConnectHostAnnList;
    std::map<PluginId, HostSearchEntry>   m_NullList; // empty list and list for network hosts
};

