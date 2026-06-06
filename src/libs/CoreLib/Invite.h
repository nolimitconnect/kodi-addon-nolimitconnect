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

#include "VxPtopUrl.h"

#include <vector>

class Invite
{
public:
    static const char* INVITE_BEGIN;
    static const char* INVITE_HDR_MSG;
    static const char* INVITE_HDR_NET_SETTING;
    static const char* INVITE_END;
    static const char* PTOP_URL_PREFIX;             // ptop://
    static const char SUFFIX_CHAR_PERSON_RELAYED;   // P
    static const char SUFFIX_CHAR_PERSON_DIRECT;    // D
    static const char SUFFIX_CHAR_GROUP;            // G
    static const char SUFFIX_CHAR_CHAT_ROOM;        // C
    static const char SUFFIX_CHAR_RANDOM_CONNECT;   // R
    static const char SUFFIX_CHAR_NETWORK_HOST;     // N
    static const char SUFFIX_CHAR_CONNECT_TEST;     // T
    static const char SUFFIX_CHAR_UNKNOWN;          // U

    static const char* INVITE_PRIVATE_KEY;          // NetworkKey-Private

    bool                        setInviteText( std::string inviteText );
    std::string&                getInviteText( void )                       { return m_InviteText; }
    bool                        getInviteUrls( std::vector<VxPtopUrl>& hostUrls, std::vector<VxPtopUrl>& networkUrls, std::string& userMsg );

    static bool                 isInviteTextValid( std::string inviteText );
    static bool                 inviteHasPrivateNetworkKey( std::string inviteText );

    bool                        setInviteUrl( EHostType hostType, std::string& url, bool isNetworkUrl = false );

    std::string&                getInviteUrl( EHostType hostType );
    bool                        getInviteUrl( EHostType hostType, VxPtopUrl& retUrl );
    std::string&                getNetSettingUrl( EHostType hostType );
    bool                        getNetSettingUrl( EHostType hostType, VxPtopUrl& retUrl );

    static char                 getHostTypeSuffix( EHostType hostType );
    static EHostType            getHostTypeFromSuffix( const char suffix );
    static bool                 isValidHostTypeSuffix( const char suffix );

    static std::string          makeInviteUrl( EHostType hostType, std::string& onlineUrl ); // assumes vaild node url with online id
    static bool                 appendHostTypeSuffix( EHostType hostType, std::string& onlineUrl ); // add host type suffix if needed

protected:
    bool                        parseInviteText( void );
    void                        clearInvite( void );
    static bool                 textContains( std::string& inviteText, std::string searchText );

    std::string                 m_InviteText;

    std::string                 m_UserMsg;

    std::string                 m_PersonUrl;
    std::string                 m_GroupUrl;
    std::string                 m_ChatRoomUrl;
    std::string                 m_RandomConnectUrl;

    std::string                 m_NetSettingGroupUrl;
    std::string                 m_NetSettingChatRoomUrl;
    std::string                 m_NetSettingRandomConnectUrl;
    std::string                 m_NetSettingNetworkHostUrl;
    std::string                 m_NetSettingConnectTestUrl;

    std::string                 m_EmptyString;
};

